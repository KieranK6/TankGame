#include "MultiplayerGameState.hpp"
#include "MusicPlayer.hpp"
#include "Foreach.hpp"
#include "Utility.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Network/IpAddress.hpp>

#include <fstream>


sf::IpAddress getAddressFromFile()
{
	{ // Try to open existing file (RAII block)
		std::ifstream inputFile("ip.txt");
		std::string ipAddress;
		if (inputFile >> ipAddress)
			return ipAddress;
	}

	// If open/read failed, create new file
	std::ofstream outputFile("ip.txt");
	std::string localAddress = "127.0.0.1";
	outputFile << localAddress;
	return localAddress;
}

MultiplayerGameState::MultiplayerGameState(StateStack& stack, Context context, bool isHost)
	: State(stack, context)
	, mWorld(*context.window, *context.fonts, *context.sounds, true)
	, mWindow(*context.window)
	, mTextureHolder(*context.textures)
	, mConnected(false)
	, mGameServer(nullptr)
	, mActiveState(true)
	, mHasFocus(true)
	, mHost(isHost)
	, mGameStarted(false)
	, mClientTimeout(sf::seconds(2.f))
	, mTimeSinceLastPacket(sf::seconds(0.f))
	, playerTank(mWorld.addTank(1, Tank::Hotchkiss))
{
	mBroadcastText.setFont(context.fonts->get(Fonts::Main));
	mBroadcastText.setPosition(1024.f / 2, 100.f);

	mPlayerInvitationText.setFont(context.fonts->get(Fonts::Main));
	mPlayerInvitationText.setCharacterSize(20);
	mPlayerInvitationText.setFillColor(sf::Color::White);
	mPlayerInvitationText.setString("Press Enter to spawn player 2");
	mPlayerInvitationText.setPosition(1000 - mPlayerInvitationText.getLocalBounds().width, 760 - mPlayerInvitationText.getLocalBounds().height);

	// We reuse this text for "Attempt to connect" and "Failed to connect" messages
	mFailedConnectionText.setFont(context.fonts->get(Fonts::Main));
	mFailedConnectionText.setString("Attempting to connect...");
	mFailedConnectionText.setCharacterSize(35);
	mFailedConnectionText.setFillColor(sf::Color::White);
	centerOrigin(mFailedConnectionText);
	mFailedConnectionText.setPosition(mWindow.getSize().x / 2.f, mWindow.getSize().y / 2.f);

	// Render a "establishing connection" frame for user feedback
	mWindow.clear(sf::Color::Black);
	mWindow.draw(mFailedConnectionText);
	mWindow.display();
	mFailedConnectionText.setString("Could not connect to the remote server!");
	centerOrigin(mFailedConnectionText);

	sf::IpAddress ip;
	if (isHost)
	{
		mGameServer.reset(new GameServer(sf::Vector2f(mWindow.getSize())));
		ip = "127.0.0.1";
	}
	else
	{
		ip = getAddressFromFile();
	}

	if (mSocket.connect(ip, ServerPort, sf::seconds(5.f)) == sf::TcpSocket::Done)
		mConnected = true;
	else
		mFailedConnectionClock.restart();

	mSocket.setBlocking(false);

	// Play game theme
	context.music->play(Music::MissionTheme);
}

void MultiplayerGameState::draw()
{
	if (mConnected)
	{
		mWorld.draw();

		// Broadcast messages in default view
		mWindow.setView(mWindow.getDefaultView());

		if (!mBroadcasts.empty())
			mWindow.draw(mBroadcastText);

		if (mLocalPlayerIdentifiers.size() < 2 && mPlayerInvitationTime < sf::seconds(0.5f))
			mWindow.draw(mPlayerInvitationText);
	}
	else
	{
		mWindow.draw(mFailedConnectionText);
	}
}

void MultiplayerGameState::onActivate()
{
	mActiveState = true;
}

void MultiplayerGameState::onDestroy()
{
	if (!mHost && mConnected)
	{
		// Inform server this client is dying
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Client::Quit);
		mSocket.send(packet);
	}
}

