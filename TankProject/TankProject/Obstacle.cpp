#pragma once
#include "Obstacle.hpp"

#include "DataTables.hpp"
#include "Utility.hpp"
#include "Pickup.hpp"
#include "CommandQueue.hpp"
#include "SoundNode.hpp"
#include "NetworkNode.hpp"
#include "ResourceHolder.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <cmath>
#include <iostream>

namespace
{
	const std::vector<ObstacleData> Table = initializeObstacleData();
}

Obstacle::Obstacle(ObType type, const TextureHolder& textures) : Entity(Table[type].hitpoints) //Constructor, inherits from entity, has hitpoints type and texture defined in datatables
, mType(type)
, mSprite(textures.get(Table[type].texture), Table[type].textureRect)

{
	centerOrigin(mSprite);
}


Obstacle::~Obstacle()
{
}

sf::FloatRect Obstacle::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds()); //returns boudning rectangle used for collision detection
}

void Obstacle::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	
		target.draw(mSprite, states); //draws sprite onto target
		drawBoundingCirc(target, states, mObRadius);
}

unsigned int Obstacle::getCategory() const
{
		return Category::Obstacle;
}

float Obstacle::getObstacleRadius() const
{
	return mObRadius;
}

