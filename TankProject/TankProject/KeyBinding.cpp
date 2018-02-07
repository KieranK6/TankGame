#include "KeyBinding.hpp"
#include "Foreach.hpp"

#include <string>
#include <algorithm>


KeyBinding::KeyBinding(int controlPreconfiguration)
	: mKeyMap()
{
	// Set initial key bindings for player 1
	if (controlPreconfiguration == 1)
	{
		mKeyMap[sf::Keyboard::Left] = PlayerAction::RotateLeft;
		mKeyMap[sf::Keyboard::Right] = PlayerAction::RotateRight;
		mKeyMap[sf::Keyboard::Up] = PlayerAction::MoveUp;
		mKeyMap[sf::Keyboard::Down] = PlayerAction::MoveDown;
		mKeyMap[sf::Keyboard::Q] = PlayerAction::RotateTurretLeft;
		mKeyMap[sf::Keyboard::E] = PlayerAction::RotateTurretRight;
		mKeyMap[sf::Keyboard::Space] = PlayerAction::Fire;
	}
	else if (controlPreconfiguration == 2)
	{
		// Player 2
		mKeyMap[sf::Keyboard::Numpad4] = PlayerAction::RotateLeft;
		mKeyMap[sf::Keyboard::Numpad6] = PlayerAction::RotateRight;
		mKeyMap[sf::Keyboard::Numpad8] = PlayerAction::MoveUp;
		mKeyMap[sf::Keyboard::Numpad5] = PlayerAction::MoveDown;
		mKeyMap[sf::Keyboard::LControl] = PlayerAction::RotateTurretLeft;
		mKeyMap[sf::Keyboard::LAlt] = PlayerAction::RotateTurretRight;
		mKeyMap[sf::Keyboard::Numpad0] = PlayerAction::Fire;
	}
}

void KeyBinding::assignKey(Action action, sf::Keyboard::Key key)
{
	// Remove all keys that already map to action
	for (auto itr = mKeyMap.begin(); itr != mKeyMap.end(); )
	{
		if (itr->second == action)
			mKeyMap.erase(itr++);
		else
			++itr;
	}

	// Insert new binding
	mKeyMap[key] = action;
}

sf::Keyboard::Key KeyBinding::getAssignedKey(Action action) const
{
	FOREACH(auto pair, mKeyMap)
	{
		if (pair.second == action)
			return pair.first;
	}

	return sf::Keyboard::Unknown;
}

bool KeyBinding::checkAction(sf::Keyboard::Key key, Action& out) const
{
	auto found = mKeyMap.find(key);
	if (found == mKeyMap.end())
	{
		return false;
	}
	else
	{
		out = found->second;
		return true;
	}
}

std::vector<KeyBinding::Action> KeyBinding::getRealtimeActions() const
{
	// Return all realtime actions that are currently active.
	std::vector<Action> actions;

	FOREACH(auto pair, mKeyMap)
	{
		// If key is pressed and an action is a realtime action, store it
		if (sf::Keyboard::isKeyPressed(pair.first) && isRealtimeAction(pair.second))
			actions.push_back(pair.second);
	}

	return actions;
}

bool isRealtimeAction(PlayerAction::Type action)
{
	switch (action)
	{
	case PlayerAction::RotateLeft:
	case PlayerAction::RotateRight:
	case PlayerAction::MoveDown:
	case PlayerAction::MoveUp:
	case PlayerAction::RotateTurretLeft:
	case PlayerAction::RotateTurretRight:
	case PlayerAction::Fire:
		return true;

	default:
		return false;
	}
}