#include "GameServer.hpp"
#include "NetworkProtocol.hpp"
#include "Foreach.hpp"
#include "Utility.hpp"
#include "Pickup.hpp"
#include "Tank.hpp"

#include <SFML/Network/Packet.hpp>

GameServer::RemotePeer::RemotePeer()
	: ready(false)
	, timedOut(false)
{
	socket.setBlocking(false);
}

GameServer::GameServer(sf::Vector2f battlefieldSize)
	: mThread(&GameServer::executionThread, this)
	, mListeningState(false)
	, mClientTimeoutTime(sf::seconds(3.f))
	, mMaxConnectedPlayers(16)
	, mConnectedPlayers(0)
	, mBattleFieldRect(0.f, 0.f, battlefieldSize.x, battlefieldSize.y)
	, mBattleFieldScrollSpeed(-50.f)
	, mTankCount(0)
	, mPeers(1)
	, mTankIdentifierCounter(1)
	, mWaitingThreadEnd(false)
	, mLastSpawnTime(sf::Time::Zero)
	, mTimeForNextSpawn(sf::seconds(5.f))
{
	mListenerSocket.setBlocking(false);
	mPeers[0].reset(new RemotePeer());
	mThread.launch();
}

GameServer::~GameServer()
{
	mWaitingThreadEnd = true;
	mThread.wait();
}

void GameServer::notifyPlayerRealtimeChange(sf::Int32 tankIdentifier, sf::Int32 action, bool actionEnabled)
{
	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::PlayerRealtimeChange);
			packet << tankIdentifier;
			packet << action;
			packet << actionEnabled;

			mPeers[i]->socket.send(packet);
		}
	}
}

void GameServer::notifyPlayerEvent(sf::Int32 tankIdentifier, sf::Int32 action)
{
	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::PlayerEvent);
			packet << tankIdentifier;
			packet << action;

			mPeers[i]->socket.send(packet);
		}
	}
}

void GameServer::notifyPlayerSpawn(sf::Int32 tankIdentifier)
{
	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::PlayerConnect);
			packet << tankIdentifier 
				<< mTankInfo[tankIdentifier].isLiberator 
				<< mTankInfo[tankIdentifier].position.x 
				<< mTankInfo[tankIdentifier].position.y 
				<< mTankInfo[tankIdentifier].tankRotation
				<< mTankInfo[tankIdentifier].turretRotation;
			mPeers[i]->socket.send(packet);
		}
	}
}

void GameServer::setListening(bool enable)
{
	// Check if it isn't already listening
	if (enable)
	{
		if (!mListeningState)
			mListeningState = (mListenerSocket.listen(ServerPort) == sf::TcpListener::Done);
	}
	else
	{
		mListenerSocket.close();
		mListeningState = false;
	}
}

void GameServer::executionThread()
{
	setListening(true);

	sf::Time stepInterval = sf::seconds(1.f / 60.f);
	sf::Time stepTime = sf::Time::Zero;
	sf::Time tickInterval = sf::seconds(1.f / 20.f);
	sf::Time tickTime = sf::Time::Zero;
	sf::Clock stepClock, tickClock;

	while (!mWaitingThreadEnd)
	{
		handleIncomingPackets();
		handleIncomingConnections();

		stepTime += stepClock.getElapsedTime();
		stepClock.restart();

		tickTime += tickClock.getElapsedTime();
		tickClock.restart();

		// Fixed update step
		while (stepTime >= stepInterval)
		{
			mBattleFieldRect.top += mBattleFieldScrollSpeed * stepInterval.asSeconds();
			stepTime -= stepInterval;
		}

		// Fixed tick step
		while (tickTime >= tickInterval)
		{
			tick();
			tickTime -= tickInterval;
		}

		// Sleep to prevent server from consuming 100% CPU
		sf::sleep(sf::milliseconds(100));
	}
}

