#pragma once
#include "SceneNode.hpp"
#include "ResourceIdentifiers.hpp"

class SoundPlayer;

class SoundNode : public SceneNode
{
public:
	explicit SoundNode(SoundPlayer& player);
	void playSound(SoundEffect::ID sound, sf::Vector2f position);
	void playSound(SoundEffect::ID sound, sf::Vector2f position, bool looped);

	virtual unsigned int getCategory() const;

private:
	SoundPlayer& mSounds;
};