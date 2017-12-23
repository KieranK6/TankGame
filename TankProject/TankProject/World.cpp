#include "World.hpp"
#include "Projectile.hpp"
#include "Pickup.hpp"
#include "Foreach.hpp"
#include "TextNode.hpp"
#include "ParticleNode.hpp"
#include "SoundNode.hpp"
#include "NetworkNode.hpp"
#include "Utility.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include "SceneNode.hpp"
#include "Base.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>
#include <vector>


World::World(sf::RenderTarget& outputTarget, FontHolder& fonts, SoundPlayer& sounds, bool networked)
	: mTarget(outputTarget)
	, mSceneTexture()
	, mWorldView(outputTarget.getDefaultView())
	, mTextures()
	, mFonts(fonts)
	, mSounds(sounds)
	, mSceneGraph()
	, mSceneLayers()
	, mWorldBounds(0.f, 0.f, 1500.f, 2000.f)
	, mSpawnPosition(mWorldView.getSize().x / 2.f, mWorldBounds.height - mWorldView.getSize().y / 2.f)
	, mScrollSpeed(-50.f)
	, mScrollSpeedCompensation(1.f)
	, mPlayerTanks()
	, mEnemySpawnPoints()
	, mActiveEnemies()
	, mNetworkedWorld(networked)
	, mNetworkNode(nullptr)
	, mFinishSprite(nullptr)
	, isBaseDestroyed(false)
{
	mSceneTexture.create(mTarget.getSize().x, mTarget.getSize().y);

	loadTextures();
	buildScene();
	SpawnObstacles(6);

	//bool collisionSoundPlaying = false;
	
	SpawnEnemyBase();

	// Prepare the view
	mWorldView.setCenter(mSpawnPosition);
}

void World::setWorldScrollCompensation(float compensation)
{
	mScrollSpeedCompensation = compensation;
}

void World::update(sf::Time dt)
{
	// Move the world to player position
	centerWorldToPlayer();

	FOREACH(Tank* a, mPlayerTanks)
		a->setVelocity(0.f, 0.f);

	// Setup commands to destroy entities, and guide turrets
	destroyEntitiesOutsideView();
	enemyTurretTargeting();

	// Forward commands to scene graph, adapt velocity (scrolling, diagonal correction)
	while (!mCommandQueue.isEmpty())
		mSceneGraph.onCommand(mCommandQueue.pop(), dt);
	adaptPlayerVelocity();

	// Collision detection and response (may destroy entities)
	handleCollisions();

	// Remove tanks that were destroyed (World::removeWrecks() only destroys the entities, not the pointers in mPlayerTanks)
	auto firstToRemove = std::remove_if(mPlayerTanks.begin(), mPlayerTanks.end(), std::mem_fn(&Tank::isMarkedForRemoval));
	mPlayerTanks.erase(firstToRemove, mPlayerTanks.end());

	// Remove all destroyed entities, create new ones
	mSceneGraph.removeWrecks();
	spawnEnemies();
	

	// Regular update step, adapt position (correct if outside view)
	mSceneGraph.update(dt, mCommandQueue);
	adaptPlayerPosition();

	updateSounds();
}

void World::draw()
{
	if (PostEffect::isSupported())
	{
		mSceneTexture.clear();
		mSceneTexture.setView(mWorldView);
		mSceneTexture.draw(mSceneGraph);
		mSceneTexture.display();
		mBloomEffect.apply(mSceneTexture, mTarget);
	}
	else
	{
		mTarget.setView(mWorldView);
		mTarget.draw(mSceneGraph);
	}
}

CommandQueue& World::getCommandQueue()
{
	return mCommandQueue;
}

Tank* World::getTank(int identifier) const
{
	FOREACH(Tank* a, mPlayerTanks)
	{
		if (a->getIdentifier() == identifier)
			return a;
	}

	return nullptr;
}

void World::removeTank(int identifier)
{
	Tank* tank = getTank(identifier);
	if (tank)
	{
		tank->destroy();
		mPlayerTanks.erase(std::find(mPlayerTanks.begin(), mPlayerTanks.end(), tank));
	}
}

