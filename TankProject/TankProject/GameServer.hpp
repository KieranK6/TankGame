#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Thread.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include <vector>
#include <memory>
#include <map>

class GameServer
{
public:
	explicit							GameServer(sf::Vector2f battlefieldSize);
	~GameServer();

	void								notifyPlayerSpawn(sf::Int32 tankIdentifier);
	void								notifyPlayerRealtimeChange(sf::Int32 tankIdentifier, sf::Int32 action, bool actionEnabled);
	void								notifyPlayerEvent(sf::Int32 tankIdentifier, sf::Int32 action);

private:
	// A GameServerRemotePeer refers to one instance of the game, may it be local or from another computer
	struct RemotePeer
	{
		RemotePeer();

		sf::TcpSocket			socket;
		sf::Time				lastPacketTime;
		std::vector<sf::Int32>	tankIdentifiers;
		bool					ready;
		bool					timedOut;
	};

	// Structure to store information about current tank state
	struct TankInfo
	{
		bool						isLiberator;
		sf::Vector2f				position;
		float						tankRotation;
		float						turretRotation;
		sf::Int32					hitpoints;
		sf::Int32                   missileAmmo;
		std::map<sf::Int32, bool>	realtimeActions;
	};

	// Unique pointer to remote peers
	typedef std::unique_ptr<RemotePeer> PeerPtr;


private:
	void								setListening(bool enable);
	void								executionThread();
	void								tick();
	

	void								handleIncomingPackets();
	void								handleIncomingPacket(sf::Packet& packet, RemotePeer& receivingPeer, bool& detectedTimeout);

	void								handleIncomingConnections();
	void								handleDisconnections();

	void								informWorldState(sf::TcpSocket& socket);
	void								broadcastMessage(const std::string& message);
	void								sendToAll(sf::Packet& packet);
	void								updateClientState();
	sf::Vector2f						getSpawnLocation(bool isLiberator, int tankIdentifier);

public:
	sf::Time							now() const;


private:
	sf::Thread							mThread;
	sf::Clock							mClock;
	sf::TcpListener						mListenerSocket;
	bool								mListeningState;
	sf::Time							mClientTimeoutTime;

	std::size_t							mMaxConnectedPlayers;
	std::size_t							mConnectedPlayers;

	sf::FloatRect						mBattleFieldRect;
	float								mBattleFieldScrollSpeed;

	std::size_t							mTankCount;
	std::map<sf::Int32, TankInfo>		mTankInfo;

	std::vector<PeerPtr>				mPeers;
	sf::Int32							mTankIdentifierCounter;
	bool								mWaitingThreadEnd;

	sf::Time							mLastSpawnTime;
	sf::Time							mTimeForNextSpawn;
};