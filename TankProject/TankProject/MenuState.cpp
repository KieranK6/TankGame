#include "MenuState.hpp"
#include "Utility.hpp"
#include "Button.hpp"
#include "ResourceHolder.hpp"
#include "MusicPlayer.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>


MenuState::MenuState(StateStack& stack, Context context)
	: State(stack, context)
	, mGUIContainer()
{
	sf::Vector2f windowSize(context.window->getSize());
	sf::Texture& texture = context.textures->get(Textures::TitleScreen);
	mBackgroundSprite.setTexture(texture);

	menuBacking.setFillColor(sf::Color(0, 0, 0, 150));
	menuBacking.setPosition(sf::Vector2f(windowSize.x / 4, 220));
	menuBacking.setSize(sf::Vector2f(windowSize.x / 2, 460));

	TitleText.setFont(context.fonts->get(Fonts::Main));
	TitleText.setString("Freedom By Force");
	TitleText.setCharacterSize(75);
	TitleText.setFillColor(sf::Color::Black);
	centerOrigin(TitleText);
	TitleText.setPosition(sf::Vector2f(windowSize.x / 2, 150));

	auto playButton = std::make_shared<GUI::Button>(context);
	playButton->setPosition(0.5f * windowSize.x, 300);\
	playButton->setText("Training");
	playButton->setCallback([this]()
	{
		requestStackPop();
		requestStackPush(States::Game);
	});

	auto multiplayerButton = std::make_shared<GUI::Button>(context);
	multiplayerButton->setPosition(0.5f * windowSize.x, 400);
	multiplayerButton->setText("Multiplayer");
	multiplayerButton->setCallback([this]()
	{
		requestStackPush(States::MultiplayerMenu);
	});

	auto settingsButton = std::make_shared<GUI::Button>(context);
	settingsButton->setPosition(0.5f * windowSize.x, 500);
	settingsButton->setText("Settings");
	settingsButton->setCallback([this]()
	{
		requestStackPush(States::Settings);
	});

	auto exitButton = std::make_shared<GUI::Button>(context);
	exitButton->setPosition(0.5f * windowSize.x, 600);
	exitButton->setText("Exit");
	exitButton->setCallback([this]()
	{
		requestStackPop();
	});

	mGUIContainer.pack(playButton);
	mGUIContainer.pack(multiplayerButton);
	mGUIContainer.pack(settingsButton);
	mGUIContainer.pack(exitButton);

	//Play menu theme
	context.music->play(Music::MenuTheme);
}

void MenuState::draw()
{
	sf::RenderWindow& window = *getContext().window;

	window.setView(window.getDefaultView());

	window.draw(mBackgroundSprite);
	window.draw(menuBacking);
	window.draw(TitleText);
	window.draw(mGUIContainer);
}

bool MenuState::update(sf::Time)
{
	return true;
}

bool MenuState::handleEvent(const sf::Event& event)
{
	mGUIContainer.handleEvent(event);
	return false;
}