Tank* World::addTank(int identifier)
{
	std::unique_ptr<Tank> player(new Tank(Tank::Hotchkiss, mTextures, mFonts));
	player->setPosition(mWorldView.getCenter());
	player->setIdentifier(identifier);

	mPlayerTanks.push_back(player.get());
	mSceneLayers[UpperAir]->attachChild(std::move(player));
	return mPlayerTanks.back();
}
/*
Obstacle* World::addObstacle(int identifier)
{
	std::unique_ptr<Tank> player(new Tank(Tank::Hotchkiss, mTextures, mFonts));
	player->setPosition(mWorldView.getCenter());
	player->setIdentifier(identifier);

	mPlayerTanks.push_back(player.get());
	mSceneLayers[UpperAir]->attachChild(std::move(player));
	return mPlayerTanks.back();
}*/

void World::createPickup(sf::Vector2f position, Pickup::Type type)
{
	std::unique_ptr<Pickup> pickup(new Pickup(type, mTextures));
	pickup->setPosition(position);
	pickup->setVelocity(0.f, 1.f);
	mSceneLayers[UpperAir]->attachChild(std::move(pickup));
}

bool World::pollGameAction(GameActions::Action& out)
{
	return mNetworkNode->pollGameAction(out);
}

void World::setCurrentBattleFieldPosition(float lineY)
{
	mWorldView.setCenter(mWorldView.getCenter().x, lineY - mWorldView.getSize().y / 2);
	mSpawnPosition.y = mWorldBounds.height;
}

void World::setWorldHeight(float height)
{
	mWorldBounds.height = height;
}

bool World::hasAlivePlayer() const
{
	return mPlayerTanks.size() > 0;
}

bool World::hasBaseBeenDestroyed() const
{
	return isBaseDestroyed;
}

void World::loadTextures()
{
	mTextures.load(Textures::Entities, "Media/Textures/Entities.png");
	mTextures.load(Textures::TankChassisEntities, "Media/Textures/EntitiesTankChassis.png");
	mTextures.load(Textures::TankTurretEntities, "Media/Textures/EntitiesTankTurret.png");
	mTextures.load(Textures::Desert, "Media/Textures/Desert.png");
	mTextures.load(Textures::Explosion, "Media/Textures/Explosion.png");
	mTextures.load(Textures::Particle, "Media/Textures/Particle.png");
	mTextures.load(Textures::FinishLine, "Media/Textures/FinishLine.png");
	mTextures.load(Textures::Obstacles, "Media/Textures/obstacles.png"); 
	mTextures.load(Textures::EnemyBase, "Media/Textures/base.png");
}

void World::adaptPlayerPosition()
{
	// Keep player's position inside the screen bounds, at least borderDistance units from the border
	sf::FloatRect viewBounds = getViewBounds();
	const float borderDistance = 40.f;

	FOREACH(Tank* tank, mPlayerTanks)
	{
		sf::Vector2f position = tank->getPosition();
		position.x = std::max(position.x, viewBounds.left + borderDistance);
		position.x = std::min(position.x, viewBounds.left + viewBounds.width - borderDistance);
		position.y = std::max(position.y, viewBounds.top + borderDistance);
		position.y = std::min(position.y, viewBounds.top + viewBounds.height - borderDistance);
		tank->setPosition(position);
	}
}

void World::adaptPlayerVelocity()
{
	FOREACH(Tank* tank, mPlayerTanks)
	{
		sf::Vector2f velocity = tank->getVelocity();

		// If moving diagonally, reduce velocity (to have always same velocity)
		if (velocity.x != 0.f && velocity.y != 0.f)
			tank->setVelocity(velocity / std::sqrt(2.f));
	}
}

bool matchesCategories(SceneNode::Pair& colliders, Category::Type type1, Category::Type type2)
{
	unsigned int category1 = colliders.first->getCategory();
	unsigned int category2 = colliders.second->getCategory();

	// Make sure first pair entry has category type1 and second has type2
	if (type1 & category1 && type2 & category2)
	{
		return true;
	}
	else if (type1 & category2 && type2 & category1)
	{
		std::swap(colliders.first, colliders.second);
		return true;
	}
	else
	{
		return false;
	}
}

