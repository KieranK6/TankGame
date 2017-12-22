#pragma once
#include "Entity.hpp"
#include "Command.hpp"
#include "ResourceIdentifiers.hpp"

#include "DataTables.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include "TextNode.hpp"
#include "Animation.hpp"

#include <SFML/Graphics/Sprite.hpp>
class Base : public Entity
{
public:
	enum bType
	{
		EnemyBase,
		TypeCount,
	};

public:
	Base(bType type, const TextureHolder& textures, const FontHolder& fonts);

	unsigned int Base::getCategory() const;
	sf::FloatRect	getBoundingRect() const;
	void					playLocalSound(CommandQueue& commands, SoundEffect::ID effect);
	virtual void			remove();
	virtual bool 			isMarkedForRemoval() const;

private:
	virtual void			drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
	void					updateCurrent(sf::Time dt, CommandQueue& commands);
	void					updateTexts();

private:
	bType					mType;
	sf::Sprite				mSprite;
	TextNode*				mHealthDisplay;

	Animation				mExplosion;
	bool 					mShowExplosion;
	bool					mExplosionBegan;
	bool					mPlayedExplosionSound;


};

