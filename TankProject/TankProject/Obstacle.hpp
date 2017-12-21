
#include "Entity.hpp"
#include "Command.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/Sprite.hpp>

class Obstacle : public Entity
{

	enum Type
	{
		Rock,
		Barricade
	};

public:
	Obstacle(Type type, const TextureHolder& textures);

	virtual sf::FloatRect	getBoundingRect() const;

	~Obstacle();

private:
	Type					mType;
	sf::Sprite				mSprite;

};
