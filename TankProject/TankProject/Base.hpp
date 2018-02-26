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

//This class was worked on by Kieran Keegan

class Base : public Entity
{
public:
	enum baseTeam
	{
		EnemyBase,
		LiberatorsBase,
		ResistanceBase,
		TypeCount,
	};

public:
	Base(baseTeam type, const TextureHolder& textures, const FontHolder& fonts);

	unsigned int			getCategory() const;
	sf::FloatRect			getBoundingRect() const;
	void					playLocalSound(CommandQueue& commands, SoundEffect::ID effect);
	virtual void			remove();
	virtual bool 			isMarkedForRemoval() const;
	float					GetBaseRadius();
	baseTeam				mType;
	sf::Sprite				mSprite;

private:
	virtual void			drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
	void					updateCurrent(sf::Time dt, CommandQueue& commands);
	void					updateTexts();

private:
	
	
	TextNode*				mHealthDisplay;
	//sf::Vector2f			position;

	Animation				mBaseExplosion;
	bool 					mShowExplosion;
	bool					mExplosionBegan;
	bool					mPlayedExplosionSound;
	float					mBaseRadius = 200.f;


};