bool MultiplayerGameState::update(sf::Time dt)
{
	// Connected to server: Handle all the network logic
	if (mConnected)
	{
		mWorld.update(dt);
		mWorld.centerWorldToPlayer(playerTank);
		mWorld.adaptPlayerTankPosition(playerTank);

		//Check for win
		if (!mWorld.hasAlivePlayer())
		{
			//mPlayer.setMissionStatus(Player::MissionFailure);
			requestStackPush(States::GameOver);
		}
		else if (mWorld.hasLiberationBaseBeenDestroyed())
		{
			//mPlayer.setMissionStatus(Player::ResistanceSuccess);
			//getContext().sounds->play(SoundEffect::);     //Victory message for Resistance
			requestStackPush(States::ResistanceSuccess);
		}
		else if (mWorld.hasResistanceBaseBeenDestroyed())
		{
			//mPlayer.setMissionStatus(Player::LiberatorSuccess);
			//getContext().sounds->play(SoundEffect::);    //Victory Messagew for Liberators
			requestStackPush(States::LiberationSuccess);
		}
		else if (mWorld.hasBaseBeenDestroyed())
		{
			//mPlayer.setMissionStatus(Player::MissionSuccess);
			getContext().sounds->play(SoundEffect::Freedum);
			requestStackPush(States::MissionSuccess);
		}

		// Remove players whose aircrafts were destroyed
		bool foundLocalPlane = false;
		for (auto itr = mPlayers.begin(); itr != mPlayers.end(); )
		{
			// Check if there are no more local planes for remote clients
			if (std::find(mLocalPlayerIdentifiers.begin(), mLocalPlayerIdentifiers.end(), itr->first) != mLocalPlayerIdentifiers.end())
			{
				foundLocalPlane = true;
			}

			if (!mWorld.getTank(itr->first))
			{
				itr = mPlayers.erase(itr);

				// No more players left: Mission failed
				if (mPlayers.empty())
					requestStackPush(States::GameOver);
			}
			else
			{
				++itr;
			}
		}

		if (!foundLocalPlane && mGameStarted)
		{
			//requestStackPush(States::GameOver);
		}

		// Only handle the realtime input if the window has focus and the game is unpaused
		if (mActiveState && mHasFocus)
		{
			CommandQueue& commands = mWorld.getCommandQueue();
			FOREACH(auto& pair, mPlayers)
				pair.second->handleRealtimeInput(commands);
		}

		// Always handle the network input
		CommandQueue& commands = mWorld.getCommandQueue();
		FOREACH(auto& pair, mPlayers)
			pair.second->handleRealtimeNetworkInput(commands);

		// Handle messages from server that may have arrived
		sf::Packet packet;
		if (mSocket.receive(packet) == sf::Socket::Done)
		{
			mTimeSinceLastPacket = sf::seconds(0.f);
			sf::Int32 packetType;
			packet >> packetType;
			handlePacket(packetType, packet);
		}
		else
		{
			// Check for timeout with the server
			if (mTimeSinceLastPacket > mClientTimeout)
			{
				mConnected = false;

				mFailedConnectionText.setString("Lost connection to server");
				centerOrigin(mFailedConnectionText);

				mFailedConnectionClock.restart();
			}
		}

		updateBroadcastMessage(dt);

		// Time counter for blinking 2nd player text
		mPlayerInvitationTime += dt;
		if (mPlayerInvitationTime > sf::seconds(1.f))
			mPlayerInvitationTime = sf::Time::Zero;

		// Events occurring in the game
		GameActions::Action gameAction;
		while (mWorld.pollGameAction(gameAction))
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Client::GameEvent);
			packet << static_cast<sf::Int32>(gameAction.type);
			packet << gameAction.position.x;
			packet << gameAction.position.y;

			mSocket.send(packet);
		}

		// Regular position updates
		if (mTickClock.getElapsedTime() > sf::seconds(1.f / 20.f))
		{
			sf::Packet positionUpdatePacket;
			positionUpdatePacket << static_cast<sf::Int32>(Client::PositionUpdate);
			positionUpdatePacket << static_cast<sf::Int32>(mLocalPlayerIdentifiers.size());

			FOREACH(sf::Int32 identifier, mLocalPlayerIdentifiers)
			{
				if (Tank* tank = mWorld.getTank(identifier))
					positionUpdatePacket << identifier << tank->getPosition().x << tank->getPosition().y << tank->getRotation() << tank->getTurretRotation() << static_cast<sf::Int32>(tank->getHitpoints()) << static_cast<sf::Int32>(tank->getMissileAmmo());
			}

			mSocket.send(positionUpdatePacket);
			mTickClock.restart();
		}

		mTimeSinceLastPacket += dt;
	}

	// Failed to connect and waited for more than 5 seconds: Back to menu
	else if (mFailedConnectionClock.getElapsedTime() >= sf::seconds(5.f))
	{
		requestStackClear();
		requestStackPush(States::Menu);
	}

	return true;
}

void MultiplayerGameState::disableAllRealtimeActions()
{
	mActiveState = false;

	FOREACH(sf::Int32 identifier, mLocalPlayerIdentifiers)
		mPlayers[identifier]->disableAllRealtimeActions();
}