void GameServer::tick()
{
	updateClientState();

	// Check for mission success = all planes with position.y < offset
	//bool allAircraftsDone = true;
	//FOREACH(auto pair, mTankInfo)
	//{
	//	// As long as one player has not crossed the finish line yet, set variable to false
	//	if (pair.second.position.y > 0.f)
	//		allAircraftsDone = false;
	//}
	//if (allAircraftsDone)
	//{
	//	sf::Packet missionSuccessPacket;
	//	missionSuccessPacket << static_cast<sf::Int32>(Server::MissionSuccess);
	//	sendToAll(missionSuccessPacket);
	//}

	// Remove IDs of tank that have been destroyed (relevant if a client has two, and loses one)
	for (auto itr = mTankInfo.begin(); itr != mTankInfo.end(); )
	{
		if (itr->second.hitpoints <= 0)
			mTankInfo.erase(itr++);
		else
			++itr;
	}

	//// Check if its time to attempt to spawn enemies
	//if (now() >= mTimeForNextSpawn + mLastSpawnTime)
	//{
	//	// No more enemies are spawned near the end
	//	if (mBattleFieldRect.top > 600.f)
	//	{
	//		std::size_t enemyCount = 1u + randomInt(2);
	//		float spawnCenter = static_cast<float>(randomInt(500) - 250);

	//		// In case only one enemy is being spawned, it appears directly at the spawnCenter
	//		float planeDistance = 0.f;
	//		float nextSpawnPosition = spawnCenter;

	//		// In case there are two enemies being spawned together, each is spawned at each side of the spawnCenter, with a minimum distance
	//		if (enemyCount == 2)
	//		{
	//			planeDistance = static_cast<float>(150 + randomInt(250));
	//			nextSpawnPosition = spawnCenter - planeDistance / 2.f;
	//		}

	//		// Send the spawn orders to all clients
	//		for (std::size_t i = 0; i < enemyCount; ++i)
	//		{
	//			sf::Packet packet;
	//			packet << static_cast<sf::Int32>(Server::SpawnEnemy);
	//			packet << static_cast<sf::Int32>(1 + randomInt(Tank::TypeCount - 1));
	//			packet << mWorldHeight - mBattleFieldRect.top + 500;
	//			packet << nextSpawnPosition;

	//			nextSpawnPosition += planeDistance / 2.f;

	//			sendToAll(packet);
	//		}

	//		mLastSpawnTime = now();
	//		mTimeForNextSpawn = sf::milliseconds(2000 + randomInt(6000));
	//	}
	//}
}

sf::Time GameServer::now() const
{
	return mClock.getElapsedTime();
}

void GameServer::handleIncomingPackets()
{
	bool detectedTimeout = false;

	FOREACH(PeerPtr& peer, mPeers)
	{
		if (peer->ready)
		{
			sf::Packet packet;
			while (peer->socket.receive(packet) == sf::Socket::Done)
			{
				// Interpret packet and react to it
				handleIncomingPacket(packet, *peer, detectedTimeout);

				// Packet was indeed received, update the ping timer
				peer->lastPacketTime = now();
				packet.clear();
			}

			if (now() >= peer->lastPacketTime + mClientTimeoutTime)
			{
				peer->timedOut = true;
				detectedTimeout = true;
			}
		}
	}

	if (detectedTimeout)
		handleDisconnections();
}

