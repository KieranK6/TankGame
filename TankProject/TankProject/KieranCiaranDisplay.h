#pragma once
#include "State.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>


class KieranCiaranDisplay : public State
{
public:
	KieranCiaranDisplay(StateStack& stack, Context context);

	virtual void draw();
	virtual bool update(sf::Time dt);
	virtual bool handleEvent(const sf::Event& event);

private:
	sf::Sprite mBackgroundSprite;
	sf::Text TitleText;
	sf::Text ByText;
	sf::Text mText;

	bool mFadeText;
	sf::Time mTextEffectTime;
	sf::Color mTextColor;
};