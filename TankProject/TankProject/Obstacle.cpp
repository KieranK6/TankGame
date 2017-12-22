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

Obstacle::Obstacle(ObType type, const TextureHolder& textures) : Entity(Table[type].hitpoints)
, mType(type)
, mSprite(textures.get(Table[type].texture), Table[type].textureRect)
, mIdentifier(0)
{
	
}


Obstacle::~Obstacle()
{
}

sf::FloatRect Obstacle::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

