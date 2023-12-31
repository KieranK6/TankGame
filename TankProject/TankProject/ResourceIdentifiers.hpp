#pragma once
//Forward declaration of SFML classes
namespace sf
{
	class Texture;
	class Font;
	class Shader;
	class SoundBuffer;
}

namespace Textures
{
	enum ID
	{
		Entities,
		TankChassisEntities,
		TankTurretEntities,
		Desert,
		TitleScreen,
		Buttons,
		Explosion,
		Particle,
		FinishLine,
		Obstacles,
		Walls,
		EnemyBase,
		LiberatorsBase,
		ResistanceBase,
	};
}

namespace Shaders
{
	enum ID
	{
		BrightnessPass,
		DownSamplePass,
		GaussianBlurPass,
		AddPass,
	};
}

namespace Fonts
{
	enum ID
	{
		Main,
		Clear,
	};
}

namespace Music
{
	enum ID
	{
		MenuTheme,
		MissionTheme,
	};
}

namespace SoundEffect
{
	enum ID
	{
		AlliedGunfire,
		EnemyGunfire,
		Explosion1,
		Explosion2,
		CollectPickup,
		Button,
		Collision,
		Oohrah,
		Freedum,
		TankIdle,
		TankMove,
		TankTurretRotate,
		TankFire,
		TankReload,
	};
}


//Forward declaration 
template<typename Resource, typename Identifier> 
class ResourceHolder;

typedef ResourceHolder<sf::Texture, Textures::ID> TextureHolder;
typedef ResourceHolder<sf::Font, Fonts::ID> FontHolder;
typedef ResourceHolder<sf::Shader, Shaders::ID>	ShaderHolder;
typedef ResourceHolder<sf::SoundBuffer, SoundEffect::ID> SoundBufferHolder; //none for music, as music is streamed rather than loaded into RAM