void World::handleCollisions()
{
	std::set<SceneNode::Pair> collisionPairs;
	mSceneGraph.checkSceneCollision(mSceneGraph, collisionPairs);

	FOREACH(SceneNode::Pair pair, collisionPairs)
	{
		if (matchesCategories(pair, Category::PlayerTank, Category::EnemyTank))
		{
			auto& player = static_cast<Tank&>(*pair.first);
			auto& enemy = static_cast<Tank&>(*pair.second);

			// Collision: Player damage = enemy's remaining HP
			player.damage(enemy.getHitpoints());
			enemy.destroy();
		}

		else if (matchesCategories(pair, Category::PlayerTank, Category::Pickup))
		{
			auto& player = static_cast<Tank&>(*pair.first);
			auto& pickup = static_cast<Pickup&>(*pair.second);

			// Apply pickup effect to player, destroy projectile
			pickup.apply(player);
			pickup.destroy();
			player.playLocalSound(mCommandQueue, SoundEffect::CollectPickup);
		}

		else if (matchesCategories(pair, Category::PlayerTank, Category::Obstacle))
		{
			auto& player = static_cast<Tank&>(*pair.first);
			auto& obstacle = static_cast<Obstacle&>(*pair.second);

			// Apply pickup effect to player, destroy projectile
			//std::cout << "Collision!!" << std::endl;
			player.setVelocity(0.f, 0.f);
			//collisionSoundPlaying = true;
			player.playLocalSound(mCommandQueue, SoundEffect::Collision);
		}

		else if (matchesCategories(pair, Category::Obstacle, Category::AlliedProjectile)
			|| matchesCategories(pair, Category::Obstacle, Category::EnemyProjectile))
		{
			auto& obstacle = static_cast<Tank&>(*pair.first);
			auto& projectile = static_cast<Projectile&>(*pair.second);

			// Destroy projectile
			projectile.destroy();
		}

		else if (matchesCategories(pair, Category::Base, Category::AlliedProjectile))
		{
			auto& base = static_cast<Base&>(*pair.first);
			auto& projectile = static_cast<Projectile&>(*pair.second);

			base.damage(projectile.getDamage());

			if (base.isDestroyed())
				isBaseDestroyed = true;

			projectile.destroy();
		}

		else if (matchesCategories(pair, Category::EnemyTank, Category::AlliedProjectile)
			|| matchesCategories(pair, Category::PlayerTank, Category::EnemyProjectile))
		{
			auto& tank = static_cast<Tank&>(*pair.first);
			auto& projectile = static_cast<Projectile&>(*pair.second);

			// Apply projectile damage to tank, destroy projectile
			tank.damage(projectile.getDamage());
			projectile.destroy();
		}
	}
}

void World::updateSounds()
{
	sf::Vector2f listenerPosition;

	// 0 players (multiplayer mode, until server is connected) -> view center
	if (mPlayerTanks.empty())
	{
		listenerPosition = mWorldView.getCenter();
	}

	// 1 or more players -> mean position between all aircrafts
	else
	{
		FOREACH(Tank* tank, mPlayerTanks)
			listenerPosition += tank->getWorldPosition();

		listenerPosition /= static_cast<float>(mPlayerTanks.size());
	}

	// Set listener's position
	mSounds.setListenerPosition(listenerPosition);

	// Remove unused sounds
	mSounds.removeStoppedSounds();
}