bool MultiplayerGameState::handleEvent(const sf::Event& event)
{
	// Game input handling
	CommandQueue& commands = mWorld.getCommandQueue();

	// Forward event to all players
	FOREACH(auto& pair, mPlayers)
		pair.second->handleEvent(event, commands);

	if (event.type == sf::Event::KeyPressed)
	{
		// Enter pressed, add second player co-op (only if we are one player)
		if (event.key.code == sf::Keyboard::Return && mLocalPlayerIdentifiers.size() == 1)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Client::RequestCoopPartner);
			
			bool isLiberator;
			if (playerTank->getType() == Tank::Hotchkiss)
			{
				isLiberator = true;
			}
			else
			{
				isLiberator = false;
			}

			mSocket.send(packet);
		}

		// Escape pressed, trigger the pause screen
		else if (event.key.code == sf::Keyboard::Escape)
		{
			disableAllRealtimeActions();
			requestStackPush(States::NetworkPause);
		}
	}
	else if (event.type == sf::Event::GainedFocus)
	{
		mHasFocus = true;
	}
	else if (event.type == sf::Event::LostFocus)
	{
		mHasFocus = false;
	}

	return true;
}

void MultiplayerGameState::updateBroadcastMessage(sf::Time elapsedTime)
{
	if (mBroadcasts.empty())
		return;

	// Update broadcast timer
	mBroadcastElapsedTime += elapsedTime;
	if (mBroadcastElapsedTime > sf::seconds(2.5f))
	{
		// If message has expired, remove it
		mBroadcasts.erase(mBroadcasts.begin());

		// Continue to display next broadcast message
		if (!mBroadcasts.empty())
		{
			mBroadcastText.setString(mBroadcasts.front());
			centerOrigin(mBroadcastText);
			mBroadcastElapsedTime = sf::Time::Zero;
		}
	}
}

