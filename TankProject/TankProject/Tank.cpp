#define _USE_MATH_DEFINES

#include "Tank.hpp"
#include "DataTables.hpp"
#include "Utility.hpp"
#include "Pickup.hpp"
#include "CommandQueue.hpp"
#include "SoundNode.hpp"
#include "NetworkNode.hpp"
#include "ResourceHolder.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <cmath>
#include <iostream>

using namespace std::placeholders;


namespace
{
	const std::vector<TankData> Table = initializeTankData();
	const std::vector<TankTurretData> TableTurrets = initializeTankTurretData();
}

Tank::Tank(Type type, const TextureHolder& textures, const FontHolder& fonts)
	: Entity(Table[type].hitpoints)
	, mType(type)
	, mSprite(textures.get(Table[type].texture), Table[type].textureRect)
	, mExplosion(textures.get(Textures::Explosion))
	, mFireCommand()
	, mMissileCommand()
	, mFireCountdown(sf::Time::Zero)
	, mRotateCountdown(sf::Time::Zero)
	, mIsFiring(false)
	, mIsLaunchingMissile(false)
	, mShowExplosion(true)
	, mExplosionBegan(false)
	, mSpawnedPickup(false)
	, mPickupsEnabled(true)
	, mFireRateLevel(1)
	, mRotateRateLevel(1)
	, mSpreadLevel(1)
	, mMissileAmmo(2)
	, mDropPickupCommand()
	, mTravelledDistance(0.f)
	, mDirectionIndex(0)
	, ammoDisplay(nullptr)
	, mIdentifier(0)
	, speedBoostMultiplier(1.0f)
	, turretRotationVelocity(0.0f)
	, isRotating(false)
{
	mExplosion.setFrameSize(sf::Vector2i(256, 256));
	mExplosion.setNumFrames(16);
	mExplosion.setDuration(sf::seconds(1));

	centerOrigin(mSprite);
	centerOrigin(mExplosion);

	mFireCommand.category = Category::SceneAirLayer;
	mFireCommand.action = [this, &textures](SceneNode& node, sf::Time)
	{
		createBullets(node, textures);
	};

	mMissileCommand.category = Category::SceneAirLayer;
	mMissileCommand.action = [this, &textures](SceneNode& node, sf::Time)
	{
		createProjectile(node, Projectile::Missile, 0.f, 0.5f, textures);
	};

	mDropPickupCommand.category = Category::SceneAirLayer;
	mDropPickupCommand.action = [this, &textures](SceneNode& node, sf::Time)
	{
		createPickup(node, textures);
	};

	std::unique_ptr<TextNode> healthDisplay(new TextNode(fonts, ""));
	mHealthDisplay = healthDisplay.get();
	mHealthDisplay->setOrigin(0, -50);
	attachChild(std::move(healthDisplay));

	std::unique_ptr<TextNode> missileDisplay(new TextNode(fonts, ""));
	missileDisplay->setOrigin(0, -70);
	ammoDisplay = missileDisplay.get();
	attachChild(std::move(missileDisplay));

	ammoCount = Table[type].ammoCount;

	//adds turret type depending on what type of tank it is
	switch (type)
	{
	case Hotchkiss:
		turretType = HotchkissTurret;
		break;

	case T34:
		turretType = T34Turret;
		break;

	case Panzer:
		turretType = PanzerTurret;
		break;

	case Panther:
		turretType = PantherTurret;
	}

	turretRotationVelocity = 0.0f;
	turretSprite = sf::Sprite(textures.get(TableTurrets[turretType].texture), TableTurrets[turretType].textureRect);
	centerOrigin(turretSprite);

	playLocalSound(SoundEffect::TankIdle, true);

	updateTexts();
}

int Tank::getMissileAmmo() const
{
	return mMissileAmmo;
}

void Tank::setMissileAmmo(int ammo)
{
	mMissileAmmo = ammo;
}

void Tank::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (isDestroyed() && mShowExplosion)
		target.draw(mExplosion, states);
	else
	{
		target.draw(mSprite, states);
		target.draw(turretSprite, states);
		drawBoundingCirc(target, states, mRadius);
	}
}

