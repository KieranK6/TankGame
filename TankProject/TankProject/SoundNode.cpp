#include "SoundNode.hpp"
#include "SoundPlayer.hpp"

SoundNode::SoundNode(SoundPlayer& player)
	:SceneNode()
	,mSounds(player)
{

}

void SoundNode::playSound(SoundEffect::ID sound, sf::Vector2f position)
{
	mSounds.play(sound, position);
}

void SoundNode::playSound(SoundEffect::ID sound, sf::Vector2f position, bool looped)
{
	mSounds.play(sound, position, looped);
}



unsigned int SoundNode::getCategory() const
{
	return Category::SoundEffect;
}