void MultiplayerGameState::handlePacket(sf::Int32 packetType, sf::Packet& packet)
{
	switch (packetType)
	{
		// Send message to all clients
	case Server::BroadcastMessage:
	{
		std::string message;
		packet >> message;
		mBroadcasts.push_back(message);

		// Just added first message, display immediatelyk
		if (mBroadcasts.size() == 1)
		{
			mBroadcastText.setString(mBroadcasts.front());
			centerOrigin(mBroadcastText);
			mBroadcastElapsedTime = sf::Time::Zero;
		}
	} break;

	// Sent by the server to order to spawn player 1 tank on connect
	case Server::SpawnSelf:
	{
		sf::Int32 tankIdentifier;
		bool isLiberator;
		sf::Vector2f tankPosition;// = playerTank->getPosition();
		float tankRotation, turretRotation;

		packet >> tankIdentifier >> isLiberator >> tankPosition.x >> tankPosition.y >> tankRotation >> turretRotation;

		mPlayers[tankIdentifier].reset(new Player(&mSocket, tankIdentifier, getContext().keys1));
		mLocalPlayerIdentifiers.push_back(tankIdentifier);

		mGameStarted = true;
	} break;

	// 
	case Server::PlayerConnect:
	{
		sf::Int32 tankIdentifier;
		bool isLiberator;
		sf::Vector2f tankPosition;
		float tankRotation, turretRotation;
		Tank::Type type;

		packet >> tankIdentifier >> isLiberator >> tankPosition.x >> tankPosition.y >> tankRotation >> turretRotation;

		if (isLiberator)
		{
			type = Tank::Hotchkiss;
		}
		else
		{
			type = Tank::Panzer;
		}

		Tank* tank = mWorld.addTank(tankIdentifier, type);
		tankPosition.x = 2000.f;
		tank->setPosition(tankPosition);
		tank->setRotation(tankRotation);
		tank->setTurretRotation(turretRotation);

		mPlayers[tankIdentifier].reset(new Player(&mSocket, tankIdentifier, nullptr));
	} break;

	// 
	case Server::PlayerDisconnect:
	{
		sf::Int32 tankIdentifier;
		packet >> tankIdentifier;

		mWorld.removeTank(tankIdentifier);
		mPlayers.erase(tankIdentifier);
	} break;

	// 
	case Server::InitialState:
	{
		sf::Int32 tankCount;
		//float worldHeight, currentScroll;
		//packet >> worldHeight >> currentScroll;

		//mWorld.setWorldHeight(worldHeight);
		//mWorld.setCurrentBattleFieldPosition(currentScroll);

		packet >> tankCount;
		for (sf::Int32 i = 0; i < tankCount; ++i)
		{
			sf::Int32 tankIdentifier;
			bool isLiberator;
			sf::Int32 hitpoints;
			sf::Int32 missileAmmo;
			sf::Vector2f tankPosition;
			float tankRotation, turretRotation;

			packet >> tankIdentifier >> isLiberator >> tankPosition.x >> tankPosition.y >> tankRotation >> turretRotation >> hitpoints >> missileAmmo;

			Tank::Type type = Tank::Panzer;
			if (isLiberator)
			{
				type = Tank::Hotchkiss;
				//tankPosition.x = 100.f; //Liberation
			//	tankPosition.y = 100.f;
			}
			

			//tankPosition.x = 2000.f; //Worked on spawning
			//tankPosition.y = 100.f;

			Tank* tank = mWorld.addTank(tankIdentifier, type);
			tank->setPosition(tankPosition);
			tank->setHitpoints(hitpoints);
			tank->setMissileAmmo(missileAmmo);
			tank->setRotation(tankRotation);
			tank->setTurretRotation(turretRotation);
			playerTank = tank;

			mPlayers[tankIdentifier].reset(new Player(&mSocket, tankIdentifier, nullptr));
		}
	} break;

	//
	case Server::AcceptCoopPartner:
	{
		sf::Int32 tankIdentifier;
		bool isLiberator;
		Tank::Type type;

		packet >> tankIdentifier;
		packet >> isLiberator;

		if (isLiberator)
		{
			type = Tank::T34;
		}
		else
		{
			type = Tank::Panther;
		}

		mWorld.addTank(tankIdentifier, type);
		mPlayers[tankIdentifier].reset(new Player(&mSocket, tankIdentifier, getContext().keys2));
		mLocalPlayerIdentifiers.push_back(tankIdentifier);
	} break;

	// Player event (like missile fired) occurs
	case Server::PlayerEvent:
	{
		sf::Int32 tankIdentifier;
		sf::Int32 action;
		packet >> tankIdentifier >> action;

		auto itr = mPlayers.find(tankIdentifier);
		if (itr != mPlayers.end())
			itr->second->handleNetworkEvent(static_cast<Player::Action>(action), mWorld.getCommandQueue());
	} break;

	// Player's movement or fire keyboard state changes
	case Server::PlayerRealtimeChange:
	{
		sf::Int32 tankIdentifier;
		sf::Int32 action;
		bool actionEnabled;
		packet >> tankIdentifier >> action >> actionEnabled;

		auto itr = mPlayers.find(tankIdentifier);
		if (itr != mPlayers.end())
			itr->second->handleNetworkRealtimeChange(static_cast<Player::Action>(action), actionEnabled);
	} break;

	// New enemy to be created
	case Server::SpawnEnemy:
	{
		float height;
		sf::Int32 type;
		float relativeX;
		packet >> type >> height >> relativeX;

		//mWorld.addEnemy(static_cast<Tank::Type>(type), relativeX, height);
		//mWorld.sortEnemies();
	} break;

	// Mission successfully completed
	case Server::MissionSuccess:
	{
		requestStackPush(States::MissionSuccess);
	} break;

	// Pickup created
	case Server::SpawnPickup:
	{
		sf::Int32 type;
		sf::Vector2f position;
		packet >> type >> position.x >> position.y;

		mWorld.createPickup(position, static_cast<Pickup::Type>(type));
	} break;

	//
	case Server::UpdateClientState:
	{
		float currentWorldPosition;
		sf::Int32 aircraftCount;
		packet >> currentWorldPosition >> aircraftCount;

		float currentViewPosition = mWorld.getViewBounds().top + mWorld.getViewBounds().height;

		// Set the world's scroll compensation according to whether the view is behind or too advanced
		//mWorld.setWorldScrollCompensation(currentViewPosition / currentWorldPosition);

		for (sf::Int32 i = 0; i < aircraftCount; ++i)
		{
			sf::Vector2f tankPosition;
			sf::Int32 tankIdentifier;
			float tankRotation, turretRotation;
			packet >> tankIdentifier >> tankPosition.x >> tankPosition.y >> tankRotation >> turretRotation;

			Tank* tank = mWorld.getTank(tankIdentifier);
			bool isLocalPlane = std::find(mLocalPlayerIdentifiers.begin(), mLocalPlayerIdentifiers.end(), tankIdentifier) != mLocalPlayerIdentifiers.end();
			if (tank && !isLocalPlane)
			{
				sf::Vector2f interpolatedPosition = tank->getPosition() + (tankPosition - tank->getPosition()) * 0.1f;
				tank->setPosition(interpolatedPosition);
				float interpolatedRotation = tank->getRotation() + (tankRotation - tank->getRotation()) * 0.1f;
				tank->setRotation(interpolatedRotation);
				float interpolatedTurretRotation = tank->getTurretRotation() + (turretRotation - tank->getTurretRotation()) * 0.1f;
				tank->setTurretRotation(interpolatedTurretRotation);
			}
		}
	} break;
	}
}