void Tank::disablePickups()
{
	mPickupsEnabled = false;
}

float Tank::getTankRadius()
{
	return mRadius;
}

void Tank::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	// Update texts and roll animation
	updateTexts();
	updateTurret(dt);
	checkSpeedBoost(dt);

	// Entity has been destroyed: Possibly drop pickup, mark for removal
	if (isDestroyed())
	{
		checkPickupDrop(commands);
		mExplosion.update(dt);

		// Play explosion sound only once
		if (!mExplosionBegan)
		{
			SoundEffect::ID soundEffect = (randomInt(2) == 0) ? SoundEffect::Explosion1 : SoundEffect::Explosion2;
			playLocalSound(commands, soundEffect);

			// Emit network game action for enemy explosions
			if (!isAllied())
			{
				sf::Vector2f position = getWorldPosition();

				Command command;
				command.category = Category::Network;
				command.action = derivedAction<NetworkNode>([position](NetworkNode& node, sf::Time)
				{
					node.notifyGameAction(GameActions::EnemyExplode, position);
				});

				commands.push(command);
			}

			mExplosionBegan = true;
		}
		return;
	}



	if (isTankRotating)
	{
		
		playLocalSound(commands, SoundEffect::TankTurretRotate);
	
	}

	

	/*if () 
	{
		playLocalSound(commands, SoundEffect::TankMove);
	}
	else
		playLocalSound(commands, SoundEffect::TankIdle);
*/
	// Check if bullets are fired
	checkProjectileLaunch(dt, commands);

	

	// Update enemy movement pattern; apply velocity
	//updateMovementPattern(dt);
	Entity::updateCurrent(dt, commands);
}

unsigned int Tank::getCategory() const
{
	if (isAllied())
		return Category::LiberatorTank;
	else
		return Category::ResistanceTank;
}

sf::FloatRect Tank::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

//sf::CircleShape Tank::getBoundingCircle() const
//{
//	return getWorldTransform().transformCircle(mSprite.getGlobalBounds());
//}


bool Tank::isMarkedForRemoval() const
{
	return isDestroyed() && (mExplosion.isFinished() || !mShowExplosion);
}

void Tank::remove()
{
	Entity::remove();
	mShowExplosion = false;
}

bool Tank::isAllied() const
{
	return mType == Hotchkiss || mType == T34;
}

float Tank::getMaxSpeed() const
{
	return Table[mType].speed;
}

float Tank::getTurretRotationSpeed() const
{
	return TableTurrets[turretType].rotationSpeed;
}

float Tank::getMaxTurretRotationSpeed() const
{
	return TableTurrets[turretType].maxRotationSpeed;
}

int	Tank::getAmmoCount() const
{
	return ammoCount;
}

float Tank::getSpeedBoost() const
{
	return speedBoostMultiplier;
}

int	Tank::getIdentifier()
{
	return mIdentifier;
}

void Tank::setIdentifier(int identifier)
{
	mIdentifier = identifier;
}

float Tank::getTurretRotation()
{
	return turretSprite.getRotation();
}

void Tank::setTurretRotation(float rotation)
{
	turretSprite.setRotation(rotation);
}

Tank::Type Tank::getType()
{
	return mType;
}

Tank::Type Tank::getAllyType()
{
	if (mType == Tank::Hotchkiss)
	{
		return Tank::T34;
	}
	else
	{
		return Tank::Panther;
	}
}





// ---- Boosts ----
void Tank::increaseFireRate()
{
	if (mFireRateLevel < 10)
		++mFireRateLevel;
}

//ammo pickup
void Tank::collectAmmo(unsigned int count)
{
	ammoCount += count;
}

//tank speed increase pickup
void Tank::increaseTankSpeed(float lengthInSeconds)
{
	hasSpeedBoost = true;

	speedBoostMultiplier = 1.66f;

	speedBoostCountdown = sf::seconds(lengthInSeconds);
}

