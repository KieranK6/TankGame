#include "SoundPlayer.hpp"

#include <SFML/Audio/Listener.hpp>

#include <cmath>

namespace
{
	//Sound coordinate system, point of view of the player in front of the screen
	//X = left, Y = up, Z = (out of the screen)
	const float ListenerZ = 300.f;
	const float Attenuation = 8.f;
	const float MinDistance2D = 200.f;
	const float MinDistance3D = std::sqrt(MinDistance2D * MinDistance2D + ListenerZ * ListenerZ);
}

SoundPlayer::SoundPlayer()
	:mSoundBuffers()
	, mSounds()
{
	//legacy
	mSoundBuffers.load(SoundEffect::AlliedGunfire, "Media/Sound/AlliedGunfire.wav");
	mSoundBuffers.load(SoundEffect::EnemyGunfire, "Media/Sound/EnemyGunfire.wav");
	//Explosions
	mSoundBuffers.load(SoundEffect::Explosion1, "Media/Sound/Explosion1.wav");
	mSoundBuffers.load(SoundEffect::Explosion2, "Media/Sound/Explosion2.wav");
	//Pickups
	mSoundBuffers.load(SoundEffect::CollectPickup, "Media/Sound/collectPickup.wav");
	//UI
	mSoundBuffers.load(SoundEffect::Button, "Media/Sound/Button.wav"); 
	//Collision
	mSoundBuffers.load(SoundEffect::Collision, "Media/Sound/Collision.wav");
	//Dialog
	mSoundBuffers.load(SoundEffect::Oohrah, "Media/Sound/oohrah.wav");
	mSoundBuffers.load(SoundEffect::Freedum, "Media/Sound/freedum.wav");
	//Tank movement
	mSoundBuffers.load(SoundEffect::TankIdle, "Media/Sound/EngineLoop.wav");
	mSoundBuffers.load(SoundEffect::TankMove, "Media/Sound/MovementLoop.wav");
	mSoundBuffers.load(SoundEffect::TankTurretRotate, "Media/Sound/TankTurretRotate.wav");
	//Tank Firing
	mSoundBuffers.load(SoundEffect::TankFire, "Media/Sound/TankFire.wav");
	mSoundBuffers.load(SoundEffect::TankReload, "Media/Sound/TankReload.wav");


	//Listener points towards the screen
	sf::Listener::setDirection(0.f, 0.f, -1.f);
}

void SoundPlayer::play(SoundEffect::ID effect)
{
	play(effect, getListenerPosition()); //e.g sound for a button
}


void SoundPlayer::play(SoundEffect::ID effect, sf::Vector2f position)
{
	mSounds.push_back(sf::Sound());
	sf::Sound& sound = mSounds.back();

	sound.setBuffer(mSoundBuffers.get(effect));
	sound.setPosition(position.x, -position.y, 0.f);
	sound.setAttenuation(Attenuation);
	sound.setMinDistance(MinDistance3D);
	sound.play();    //sound for explosions or enemy gunfire
}

void SoundPlayer::play(SoundEffect::ID effect, sf::Vector2f position, bool looped)
{
	mSounds.push_back(sf::Sound());
	sf::Sound& sound = mSounds.back();

	sound.setBuffer(mSoundBuffers.get(effect));
	sound.setPosition(position.x, -position.y, 0.f);
	sound.setAttenuation(Attenuation);
	sound.setMinDistance(MinDistance3D);
	sound.play();    //sound for explosions or enemy gunfire
	if (looped)
		sound.setLoop(true);
}



void SoundPlayer::removeStoppedSounds()
{
	mSounds.remove_if([](const sf::Sound& s)
	{
		return s.getStatus() == sf::Sound::Stopped;
	});
}

void SoundPlayer::setListenerPosition(sf::Vector2f position)
{
	sf::Listener::setPosition(position.x, -position.y, ListenerZ);
}

sf::Vector2f SoundPlayer::getListenerPosition() const
{
	sf::Vector3f position = sf::Listener::getPosition(); //returns a 3D vectors but Z is fixed
	return sf::Vector2f(position.x, -position.y);
}