void World::buildScene()
{
	// Initialize the different layers
	for (std::size_t i = 0; i < LayerCount; ++i)
	{
		Category::Type category = (i == LowerAir) ? Category::SceneAirLayer : Category::None;

		SceneNode::Ptr layer(new SceneNode(category));
		mSceneLayers[i] = layer.get();

		mSceneGraph.attachChild(std::move(layer));
	}

	// Prepare the tiled background
	sf::Texture& desertTexture = mTextures.get(Textures::Desert);
	desertTexture.setRepeated(true);

	float viewHeight = mWorldView.getSize().y;
	sf::IntRect textureRect(mWorldBounds);
	textureRect.height += static_cast<int>(viewHeight);

	// Add the background sprite to the scene
	std::unique_ptr<SpriteNode> jungleSprite(new SpriteNode(desertTexture, textureRect));
	jungleSprite->setPosition(mWorldBounds.left, mWorldBounds.top - viewHeight);
	mSceneLayers[Background]->attachChild(std::move(jungleSprite));

	// Add the finish line to the scene
	//sf::Texture& finishTexture = mTextures.get(Textures::FinishLine);
	//std::unique_ptr<SpriteNode> finishSprite(new SpriteNode(finishTexture));
	//finishSprite->setPosition(0.f, -76.f);
	//mSceneLayers[Background]->attachChild(std::move(finishSprite));


	// Add particle node to the scene
	std::unique_ptr<ParticleNode> smokeNode(new ParticleNode(Particle::Smoke, mTextures));
	mSceneLayers[LowerAir]->attachChild(std::move(smokeNode));

	// Add propellant particle node to the scene
	std::unique_ptr<ParticleNode> propellantNode(new ParticleNode(Particle::Propellant, mTextures));
	mSceneLayers[LowerAir]->attachChild(std::move(propellantNode));

	//Add sound effect node
	std::unique_ptr<SoundNode> soundNode(new SoundNode(mSounds));
	mSceneGraph.attachChild(std::move(soundNode));

	// Add network node, if necessary
	if (mNetworkedWorld)
	{
		std::unique_ptr<NetworkNode> networkNode(new NetworkNode());
		mNetworkNode = networkNode.get();
		mSceneGraph.attachChild(std::move(networkNode));
	}

	// Add enemy tank
	addEnemies(10);
}

void World::addEnemies(int enemyCount)
{
	if (mNetworkedWorld)
		return;

	std::vector<float> xPositions;
	xPositions.push_back(mWorldBounds.width / 4 * 1);
	xPositions.push_back(mWorldBounds.width / 4 * 2);
	xPositions.push_back(mWorldBounds.width / 4 * 3);

	float currentValue = 0;
	int xPos = 0;
	int randomEnemyType = 0;

	Tank::Type spawnType = Tank::Type::Panzer;

	for (int i = 0; i < enemyCount; i++)
	{
		xPos = rand() % 3; //Random number between 1 and 3, for getting an xPos for obstacle spawning
		randomEnemyType = rand() % 2 + 1; //Random num between 1 and 2, for getting a random type of enemy
		currentValue = xPositions.at(xPos); //sets current x value to one of the three possible values form xPositions vector

		if (randomEnemyType == 1)
		{
			spawnType = Tank::Type::Panther;
		}

		addEnemy(spawnType, xPositions[xPos], 700 + (300 * i));
	}


	// Sort all enemies according to their y value, such that lower enemies are checked first for spawning
	sortEnemies();
}

void World::sortEnemies()
{
	// Sort all enemies according to their y value, such that lower enemies are checked first for spawning
	std::sort(mEnemySpawnPoints.begin(), mEnemySpawnPoints.end(), [](SpawnPoint lhs, SpawnPoint rhs)
	{
		return lhs.y < rhs.y;
	});
}

void World::SpawnEnemyBase()
{
	//Base m(Base::EnemyBase, mTextures, mFonts);
	std::unique_ptr<Base> base1(new Base(Base::EnemyBase, mTextures, mFonts));
	base1->setPosition(mWorldBounds.width / 2 - base1->getBoundingRect().width/2, 0.f);
	mSceneLayers[Background]->attachChild(std::move(base1));
}



void World::SpawnObstacles(int obstacleCount)
{
	std::vector<float> xPositions;
	xPositions.push_back(150.f);
	xPositions.push_back(400.f);
	xPositions.push_back(750.f);
	float currentValue = 0;
	int xPos = 0;
	int randomObsIndex = 0;


	for (int i = 0; i < obstacleCount; i++)
	{
		xPos = rand() % 3; //Random number between 1 and 3, for getting an xPos for obstacle spawning
		randomObsIndex = rand() % 2 + 1; //Random num between 1 and 2, for getting a random type of obstacle from vector of obstacle types
		currentValue = xPositions.at(xPos); //sets current x value to one of the three possible values form xPositions vector

		Obstacle::ObType barricade = Obstacle::Barricade;

		if (randomObsIndex == 1)
		{
			barricade = Obstacle::Stone;
		}

		std::unique_ptr<Obstacle> ob1(new Obstacle(barricade, mTextures)); //creates a pointer to an Obstacle object with a random texture
		ob1->setPosition(currentValue + (i * 4), 400 + (200 * i)); //Spawns in obstacles at varied positions
		ob1->setScale(.5f, .5f); //scales down object as too big by default
		mSceneLayers[Background]->attachChild(std::move(ob1)); //attaches current object as child to the Background scene layer
	}
}


