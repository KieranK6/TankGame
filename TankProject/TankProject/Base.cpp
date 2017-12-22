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


Base::Base(bType type, const TextureHolder& textures, const FontHolder& fonts) : Entity(Table[type].hitpoints)
, mType(type)
, mSprite(textures.get(Table[type].texture), Table[type].textureRect)
, mShowExplosion(true)
, mExplosionBegan(false)
{
	mExplosion.setFrameSize(sf::Vector2i(256, 256));
//	mExplosion.setNumFrames(16);   //This line causing error
	mExplosion.setDuration(sf::seconds(1));

	//centerOrigin(mExplosion);

	std::unique_ptr<TextNode> healthDisplay(new TextNode(fonts, ""));
	mHealthDisplay = healthDisplay.get();
	mHealthDisplay->setOrigin(-240, -430);
	attachChild(std::move(healthDisplay));

	updateTexts();

}



sf::FloatRect Base::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

void Base::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (isDestroyed() && mShowExplosion)
		target.draw(mExplosion, states);
	else
	{
		target.draw(mSprite, states);

	}
}

void Base::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	// Update texts and roll animation
	updateTexts();

	// Entity has been destroyed: Possibly drop pickup, mark for removal
	if (isDestroyed())
	{
		mExplosion.update(dt);

		// Play explosion sound only once
		if (!mExplosionBegan)
		{
			SoundEffect::ID soundEffect = (randomInt(2) == 0) ? SoundEffect::Explosion1 : SoundEffect::Explosion2;
			playLocalSound(commands, soundEffect);

			mExplosionBegan = true;
		}
		return;
	}


	// Update enemy movement pattern; apply velocity
	Entity::updateCurrent(dt, commands);
}

void Base::playLocalSound(CommandQueue& commands, SoundEffect::ID effect)
{
	sf::Vector2f worldPosition = getWorldPosition();

	Command command;
	command.category = Category::SoundEffect;
	command.action = derivedAction<SoundNode>(
		[effect, worldPosition](SoundNode& node, sf::Time)
	{
		node.playSound(effect, worldPosition);
	});

	commands.push(command);
}

unsigned int Base::getCategory() const
{
	return Category::Base;
}

bool Base::isMarkedForRemoval() const
{
	return isDestroyed() && (mExplosion.isFinished() || !mShowExplosion);
}

void Base::remove()
{
	Entity::remove();
	mShowExplosion = false;
}

void Base::updateTexts()
{
	// Display hitpoints
	if (isDestroyed())
	{
		mHealthDisplay->setString("");
	}
	else
	{
		mHealthDisplay->setString(toString(getHitpoints()) + " HP");
	}


	mHealthDisplay->setRotation(-getRotation());

}
