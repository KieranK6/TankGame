
#include "Entity.hpp"
#include "Command.hpp"
#include "ResourceIdentifiers.hpp"

#include "DataTables.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <SFML/Graphics/Sprite.hpp>

//This class was worked on by Kieran Keegan


class Obstacle : public Entity
{
public:
	enum ObType
	{
		Stone,
		Barricade,
		Wall,
		TypeCount
	};

public:
	Obstacle(ObType type, const TextureHolder& textures);
	unsigned int Obstacle::getCategory() const;

	float getObstacleRadius();
	sf::Sprite				mSprite;


	~Obstacle();
	sf::FloatRect	getBoundingRect() const;

private:
	virtual void			drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;

private:
	ObType					mType;
	
	float					mObRadius = 40.f;
	//int						mIdentifier;

};