void GameServer::handleIncomingPacket(sf::Packet& packet, RemotePeer& receivingPeer, bool& detectedTimeout)
{
	sf::Int32 packetType;
	packet >> packetType;

	switch (packetType)
	{
	case Client::Quit:
	{
		receivingPeer.timedOut = true;
		detectedTimeout = true;
	} break;

	case Client::PlayerEvent:
	{
		sf::Int32 tankIdentifier;
		sf::Int32 action;
		packet >> tankIdentifier >> action;

		notifyPlayerEvent(tankIdentifier, action);
	} break;

	case Client::PlayerRealtimeChange:
	{
		sf::Int32 tankIdentifier;
		sf::Int32 action;
		bool actionEnabled;
		packet >> tankIdentifier >> action >> actionEnabled;
		mTankInfo[tankIdentifier].realtimeActions[action] = actionEnabled;
		notifyPlayerRealtimeChange(tankIdentifier, action, actionEnabled);
	} break;

	case Client::RequestCoopPartner:
	{
		bool isLiberator;
		packet >> isLiberator;
		receivingPeer.tankIdentifiers.push_back(mTankIdentifierCounter);
		mTankInfo[mTankIdentifierCounter].position = getSpawnLocation(isLiberator, mTankIdentifierCounter);
		mTankInfo[mTankIdentifierCounter].hitpoints = 100;
		mTankInfo[mTankIdentifierCounter].missileAmmo = 20;
		mTankInfo[mTankIdentifierCounter].tankRotation = (isLiberator == true ? 90.f : -90.f);
		mTankInfo[mTankIdentifierCounter].turretRotation = 0;
		mTankInfo[mTankIdentifierCounter].isLiberator = isLiberator;

		sf::Packet requestPacket;
		requestPacket << static_cast<sf::Int32>(Server::AcceptCoopPartner);
		requestPacket << mTankIdentifierCounter;
		requestPacket << isLiberator;
		requestPacket << mTankInfo[mTankIdentifierCounter].position.x;
		requestPacket << mTankInfo[mTankIdentifierCounter].position.y;

		receivingPeer.socket.send(requestPacket);
		mTankCount++;

		// Inform every other peer about this new tank
		FOREACH(PeerPtr& peer, mPeers)
		{
			if (peer.get() != &receivingPeer && peer->ready)
			{
				sf::Packet notifyPacket;
				notifyPacket << static_cast<sf::Int32>(Server::PlayerConnect);
				notifyPacket << mTankIdentifierCounter;
				notifyPacket << mTankInfo[mTankIdentifierCounter].isLiberator;
				notifyPacket << mTankInfo[mTankIdentifierCounter].position.x;
				notifyPacket << mTankInfo[mTankIdentifierCounter].position.y;
				notifyPacket << mTankInfo[mTankIdentifierCounter].tankRotation;
				notifyPacket << mTankInfo[mTankIdentifierCounter].turretRotation;
				peer->socket.send(notifyPacket);
			}
		}
		mTankIdentifierCounter++;
	} break;

	case Client::PositionUpdate:
	{
		sf::Int32 numTanks;
		packet >> numTanks;

		for (sf::Int32 i = 0; i < numTanks; ++i)
		{
			sf::Int32 tankIdentifier;
			sf::Int32 tankHitpoints;
			sf::Int32 missileAmmo;
			sf::Vector2f tankPosition;
			float tankRotation, turretRotation;
			packet >> tankIdentifier >> tankPosition.x >> tankPosition.y >> tankRotation >> turretRotation >> tankHitpoints >> missileAmmo;
			mTankInfo[tankIdentifier].position = tankPosition;
			mTankInfo[tankIdentifier].tankRotation = tankRotation;
			mTankInfo[tankIdentifier].turretRotation = turretRotation;
			mTankInfo[tankIdentifier].hitpoints = tankHitpoints;
			mTankInfo[tankIdentifier].missileAmmo = missileAmmo;
		}
	} break;

	case Client::GameEvent:
	{
		sf::Int32 action;
		float x;
		float y;

		packet >> action;
		packet >> x;
		packet >> y;

		

		// Enemy explodes: With certain probability, drop pickup
		// To avoid multiple messages spawning multiple pickups, only listen to first peer (host)
		if (action == GameActions::EnemyExplode && randomInt(3) == 0 && &receivingPeer == mPeers[0].get())
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::SpawnPickup);
			packet << static_cast<sf::Int32>(randomInt(Pickup::TypeCount));
			packet << x;
			packet << y;

			sendToAll(packet);
		}
	}
	}
}

void GameServer::updateClientState()
{
	sf::Packet updateClientStatePacket;
	updateClientStatePacket << static_cast<sf::Int32>(Server::UpdateClientState);
	updateClientStatePacket << static_cast<float>(mBattleFieldRect.top + mBattleFieldRect.height);
	updateClientStatePacket << static_cast<sf::Int32>(mTankInfo.size());

	FOREACH(auto tank, mTankInfo)
		updateClientStatePacket << tank.first << tank.second.position.x << tank.second.position.y << tank.second.tankRotation << tank.second.turretRotation;

	sendToAll(updateClientStatePacket);
}