void World::addEnemy(Tank::Type type, float relX, float relY)
{
	SpawnPoint spawn(type, relX, relY);
	mEnemySpawnPoints.push_back(spawn);
}

void World::spawnEnemies()
{
	// Spawn all enemies entering the view area (including distance) this frame
	while (!mEnemySpawnPoints.empty()
		&& mEnemySpawnPoints.back().y > getBattlefieldBounds().top)
	{
		SpawnPoint spawn = mEnemySpawnPoints.back();

		std::unique_ptr<Tank> enemy(new Tank(spawn.type, mTextures, mFonts));
		enemy->setPosition(spawn.x, spawn.y);
		enemy->setRotation(180.f);

		if (mNetworkedWorld) enemy->disablePickups();

		mSceneLayers[UpperAir]->attachChild(std::move(enemy));

		// Enemy is spawned, remove from the list to spawn
		mEnemySpawnPoints.pop_back();
	}
}

void World::centerWorldToPlayer()
{
	playerPositionUpdate = mPlayerTanks.at(0)->getPosition();
	worldPositionUpdate = mWorldView.getCenter();

	if ((playerPositionUpdate.x + mWorldView.getSize().x / 2 < mWorldBounds.left + mWorldBounds.width)
		&& (playerPositionUpdate.x - mWorldView.getSize().x / 2 > mWorldBounds.left))
	{
		worldPositionUpdate.x = playerPositionUpdate.x;
	}

	if ((playerPositionUpdate.y + mWorldView.getSize().y / 2 < mWorldBounds.top + mWorldBounds.height)
		&& (playerPositionUpdate.y - mWorldView.getSize().y / 2 > mWorldBounds.top))
	{
		worldPositionUpdate.y = playerPositionUpdate.y;
	}

	mWorldView.setCenter(worldPositionUpdate);
}

void World::destroyEntitiesOutsideView()
{
	Command command;
	command.category = Category::Projectile; // | Category::EnemyTank;
	command.action = derivedAction<Entity>([this](Entity& e, sf::Time)
	{
		if (!getBattlefieldBounds().intersects(e.getBoundingRect()))
			e.remove();
	});

	mCommandQueue.push(command);
}

void World::enemyTurretTargeting()
{
	// Setup command that stores all enemys in active enemies
	Command enemyCollector;
	enemyCollector.category = Category::EnemyTank;
	enemyCollector.action = derivedAction<Tank>([this](Tank& enemy, sf::Time)
	{
		if (!enemy.isDestroyed())
			mActiveEnemies.push_back(&enemy);
	});

	// Setup command that targets turrets toward player
	Command turretGuider;
	turretGuider.category = Category::EnemyTank;
	turretGuider.action = derivedAction<Tank>([this](Tank& tank, sf::Time)
	{
		Tank* player = mPlayerTanks.at(0);

		tank.guideTurretTowards(player->getWorldPosition());
	});

	// Push commands, reset active enemies
	mCommandQueue.push(enemyCollector);
	mCommandQueue.push(turretGuider);
	mActiveEnemies.clear();
}

void World::guideMissiles()
{
	// Setup command that stores all enemies in mActiveEnemies
	Command enemyCollector;
	enemyCollector.category = Category::EnemyTank;
	enemyCollector.action = derivedAction<Tank>([this](Tank& enemy, sf::Time)
	{
		if (!enemy.isDestroyed())
			mActiveEnemies.push_back(&enemy);
	});
}

sf::FloatRect World::getViewBounds() const
{
	return sf::FloatRect(mWorldView.getCenter() - mWorldView.getSize() / 2.f, mWorldView.getSize());
}

sf::FloatRect World::getBattlefieldBounds() const
{
	// Return view bounds + some area at top, where enemies spawn
	sf::FloatRect bounds = getViewBounds();
	bounds.top -= 100.f;
	bounds.height += 100.f;

	return bounds;
}
