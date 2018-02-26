#include "Player.hpp"
#include "CommandQueue.hpp"
#include "Tank.hpp"
#include "Foreach.hpp"
#include "NetworkProtocol.hpp"

#include <SFML/Network/Packet.hpp>

#include <map>
#include <string>
#include <algorithm>

using namespace std::placeholders;

enum Direction
{
	right,
	left
};

struct TankMover
{
	TankMover(float vx, float vy, int identifier)
		: velocity(vx, vy), tankID(identifier)
	{
	}

	void operator() (Tank& tank, sf::Time) const
	{
		if (tank.getIdentifier() == tankID)
		{
			tank.accelerate(velocity * tank.getSpeedBoost() * tank.getMaxSpeed());

		}
	}

	sf::Vector2f velocity;
	int tankID;
};

struct TankRotater
{
	TankRotater(Direction rotationDirection, int identifier)
		: tankRotationDirection(rotationDirection), tankID(identifier)
	{
	}

	void operator() (Tank& tank, sf::Time) const
	{
		if (tank.getIdentifier() == tankID)
		{
			if (tankRotationDirection == Direction::left)
				tank.rotate(-1 * tank.getMaxSpeed()/100);
			else
				tank.rotate(1 * tank.getMaxSpeed()/100);

		}

	}

	Direction tankRotationDirection;
	int tankID;
};

struct TurretRotater
{
	TurretRotater(Direction rotationDirection, int identifier)
		: turretRotationDirection(rotationDirection), tankID(identifier)
	{
	}

	void operator() (Tank& tank, sf::Time) const
	{
		if (tank.getIdentifier() == tankID)
		{
			if (turretRotationDirection == Direction::left)
				tank.accelerateTurretRotation(-1 * tank.getTurretRotationSpeed());
			else
				tank.accelerateTurretRotation(1 * tank.getTurretRotationSpeed());
		}

	}

	Direction turretRotationDirection;
	int tankID;
};

struct TankFireTrigger
{
	TankFireTrigger(int identifier)
		: tankID(identifier)
	{
	}

	void operator() (Tank& tank, sf::Time) const
	{
		if (tank.getIdentifier() == tankID)
			tank.fire();
	}

	int tankID;
};

Player::Player(sf::TcpSocket* socket, sf::Int32 identifier, const KeyBinding* binding)
	: mKeyBinding(binding)
	, mCurrentMissionStatus(MissionRunning)
	, mIdentifier(identifier)
	, mSocket(socket)
{
	// Set initial action bindings
	initializeActions();

	// Assign all categories to player's tank
	FOREACH(auto& pair, mActionBinding)
		pair.second.category = Category::LiberatorTank | Category::ResistanceTank;
}

void Player::handleEvent(const sf::Event& event, CommandQueue& commands)
{
	if (event.type == sf::Event::KeyPressed)
	{
		Action action;
		if (mKeyBinding && mKeyBinding->checkAction(event.key.code, action) && !isRealtimeAction(action))
		{
			// Network connected -> send event over network
			if (mSocket)
			{
				sf::Packet packet;
				packet << static_cast<sf::Int32>(Client::PlayerEvent);
				packet << mIdentifier;
				packet << static_cast<sf::Int32>(action);
				mSocket->send(packet);
			}

			// Network disconnected -> local event
			else
			{
				commands.push(mActionBinding[action]);
			}
		}
	}
	// Realtime change (network connected)
	if ((event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased) && mSocket)
	{
		Action action;
		if (mKeyBinding && mKeyBinding->checkAction(event.key.code, action) && isRealtimeAction(action))
		{
			// Send realtime change over network
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Client::PlayerRealtimeChange);
			packet << mIdentifier;
			packet << static_cast<sf::Int32>(action);
			packet << (event.type == sf::Event::KeyPressed);
			mSocket->send(packet);
		}
	}
}

bool Player::isLocal() const
{
	// No key binding means this player is remote
	return mKeyBinding != nullptr;
}

void Player::disableAllRealtimeActions()
{
	FOREACH(auto& action, mActionProxies)
	{
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Client::PlayerRealtimeChange);
		packet << mIdentifier;
		packet << static_cast<sf::Int32>(action.first);
		packet << false;
		mSocket->send(packet);
	}
}

void Player::handleRealtimeInput(CommandQueue& commands)
{
	// Check if this is a networked game and local player or just a single player game
	if ((mSocket && isLocal()) || !mSocket)
	{
		// Lookup all actions and push corresponding commands to queue
		std::vector<Action> activeActions = mKeyBinding->getRealtimeActions();
		FOREACH(Action action, activeActions)
			commands.push(mActionBinding[action]);
	}
}

void Player::handleRealtimeNetworkInput(CommandQueue& commands)
{
	if (mSocket && !isLocal())
	{
		// Traverse all realtime input proxies. Because this is a networked game, the input isn't handled directly
		FOREACH(auto pair, mActionProxies)
		{
			if (pair.second && isRealtimeAction(pair.first))
				commands.push(mActionBinding[pair.first]);
		}
	}
}

void Player::handleNetworkEvent(Action action, CommandQueue& commands)
{
	commands.push(mActionBinding[action]);
}

void Player::handleNetworkRealtimeChange(Action action, bool actionEnabled)
{
	mActionProxies[action] = actionEnabled;
}

void Player::setMissionStatus(MissionStatus status)
{
	mCurrentMissionStatus = status;
}

Player::MissionStatus Player::getMissionStatus() const
{
	return mCurrentMissionStatus;
}

void Player::initializeActions()
{
	mActionBinding[PlayerAction::RotateLeft].action = derivedAction<Tank>(TankRotater(Direction::left, mIdentifier));
	mActionBinding[PlayerAction::RotateRight].action = derivedAction<Tank>(TankRotater(Direction::right, mIdentifier));
	mActionBinding[PlayerAction::MoveUp].action = derivedAction<Tank>(TankMover(-0.6f, -0.6f, mIdentifier));
	mActionBinding[PlayerAction::MoveDown].action = derivedAction<Tank>(TankMover(+0.25f, +0.25f, mIdentifier));
	mActionBinding[PlayerAction::RotateTurretLeft].action = derivedAction<Tank>(TurretRotater(Direction::left, mIdentifier));
	mActionBinding[PlayerAction::RotateTurretRight].action = derivedAction<Tank>(TurretRotater(Direction::right, mIdentifier));
	mActionBinding[PlayerAction::Fire].action = derivedAction<Tank>(TankFireTrigger(mIdentifier));
}