void GameServer::handleIncomingConnections()
{
	if (!mListeningState)
		return;

	if (mListenerSocket.accept(mPeers[mConnectedPlayers]->socket) == sf::TcpListener::Done)
	{
		
		// order the new client to spawn its own tank	
		mTankInfo[mTankIdentifierCounter].hitpoints = 100;
		mTankInfo[mTankIdentifierCounter].missileAmmo = 20;
		mTankInfo[mTankIdentifierCounter].turretRotation = 0;

		//Check for if on the resistace team
		lastConnected = !lastConnected;

		mTankInfo[mTankIdentifierCounter].position = getSpawnLocation(lastConnected, mTankIdentifierCounter);
		mTankInfo[mTankIdentifierCounter].tankRotation = (lastConnected ? 90 : -90);
		mTankInfo[mTankIdentifierCounter].isLiberator = lastConnected;

		sf::Packet packet;
		packet << static_cast<sf::Int32>(Server::SpawnSelf);
		packet << mTankIdentifierCounter;
		packet << mTankInfo[mTankIdentifierCounter].isLiberator;
		packet << mTankInfo[mTankIdentifierCounter].position.x;
		packet << mTankInfo[mTankIdentifierCounter].position.y;
		packet << mTankInfo[mTankIdentifierCounter].tankRotation;
		packet << mTankInfo[mTankIdentifierCounter].turretRotation;

		mPeers[mConnectedPlayers]->tankIdentifiers.push_back(mTankIdentifierCounter);

		broadcastMessage("Someone has joined the fight!");
		informWorldState(mPeers[mConnectedPlayers]->socket);
		notifyPlayerSpawn(mTankIdentifierCounter++);

		mPeers[mConnectedPlayers]->socket.send(packet);
		mPeers[mConnectedPlayers]->ready = true;
		mPeers[mConnectedPlayers]->lastPacketTime = now(); // prevent initial timeouts
		mTankCount++;
		mConnectedPlayers++;

		if (mConnectedPlayers >= mMaxConnectedPlayers)
			setListening(false);
		else // Add a new waiting peer
			mPeers.push_back(PeerPtr(new RemotePeer()));
	}
}

sf::Vector2f GameServer::getSpawnLocation(bool isLiberator, int tankIdentifier)
{
	sf::Vector2f spawnPosition;

	if (isLiberator) //Liberation
	{
		spawnPosition.x = 700;
	}
	else
	{
		spawnPosition.x = 2300;
	}

	spawnPosition.y = 100.f + (100.f * tankIdentifier);

	return spawnPosition;
}

void GameServer::handleDisconnections()
{
	for (auto itr = mPeers.begin(); itr != mPeers.end(); )
	{
		if ((*itr)->timedOut)
		{
			// Inform everyone of the disconnection, erase 
			FOREACH(sf::Int32 identifier, (*itr)->tankIdentifiers)
			{
				sendToAll(sf::Packet() << static_cast<sf::Int32>(Server::PlayerDisconnect) << identifier);

				mTankInfo.erase(identifier);
			}

			mConnectedPlayers--;
			mTankCount -= (*itr)->tankIdentifiers.size();

			itr = mPeers.erase(itr);

			// Go back to a listening state if needed
			if (mConnectedPlayers < mMaxConnectedPlayers)
			{
				mPeers.push_back(PeerPtr(new RemotePeer()));
				setListening(true);
			}

			broadcastMessage("A player has disconnected.");
		}
		else
		{
			++itr;
		}
	}
}

// Tell the newly connected peer about how the world is currently
void GameServer::informWorldState(sf::TcpSocket& socket)
{
	sf::Packet packet;
	packet << static_cast<sf::Int32>(Server::InitialState);
	packet << static_cast<sf::Int32>(mTankCount);

	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			FOREACH(sf::Int32 identifier, mPeers[i]->tankIdentifiers)
				packet << identifier << mTankInfo[identifier].isLiberator << mTankInfo[identifier].position.x << mTankInfo[identifier].position.y << mTankInfo[identifier].tankRotation << mTankInfo[identifier].turretRotation << mTankInfo[identifier].hitpoints << mTankInfo[identifier].missileAmmo;
		}
	}

	socket.send(packet);
}

void GameServer::broadcastMessage(const std::string& message)
{
	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::BroadcastMessage);
			packet << message;

			mPeers[i]->socket.send(packet);
		}
	}
}

void GameServer::sendToAll(sf::Packet& packet)
{
	FOREACH(PeerPtr& peer, mPeers)
	{
		if (peer->ready)
			peer->socket.send(packet);
	}
}
