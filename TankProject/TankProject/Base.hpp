#pragma once
#include "Entity.hpp"
#include "Command.hpp"
#include "ResourceIdentifiers.hpp"

#include "DataTables.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

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
	Base(bType type, const TextureHolder& textures);
	~Base();
	unsigned int Base::getCategory() const;
	sf::FloatRect	getBoundingRect() const;

private:
	virtual void			drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;

private:
	bType					mType;
	sf::Sprite				mSprite;


};