//repair tank pickup, does not go above original limit
void Tank::increaseHealth(int amount)
{
	if (amount + getHitpoints() > Table[mType].hitpoints)
	{
		amount = Table[mType].hitpoints;
	}

	setHitpoints(amount);
}





void Tank::fire()
{
	// Only ships with fire interval != 0 are able to fire
	if (Table[mType].fireInterval != sf::Time::Zero)
		mIsFiring = true;
}

void Tank::playLocalSound(CommandQueue& commands, SoundEffect::ID effect)
{
	sf::Vector2f worldPosition = getWorldPosition();

	Command command;
	command.category = Category::SoundEffect;
	command.action = derivedAction<SoundNode>(
		[effect, worldPosition](SoundNode& node, sf::Time)
	{
		node.playSound(effect, worldPosition);
	});

	commands.push(command);
}

void Tank::playLocalSound(SoundEffect::ID effect, bool looped)
{
	sf::Vector2f worldPosition = getWorldPosition();

	Command command;
	command.category = Category::SoundEffect;
	command.action = derivedAction<SoundNode>(
		[effect, worldPosition, looped](SoundNode& node, sf::Time)
	{
		node.playSound(effect, worldPosition, looped);
	});

	

	
}

void Tank::playRotateSound()
{
	isTankRotating = true;
	//sf::Time dt;

	//if (isTankRotating == false && mRotateCountdown <= sf::Time::Zero)
	//{
	//	mRotateCountdown += Table[mType].rotateInterval / (mRotateRateLevel + 1.f);

	//	isTankRotating = true;
	//}
	//else if (mRotateCountdown > sf::Time::Zero)
	//{
	//	// Interval not expired: Decrease it further

	//	mRotateCountdown -= dt;
	//	isTankRotating = false;
	//}
}

void Tank::updateMovementPattern(sf::Time dt)
{
	// Enemy tank: Movement pattern
	const std::vector<Direction>& directions = Table[mType].directions;
	if (!directions.empty())
	{
		// Moved long enough in current direction: Change direction
		if (mTravelledDistance > directions[mDirectionIndex].distance)
		{
			mDirectionIndex = (mDirectionIndex + 1) % directions.size();
			mTravelledDistance = 0.f;
		}

		// Compute velocity from direction
		float radians = toRadian(directions[mDirectionIndex].angle + 90.f);
		float vx = getMaxSpeed() * std::cos(radians);
		float vy = getMaxSpeed() * std::sin(radians);

		setVelocity(vx, vy);

		mTravelledDistance += getMaxSpeed() * dt.asSeconds();
	}
}

void Tank::checkPickupDrop(CommandQueue& commands)
{
	if (!isAllied() && randomInt(3) == 0 && !mSpawnedPickup)
		commands.push(mDropPickupCommand);

	mSpawnedPickup = true;
}

void Tank::checkProjectileLaunch(sf::Time dt, CommandQueue& commands)
{
	// Enemies rotate towards you 
	/*if (!isAllied())
	{
		if (mTargetDirection != sf::Vector2f(0,0))
		{
			updateEnemyTurretRotation(dt);
		}
	}*/

	// Check for automatic gunfire, allow only in intervals
	if (mIsFiring && mFireCountdown <= sf::Time::Zero && ammoCount > 0)
	{
		// Interval expired: We can fire a new bullet
		commands.push(mFireCommand);

		//playLocalSound(commands, isAllied() ? SoundEffect::AlliedGunfire : SoundEffect::EnemyGunfire);
		playLocalSound(commands, SoundEffect::TankFire);

		mFireCountdown += Table[mType].fireInterval / (mFireRateLevel + 1.f);
		
		ammoCount--;

		mIsFiring = false;
	}
	else if (mFireCountdown > sf::Time::Zero)
	{
		// Interval not expired: Decrease it further
		
		mFireCountdown -= dt;
		mIsFiring = false;
	}
}

void Tank::createBullets(SceneNode& node, const TextureHolder& textures) const
{
	Projectile::Type type = isAllied() ? Projectile::AlliedBullet : Projectile::EnemyBullet;

	createProjectile(node, type, 0.0f, 0.0f, textures);
}

