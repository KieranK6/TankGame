#ifndef BOOK_CATEGORY_HPP
#define BOOK_CATEGORY_HPP


// Entity/scene node category, used to dispatch commands
namespace Category
{
	enum Type
	{
		None				= 0,
		SceneAirLayer		= 1 << 0,
		LiberatorTank			= 1 << 1,
		AlliedTank			= 1 << 2,
		ResistanceTank			= 1 << 3,
		Pickup              = 1 << 4,
		LiberatorProjectile    = 1 << 5, 
		ResistanceProjectile     = 1 << 6,
		ParticleSystem		= 1 << 7,
		SoundEffect			= 1 << 8,
		Network				= 1 << 9,
		EnemyRotation		= 1 << 10,
		Obstacle			= 1 << 11,
		LiberatorsBase		= 1 << 12,
		ResistanceBase		= 1 << 13,


		Tank = LiberatorTank | AlliedTank | ResistanceTank,
		Projectile = LiberatorProjectile | ResistanceProjectile,
		Base = LiberatorsBase | ResistanceBase,
	};
}

#endif // BOOK_CATEGORY_HPP
