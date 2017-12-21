#include "DataTables.hpp"
#include "Tank.hpp"
#include "Tank.hpp"
#include "Projectile.hpp"
#include "Pickup.hpp"
#include "Particle.hpp"
#include "Obstacle.hpp"

// For std::bind() placeholders _1, _2, ...
using namespace std::placeholders;

//used count and added pixel variables to make implementation quicker. 
std::vector<TankData> initializeTankData()
{
	int pixelWidth = 64;
	int pixelHeight = 64;

	std::vector<TankData> data(Tank::TypeCount);

	data[Tank::Hotchkiss].hitpoints = 100;
	data[Tank::Hotchkiss].speed = 200.f;
	data[Tank::Hotchkiss].ammoCount = 5;
	data[Tank::Hotchkiss].fireInterval = sf::seconds(4);
	data[Tank::Hotchkiss].texture = Textures::TankChassisEntities;
	data[Tank::Hotchkiss].textureRect = sf::IntRect(0 * pixelWidth, 0, pixelWidth, pixelHeight);

	data[Tank::T34].hitpoints = 150;
	data[Tank::T34].speed = 150.f;
	data[Tank::T34].ammoCount = 100;
	data[Tank::T34].fireInterval = sf::seconds(1);
	data[Tank::T34].texture = Textures::TankChassisEntities;
	data[Tank::T34].textureRect = sf::IntRect(1 * pixelWidth, 0, pixelWidth, pixelHeight);

	data[Tank::Panzer].hitpoints = 100;
	data[Tank::Panzer].speed = 200.f;
	data[Tank::Panzer].ammoCount = 100;
	data[Tank::Panzer].fireInterval = sf::seconds(1);
	data[Tank::Panzer].texture = Textures::TankChassisEntities;
	data[Tank::Panzer].textureRect = sf::IntRect(2 * pixelWidth, 0, pixelWidth, pixelHeight);

	data[Tank::Panther].hitpoints = 100;
	data[Tank::Panther].speed = 200.f;
	data[Tank::Panther].ammoCount = 100;
	data[Tank::Panther].fireInterval = sf::seconds(1);
	data[Tank::Panther].texture = Textures::TankChassisEntities;
	data[Tank::Panther].textureRect = sf::IntRect(3 * pixelWidth, 0, pixelWidth, pixelHeight);

	return data;
}

//used count and added pixel variables to make implementation quicker. 
std::vector<ObstacleData> initializeObstacleData()
{
	int pixelWidth = 64;
	int pixelHeight = 64;

	std::vector<ObstacleData> data(Obstacle::TypeCount);

	data[Obstacle::Barricade].hitpoints = 100;
	data[Obstacle::Barricade].texture = Textures::Obstacles;
	data[Obstacle::Barricade].textureRect = sf::IntRect(0 * pixelWidth, 0, pixelWidth, pixelHeight);

	data[Obstacle::Rock].hitpoints = 150;
	data[Obstacle::Rock].texture = Textures::Obstacles;
	data[Obstacle::Rock].textureRect = sf::IntRect(1 * pixelWidth, 0, pixelWidth, pixelHeight);



	return data;
}

