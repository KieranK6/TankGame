#pragma once

#include "ResourceIdentifiers.hpp"

#include <SFML/System/Time.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <vector>
#include <functional>

class Tank;
class Tank;

struct Direction
{
	Direction(float angle, float distance)
		: angle(angle), distance(distance)
	{}

	float angle;
	float distance;
};

struct TankData
{
	int hitpoints;
	float speed;
	int ammoCount;
	Textures::ID texture;
	sf::IntRect textureRect;
	sf::Time fireInterval;
	sf::Time rotateInterval;
	std::vector<Direction> directions;
	int turretDataID;
};

struct TankTurretData
{
	float rotationSpeed;
	float maxRotationSpeed;
	Textures::ID texture;
	sf::IntRect textureRect;
	sf::Time fireInterval;
	sf::Time rotateInterval;
	std::vector<Direction> directions;
};

struct ObstacleData
{
	int hitpoints;
	Textures::ID texture;
	sf::IntRect textureRect;
};

struct BaseData
{
	int hitpoints;
	Textures::ID texture;
	sf::IntRect textureRect;
};

struct ProjectileData
{
	int damage;
	float speed;
	Textures::ID texture;
	sf::IntRect textureRect;
};

struct PickupData
{
	std::function<void(Tank&)> action;
	Textures::ID texture;
	sf::IntRect textureRect;
};

struct ParticleData
{
	sf::Color						color;
	sf::Time						lifetime;
};

std::vector<TankData> initializeTankData();
std::vector<TankTurretData> initializeTankTurretData();
std::vector<ObstacleData> initializeObstacleData();
std::vector<BaseData> initializeBaseData();
std::vector<ProjectileData> initializeProjectileData();
std::vector<PickupData> initializePickupData();
std::vector<ParticleData> initializeParticleData();
std::vector<ParticleData> initializeSpawnData();