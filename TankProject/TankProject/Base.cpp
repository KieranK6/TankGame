#include "Base.hpp"
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
	const std::vector<BaseData> Table = initializeBaseData();
}


Base::Base(bType type, const TextureHolder& textures) : Entity(Table[type].hitpoints)
, mType(type)
, mSprite(textures.get(Table[type].texture), Table[type].textureRect)
{
}


Base::~Base()
{
}

sf::FloatRect Base::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

void Base::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{

	target.draw(mSprite, states);

}

unsigned int Base::getCategory() const
{
	return Category::Base;
}