std::vector<TankTurretData> initializeTankTurretData()
{
	int pixelWidth = 128;
	int pixelHeight = 128;

	std::vector<TankTurretData> data(Tank::TypeCount);

	data[Tank::HotchkissTurret].rotationSpeed = 1.1f;
	data[Tank::HotchkissTurret].maxRotationSpeed = 110.0f;
	data[Tank::HotchkissTurret].fireInterval = sf::seconds(1);
	data[Tank::HotchkissTurret].texture = Textures::TankTurretEntities;
	data[Tank::HotchkissTurret].textureRect = sf::IntRect(0 * pixelWidth, 0, pixelWidth, pixelHeight);

	data[Tank::T34Turret].rotationSpeed = 0.95f;
	data[Tank::T34Turret].maxRotationSpeed = 110.0f ;
	data[Tank::T34Turret].fireInterval = sf::seconds(1);
	data[Tank::T34Turret].texture = Textures::TankTurretEntities;
	data[Tank::T34Turret].textureRect = sf::IntRect(1 * pixelWidth, 0, pixelWidth, pixelHeight);

	data[Tank::PantherTurret].rotationSpeed = 0.8f;
	data[Tank::PantherTurret].maxRotationSpeed = 110.0f;
	data[Tank::PantherTurret].fireInterval = sf::seconds(1);
	data[Tank::PantherTurret].texture = Textures::TankTurretEntities;
	data[Tank::PantherTurret].textureRect = sf::IntRect(2 * pixelWidth, 0, pixelWidth, pixelHeight);

	data[Tank::PanzerTurret].rotationSpeed = 0.85f;
	data[Tank::PanzerTurret].maxRotationSpeed = 110.0f;
	data[Tank::PanzerTurret].fireInterval = sf::seconds(1);
	data[Tank::PanzerTurret].texture = Textures::TankTurretEntities;
	data[Tank::PanzerTurret].textureRect = sf::IntRect(3 * pixelWidth, 0, pixelWidth, pixelHeight);

	return data;
}


std::vector<ProjectileData> initializeProjectileData()
{
	std::vector<ProjectileData> data(Projectile::TypeCount);

	data[Projectile::AlliedBullet].damage = 100;
	data[Projectile::AlliedBullet].speed = 300.f;
	data[Projectile::AlliedBullet].texture = Textures::Entities;
	data[Projectile::AlliedBullet].textureRect = sf::IntRect(175, 64, 3, 14);

	data[Projectile::EnemyBullet].damage = 10;
	data[Projectile::EnemyBullet].speed = 300.f;
	data[Projectile::EnemyBullet].texture = Textures::Entities;
	data[Projectile::EnemyBullet].textureRect = sf::IntRect(178, 64, 3, 14);

	data[Projectile::Missile].damage = 200;
	data[Projectile::Missile].speed = 150.f;
	data[Projectile::Missile].texture = Textures::Entities;
	data[Projectile::Missile].textureRect = sf::IntRect(160, 64, 15, 32);

	return data;
}

std::vector<PickupData> initializePickupData()
{
	std::vector<PickupData> data(Pickup::TypeCount);

	data[Pickup::HealthRefill].texture = Textures::Entities;
	data[Pickup::HealthRefill].textureRect = sf::IntRect(0, 64, 40, 40);
	data[Pickup::HealthRefill].action = [](Tank& a) { a.repair(25); };

	data[Pickup::MissileRefill].texture = Textures::Entities;
	data[Pickup::MissileRefill].textureRect = sf::IntRect(40, 64, 40, 40);
	data[Pickup::MissileRefill].action = std::bind(&Tank::collectMissiles, _1, 3);

	data[Pickup::FireSpread].texture = Textures::Entities;
	data[Pickup::FireSpread].textureRect = sf::IntRect(80, 64, 40, 40);
	data[Pickup::FireSpread].action = std::bind(&Tank::increaseSpread, _1);

	data[Pickup::FireRate].texture = Textures::Entities;
	data[Pickup::FireRate].textureRect = sf::IntRect(120, 64, 40, 40);
	data[Pickup::FireRate].action = std::bind(&Tank::increaseFireRate, _1);

	return data;
}

std::vector<ParticleData> initializeParticleData()
{

	std::vector<ParticleData> data(Particle::ParticleCount);

	data[Particle::Propellant].color = sf::Color(255, 255, 50);
	data[Particle::Propellant].lifetime = sf::seconds(0.6f);

	data[Particle::Smoke].color = sf::Color(50, 50, 50);
	data[Particle::Smoke].lifetime = sf::seconds(4.f);

	return data;
}

