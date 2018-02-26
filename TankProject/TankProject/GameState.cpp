#include "GameState.hpp"
#include "MusicPlayer.hpp"
#include <iostream>
#include <fstream>


#include <SFML/Graphics/RenderWindow.hpp>

GameState::GameState(StateStack& stack, Context context)
	: State(stack, context)
	, mWorld(*context.window, *context.fonts, *context.sounds, false)
	, mPlayer(nullptr, 1, context.keys1)
	, LiberatorTank(mWorld.addTank(1, Tank::Hotchkiss))
{
	mPlayer.setMissionStatus(Player::MissionRunning);

	

	// Play game theme
	context.music->play(Music::MissionTheme);
}

void GameState::draw()
{
	mWorld.draw();
}

bool GameState::update(sf::Time dt)
{
	mWorld.update(dt);
	mWorld.centerWorldToPlayer(LiberatorTank);

	if (!mWorld.hasAlivePlayer())
	{
		mPlayer.setMissionStatus(Player::MissionFailure);
		requestStackPush(States::GameOver);
	}
	else if (mWorld.hasLiberationBaseBeenDestroyed())
	{
		mPlayer.setMissionStatus(Player::ResistanceSuccess);
		//getContext().sounds->play(SoundEffect::);     //Victory message for Resistance
		requestStackPush(States::ResistanceSuccess);
	}
	else if (mWorld.hasResistanceBaseBeenDestroyed())
	{
		mPlayer.setMissionStatus(Player::LiberatorSuccess);
		//getContext().sounds->play(SoundEffect::);    //Victory Messagew for Liberators
		requestStackPush(States::LiberationSuccess);

	}
	else if (mWorld.hasBaseBeenDestroyed())
	{
		mPlayer.setMissionStatus(Player::MissionSuccess);
		getContext().sounds->play(SoundEffect::Freedum);
		requestStackPush(States::MissionSuccess);
	}


	CommandQueue& commands = mWorld.getCommandQueue();
	mPlayer.handleRealtimeInput(commands);

	return true;
}

bool GameState::handleEvent(const sf::Event& event)
{
	// Game input handling
	CommandQueue& commands = mWorld.getCommandQueue();
	mPlayer.handleEvent(event, commands);

	// Escape pressed, trigger the pause screen
	if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
		requestStackPush(States::Pause);

	return true;
}
