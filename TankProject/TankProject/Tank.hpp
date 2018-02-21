#pragma once
#include "Entity.hpp"
#include "Command.hpp"
#include "ResourceIdentifiers.hpp"
#include "Projectile.hpp"
#include "TextNode.hpp"
#include "Animation.hpp"


#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Sprite.hpp>

//This class was worked on by Ciaran Mooney


class Tank : public Entity
{
public:
	enum Type
	{
		Hotchkiss,
		T34,
		Panzer,
		Panther,
		HotchkissTurret,
		T34Turret,
		PanzerTurret,
		PantherTurret,
		TypeCount
	};


public:
	Tank(Type type, const TextureHolder& textures, const FontHolder& fonts);

	virtual unsigned int	getCategory() const;
	virtual sf::FloatRect	getBoundingRect() const;
	virtual void			remove();
	virtual bool 			isMarkedForRemoval() const;
	bool					isAllied() const;
	float					getMaxSpeed() const;
	float					getMaxTurretRotationSpeed() const;
	float					getTurretRotationSpeed() const;
	float					getSpeedBoost() const;
	void					disablePickups();
	float					getTurretRotation();
	void					setTurretRotation(float rotation);
	Tank::Type				getType();
	Tank::Type				getAllyType();

	float					getTankRadius();
	

	sf::CircleShape Tank::getBoundingCircle() const;

	void					increaseFireRate();
	//void					increaseSpread();
	void					collectAmmo(unsigned int count);
	void					increaseHealth(int amount);
	void					increaseTankSpeed(float lengthInSeconds);

	void 					fire();
	//void					launchMissile();
	void					playLocalSound(CommandQueue& commands, SoundEffect::ID effect);
	int						getIdentifier();
	void					setIdentifier(int identifier);
	int						getMissileAmmo() const;
	void					setMissileAmmo(int ammo);
	int						getAmmoCount() const;
	void					accelerateTurretRotation(float rotationVelocity);
	void					setTurretRotationVelocity(float rotationVelocity);
	void					guideTurretTowards(sf::Vector2f position);


private:
	virtual void			drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
	virtual void 			updateCurrent(sf::Time dt, CommandQueue& commands);
	void					updateMovementPattern(sf::Time dt);
	void					checkPickupDrop(CommandQueue& commands);
	void					checkProjectileLaunch(sf::Time dt, CommandQueue& commands);
	void					checkSpeedBoost(sf::Time dt);

	void					createBullets(SceneNode& node, const TextureHolder& textures) const;
	void					createProjectile(SceneNode& node, Projectile::Type type, float xOffset, float yOffset, const TextureHolder& textures) const;
	void					createPickup(SceneNode& node, const TextureHolder& textures) const;

	void					updateTexts();
	void					updateTurret(sf::Time dt);
	float					getTotalTurretRotation() const;
	void					updateEnemyTurretRotation(sf::Time dt);


private:
	Type					mType;
	sf::Sprite				mSprite;
	Animation				mExplosion;
	Command 				mFireCommand;
	Command					mMissileCommand;
	sf::Time				mFireCountdown;
	bool 					mIsFiring;
	bool					mIsLaunchingMissile;
	bool 					mShowExplosion;
	bool					mExplosionBegan;
	bool					mPlayedExplosionSound;
	bool					mSpawnedPickup;
	bool					mPickupsEnabled;

	float					mRadius = 26.f;

	int						mFireRateLevel;
	int						mSpreadLevel;
	int						mMissileAmmo;
	bool					hasSpeedBoost;
	sf::Time				speedBoostCountdown;
	float					speedBoostMultiplier;

	Command 				mDropPickupCommand;
	float					mTravelledDistance;
	std::size_t				mDirectionIndex;
	TextNode*				mHealthDisplay;
	TextNode*				ammoDisplay;

	int						mIdentifier;

	int						ammoCount;

	Type					turretType;
	sf::Sprite				turretSprite;
	float					turretRotationVelocity;
	float					turretOldRotation;
	bool					isRotating;
	sf::Vector2f			mTargetDirection;
	bool					directionChange;
};