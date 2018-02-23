#pragma once
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"
#include "SceneNode.hpp"
#include "SpriteNode.hpp"
#include "Tank.hpp"
#include "Tank.hpp"
#include "Obstacle.hpp"
#include "CommandQueue.hpp"
#include "Command.hpp"
#include "Pickup.hpp"
#include "BloomEffect.hpp"
#include "SoundPlayer.hpp"
#include "NetworkProtocol.hpp"
#include "Base.hpp"

#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <array>
#include <queue>

//Foward declaration
namespace sf
{
	class RenderTarget;
}

class NetworkNode;
class Base;

class World : private sf::NonCopyable
{
public:
	explicit World(sf::RenderTarget& window, FontHolder& font, SoundPlayer& sounds, bool networked = false);
	void update(sf::Time dt);
	void draw();

	sf::FloatRect getViewBounds() const;
	CommandQueue& getCommandQueue();
	Tank* addTank(int identifier, Tank::Type type);
	void removeTank(int identifier);
	void setCurrentBattleFieldPosition(float lineY);
	void setWorldHeight(float height);
	void centerWorldToPlayer(Tank* centredTo);
	void addEnemy(Tank::Type type, float relX, float relY);
	void sortEnemies();

	bool hasAlivePlayer() const;
	bool hasBaseBeenDestroyed() const;
	bool hasResistanceBaseBeenDestroyed() const;
	bool hasLiberationBaseBeenDestroyed() const;


	void setWorldScrollCompensation(float compensation);
	void adaptLiberatorTankPosition(Tank* player);


	Tank* getTank(int identifier) const;
	sf::FloatRect getBattlefieldBounds() const;

	void createPickup(sf::Vector2f position, Pickup::Type type);
	bool pollGameAction(GameActions::Action& out);

private:
	void loadTextures();
	void adaptTankPositions();
	void adaptPlayerVelocity();
	void handleCollisions();
	void handleCircleCollions(Tank&, Obstacle&);
	void updateSounds();

	void SpawnObstacles(int obstacleCount);
	void SpawnEnemyBase();
	void buildScene();
	void addEnemies(int enemyCount);
	void spawnEnemies();
	//void drawRingAroundPlayer();
	void destroyEntitiesOutsideView();
	void guideMissiles();

	void enemyTurretTargeting();
	void updateBase();
	void SpawnBase();

	sf::Vector2f normaliseVector(sf::Vector2f passedVector, float magnitude);
	float getMagnitude(sf::Vector2f passedVelocity);

private:
	enum Layer
	{
		Background,
		Obstacles,
		LowerAir,
		UpperAir,
		LayerCount
	};

	struct SpawnPoint
	{
		SpawnPoint(Tank::Type type, float x, float y)
			: type(type)
			, x(x)
			, y(y)
		{
		}

		Tank::Type type;
		float x;
		float y;
	};


private:
	sf::RenderTarget&					mTarget;
	sf::RenderTexture					mSceneTexture;
	sf::View							mWorldView;
	TextureHolder						mTextures;
	FontHolder&							mFonts;
	SoundPlayer&						mSounds;

	SceneNode							mSceneGraph;
	std::array<SceneNode*, LayerCount>	mSceneLayers;
	CommandQueue						mCommandQueue;

	sf::FloatRect						mWorldBounds;
	sf::Vector2f						mSpawnPosition;
	float								mScrollSpeed;
	float								mScrollSpeedCompensation;
	std::vector<Tank*>					mLiberatorTanks;

	sf::Vector2f						playerPositionUpdate;
	sf::Vector2f						worldPositionUpdate;

	std::vector<SpawnPoint>				mEnemySpawnPoints;
	std::vector<Tank*>					mActiveEnemies;
	
	//sf::Vertex							line[100];
	//bool								circleSetUp = false;

	BloomEffect							mBloomEffect;

	bool								mNetworkedWorld;
	NetworkNode*						mNetworkNode;
	SpriteNode*							mFinishSprite;

	bool								isBaseDestroyed;
	bool								isResistanceBaseDestroyed;
	bool								isLiberationBaseDestroyed;
};

