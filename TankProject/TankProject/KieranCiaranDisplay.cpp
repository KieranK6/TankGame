#include "KieranCiaranDisplay.h"
#include "Utility.hpp"
#include "ResourceHolder.hpp"

#include <SFML/Graphics/RenderWindow.hpp>


KieranCiaranDisplay::KieranCiaranDisplay(StateStack& stack, Context context)
	: State(stack, context)
	, mText()
	, mFadeText(true)
	, mTextEffectTime(sf::Time::Zero)
{
	sf::Vector2f windowSize(context.window->getSize());
	mBackgroundSprite.setTexture(context.textures->get(Textures::TitleScreen));

	TitleText.setFont(context.fonts->get(Fonts::Main));
	TitleText.setString("Freedom By Force");
	TitleText.setCharacterSize(75);
	TitleText.setFillColor(sf::Color::Black);
	centerOrigin(TitleText);
	TitleText.setPosition(sf::Vector2f(windowSize.x / 2, 150));

	ByText.setFont(context.fonts->get(Fonts::Clear));
	ByText.setString("By");
	ByText.setCharacterSize(50);
	ByText.setFillColor(sf::Color(0, 0, 0, 255));
	centerOrigin(ByText);
	ByText.setPosition(sf::Vector2f(windowSize.x / 2, (windowSize.y / 2) - 50));

	mText.setFont(context.fonts->get(Fonts::Clear));
	mText.setString("Kieran Keegan & Ciaran Mooney");
	mText.setCharacterSize(50);
	mText.setFillColor(sf::Color(0, 0, 0, 255));
	centerOrigin(mText);
	mText.setPosition(sf::Vector2f(windowSize.x / 2, (windowSize.y / 2) + 50));
}

void KieranCiaranDisplay::draw()
{
	sf::RenderWindow& window = *getContext().window;
	window.draw(mBackgroundSprite);
	window.draw(TitleText);
	window.draw(ByText);
	window.draw(mText);
}

bool KieranCiaranDisplay::update(sf::Time dt)
{

	mTextEffectTime += dt;
	mTextColor = mText.getFillColor();

	if (mTextColor.a < 2)
	{
		requestStackPop();
		requestStackPush(States::Title);
	}
	else if (!mFadeText && mTextColor.a > 2)
	{
		mTextColor.a -= 2.0f;
	}

	ByText.setFillColor(mTextColor);
	mText.setFillColor(mTextColor);

	if (mTextEffectTime >= sf::seconds(3.0f))
	{
		mFadeText = false;
		mTextEffectTime = sf::Time::Zero;
	}
	return true;

}

bool KieranCiaranDisplay::handleEvent(const sf::Event& event)
{
	//If any key is pressed, trigger the next screen
	if (event.type == sf::Event::KeyReleased)
	{
		requestStackPop();
		requestStackPush(States::Title);
	}

	return true;
}