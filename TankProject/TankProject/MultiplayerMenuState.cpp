#include "MultiplayerMenuState.h"
#include "Utility.hpp"
#include "Button.hpp"
#include "ResourceHolder.hpp"
#include "MusicPlayer.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>


MultiplayerMenuState::MultiplayerMenuState(StateStack& stack, Context context)
	: State(stack, context)
	, mGUIContainer()
{
	sf::Vector2f windowSize(context.window->getSize());
	sf::Texture& texture = context.textures->get(Textures::TitleScreen);
	mBackgroundSprite.setTexture(texture);

	menuBacking.setFillColor(sf::Color(0, 0, 0, 150));
	menuBacking.setPosition(sf::Vector2f(windowSize.x / 4, 220));
	menuBacking.setSize(sf::Vector2f(windowSize.x / 2, windowSize.y / 2
	));

	TitleText.setFont(context.fonts->get(Fonts::Main));
	TitleText.setString("Freedom By Force");
	TitleText.setCharacterSize(75);
	TitleText.setFillColor(sf::Color::Black);
	centerOrigin(TitleText);
	TitleText.setPosition(sf::Vector2f(windowSize.x / 2, 150));

	auto hostButton = std::make_shared<GUI::Button>(context);
	hostButton->setPosition(0.5f * windowSize.x, 300); \
	hostButton->setText("Host");
	hostButton->setCallback([this]()
	{
		requestStackPop();
		requestStackPush(States::HostGame);
	});

	auto playButton = std::make_shared<GUI::Button>(context);
	playButton->setPosition(0.5f * windowSize.x, 400);
	playButton->setText("Join");
	playButton->setCallback([this]()
	{
		requestStackPop();
		requestStackPush(States::JoinGame);
	});

	auto backButton = std::make_shared<GUI::Button>(context);
	backButton->setPosition(0.5f * windowSize.x, 500);
	backButton->setText("Back");
	backButton->setCallback([this]()
	{
		
		requestStackPop();
	});

	mGUIContainer.pack(hostButton);
	mGUIContainer.pack(playButton);
	mGUIContainer.pack(backButton);
}

void MultiplayerMenuState::draw()
{
	sf::RenderWindow& window = *getContext().window;

	window.setView(window.getDefaultView());

	window.draw(mBackgroundSprite);
	window.draw(menuBacking);
	window.draw(TitleText);
	window.draw(mGUIContainer);
}

bool MultiplayerMenuState::update(sf::Time)
{
	return true;
}

bool MultiplayerMenuState::handleEvent(const sf::Event& event)
{
	mGUIContainer.handleEvent(event);
	return false;
}

