#ifndef BOOK_CATEGORY_HPP
#define BOOK_CATEGORY_HPP


// Entity/scene node category, used to dispatch commands
namespace Category
{
	enum Type
	{
		None				= 0,
		SceneAirLayer		= 1 << 0,
		PlayerTank			= 1 << 1,
		AlliedTank			= 1 << 2,
		EnemyTank			= 1 << 3,
		Pickup              = 1 << 4,
		AlliedProjectile    = 1 << 5, 
		EnemyProjectile     = 1 << 6,
		ParticleSystem		= 1 << 7,
		SoundEffect			= 1 << 8,
		Network				= 1 << 9,
		Obstacle			= 1 << 10,
		Base				= 1 << 11,

		Tank = PlayerTank | AlliedTank | EnemyTank,
		Projectile = AlliedProjectile | EnemyProjectile,
	};
}

#endif // BOOK_CATEGORY_HPP