void Tank::createProjectile(SceneNode& node, Projectile::Type type, float xOffset, float yOffset, const TextureHolder& textures) const
{
	std::unique_ptr<Projectile> projectile(new Projectile(type, textures));

	sf::Vector2f offset(xOffset * mSprite.getGlobalBounds().width, yOffset * mSprite.getGlobalBounds().height);
	sf::Vector2f velocity(projectile->getMaxSpeed(), projectile->getMaxSpeed());

	float sign = isAllied() ? -1.f : +1.f;
	projectile->setPosition(getWorldPosition() + offset * sign);
	
	projectile->setVelocity(-velocity);
	projectile->setRotation(Tank::getTotalTurretRotation());
	
	node.attachChild(std::move(projectile));
}

void Tank::createPickup(SceneNode& node, const TextureHolder& textures) const
{
	auto type = static_cast<Pickup::Type>(randomInt(Pickup::TypeCount));

	std::unique_ptr<Pickup> pickup(new Pickup(type, textures));
	pickup->setPosition(getWorldPosition());
	pickup->setVelocity(0.f, 1.f);
	node.attachChild(std::move(pickup));
}

void Tank::updateTexts()
{
	// Display hitpoints
	if (isDestroyed())
	{
		mHealthDisplay->setString("");
	}
	else
	{
		mHealthDisplay->setString(toString(getHitpoints()) + " HP");
	}

	
	mHealthDisplay->setRotation(-getRotation());

	// Display ammo count
	if (ammoDisplay)
	{
		ammoDisplay->setRotation(-getRotation());

		if (isDestroyed())
		{
			ammoDisplay->setString("");
		}
		else
		{
			ammoDisplay->setString("Ammo: " + toString(ammoCount));
		}
	}
}


//updates countdown for boost if active, if finished disables update and returns multiplier to normal
void Tank::checkSpeedBoost(sf::Time dt)
{
	if (hasSpeedBoost)
	{
		if (speedBoostCountdown > sf::Time::Zero)
		{
			speedBoostCountdown -= dt;
		}
		else
		{
			hasSpeedBoost = false;
			speedBoostMultiplier = 1.0f;
			
		}
	}
}


//method to update turret rotation, continues moving with enough velocity, makes it seem heavy and realistic 
void Tank::updateTurret(sf::Time dt)
{
	turretSprite.rotate(turretRotationVelocity * dt.asSeconds());

	if (isRotating)
	{
		isTankRotating = true;
		isRotating = false;
	}
	else
	{
		turretRotationVelocity *= 0.9f;
		isTankRotating = false;
	}
}

void Tank::accelerateTurretRotation(float rotationVelocity)
{
	if (-getMaxTurretRotationSpeed() < turretRotationVelocity &&
		turretRotationVelocity < getMaxTurretRotationSpeed())
	{
		turretRotationVelocity += rotationVelocity;
	}

	isRotating = true;
}

void Tank::setTurretRotationVelocity(float rotationVelocity)
{
	turretRotationVelocity = rotationVelocity;
}

float Tank::getTotalTurretRotation() const
{
	return turretSprite.getRotation() + getRotation();
}

void Tank::guideTurretTowards(sf::Vector2f position)
{
	//mTargetDirection = unitVector(position - getWorldPosition());
}

//void Tank::rotationSound()
//{
	//if (isRotating)
	//{
	//	//commands.push(mFireCommand);

	//	//playLocalSound(commands, isAllied() ? SoundEffect::AlliedGunfire : SoundEffect::EnemyGunfire);
	//	playLocalSound(commands, SoundEffect::TankTurretRotate);
	//}
	//else
	//{
	//	return;
	//}
//}


//very important method. To be changed to use velocity so tanks dont lock on. Was unable to work out the maths
void Tank::updateEnemyTurretRotation(sf::Time dt)
{
	float angle = toDegree(std::atan2(mTargetDirection.y, mTargetDirection.x)) - 90;

	float differenceAngle = turretSprite.getRotation() - angle;

	turretSprite.setRotation(angle);		// - 90 to have it face the player.. forward is to the right on a sprite

	fire();
}