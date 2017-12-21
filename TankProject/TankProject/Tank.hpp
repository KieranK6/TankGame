#pragma once
#include "Entity.hpp"
#include "Command.hpp"
#include "ResourceIdentifiers.hpp"
#include "Projectile.hpp"
#include "TextNode.hpp"
#include "Animation.hpp"

#include <SFML/Graphics/Sprite.hpp>


class Tank : public Entity
{
public:
	enum Type
	{
		Hotchkiss,
		T34,
		Panzer,
		Panther,
		HotchkissTurret,
		T34Turret,
		PanzerTurret,
		PantherTurret,
		TypeCount
	};


public:
	Tank(Type type, const TextureHolder& textures, const FontHolder& fonts);

	virtual unsigned int	getCategory() const;
	virtual sf::FloatRect	getBoundingRect() const;
	virtual void			remove();
	virtual bool 			isMarkedForRemoval() const;
	bool					isAllied() const;
	float					getMaxSpeed() const;
	float					getMaxTurretRotationSpeed() const;
	float					getTurretRotationSpeed() const;
	void					disablePickups();

	void					increaseFireRate();
	void					increaseSpread();
	void					collectAmmo(unsigned int count);

	void 					fire();
	void					launchMissile();
	void					playLocalSound(CommandQueue& commands, SoundEffect::ID effect);
	int						getIdentifier();
	void					setIdentifier(int identifier);
	int						getMissileAmmo() const;
	void					setMissileAmmo(int ammo);
	int						getAmmoCount() const;
	void					accelerateTurretRotation(float rotationVelocity);
	void					setTurretRotationVelocity(float rotationVelocity);


private:
	virtual void			drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
	virtual void 			updateCurrent(sf::Time dt, CommandQueue& commands);
	void					updateMovementPattern(sf::Time dt);
	void					checkPickupDrop(CommandQueue& commands);
	void					checkProjectileLaunch(sf::Time dt, CommandQueue& commands);

	void					createBullets(SceneNode& node, const TextureHolder& textures) const;
	void					createProjectile(SceneNode& node, Projectile::Type type, float xOffset, float yOffset, const TextureHolder& textures) const;
	void					createPickup(SceneNode& node, const TextureHolder& textures) const;

	void					updateTexts();
	void					updateRollAnimation();
	void					updateTurret(sf::Time dt);
	float					getTotalTurretRotation() const;


private:
	Type					mType;
	sf::Sprite				mSprite;
	Animation				mExplosion;
	Command 				mFireCommand;
	Command					mMissileCommand;
	sf::Time				mFireCountdown;
	bool 					mIsFiring;
	bool					mIsLaunchingMissile;
	bool 					mShowExplosion;
	bool					mExplosionBegan;
	bool					mPlayedExplosionSound;
	bool					mSpawnedPickup;
	bool					mPickupsEnabled;

	int						mFireRateLevel;
	int						mSpreadLevel;
	int						mMissileAmmo;

	Command 				mDropPickupCommand;
	float					mTravelledDistance;
	std::size_t				mDirectionIndex;
	TextNode*				mHealthDisplay;
	TextNode*				ammoDisplay;

	int						mIdentifier;

	int						ammoCount;

	Type					turretType;
	sf::Sprite				turretSprite;
	float					turretRotationVelocity;
	float					turretOldRotation;
	bool					isRotating;
};