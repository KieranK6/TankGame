
#include "Entity.hpp"
#include "Command.hpp"
#include "ResourceIdentifiers.hpp"

#include "DataTables.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <SFML/Graphics/Sprite.hpp>

class Obstacle : public Entity
{
public:
	enum ObType
	{
		Rock,
		Barricade,
		TypeCount
	};

public:
	Obstacle(ObType type, const TextureHolder& textures);


	~Obstacle();
	sf::FloatRect	getBoundingRect() const;

	

private:
	ObType					mType;
	sf::Sprite				mSprite;
	int						mIdentifier;

};
