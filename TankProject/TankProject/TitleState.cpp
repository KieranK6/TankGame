#include "TitleState.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"

#include <SFML/Graphics/RenderWindow.hpp>


TitleState::TitleState(StateStack& stack, Context context)
	: State(stack, context)
	,mText()
	,mFadeText(true)
	,mTextEffectTime(sf::Time::Zero)
{
	sf::Vector2f windowSize(context.window->getSize());
	mBackgroundSprite.setTexture(context.textures->get(Textures::TitleScreen));

	TitleText.setFont(context.fonts->get(Fonts::Main));
	TitleText.setString("Freedom By Force");
	TitleText.setCharacterSize(75);
	TitleText.setFillColor(sf::Color::Black);
	centerOrigin(TitleText);
	TitleText.setPosition(sf::Vector2f(windowSize.x / 2, 150));

	mText.setFont(context.fonts->get(Fonts::Main));
	mText.setString("Press any key to start");
	mText.setCharacterSize(50);
	mText.setFillColor(sf::Color(0, 0, 0, 0));
	centerOrigin(mText);
	mText.setPosition(sf::Vector2f(context.window->getSize() / 2u));
}

void TitleState::draw()
{
	sf::RenderWindow& window = *getContext().window;
	window.draw(mBackgroundSprite);
	window.draw(TitleText);
	window.draw(mText);
}

bool TitleState::update(sf::Time dt)
{
	
	mTextEffectTime += dt;
	mTextColor = mText.getFillColor();

	if (mFadeText && mTextColor.a < 254)
	{
		mTextColor.a += 2.0f;
	}
	else if(!mFadeText && mTextColor.a > 2)
	{
		mTextColor.a -= 2.0f;
	}
	
	mText.setFillColor(mTextColor);

	if (mTextEffectTime >= sf::seconds(4.0f))
	{
		mFadeText = !mFadeText;
		mTextEffectTime = sf::Time::Zero;
	}
	return true;
	
}

bool TitleState::handleEvent(const sf::Event& event)
{
	//If any key is pressed, trigger the next screen
	if (event.type == sf::Event::KeyReleased)
	{
		requestStackPop();
		requestStackPush(States::Menu);
	}
	return true;
}