#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class World
{
public:
	bool loadFromTmx(const std::string& tmxPath);

	void drawBack(sf::RenderTarget& target) const;
	void drawFront(sf::RenderTarget& target) const;

	const std::vector<sf::FloatRect>& getColliders() const;
	sf::Vector2u getPixelSize() const;

private:
	struct TileLayer
	{
		std::string name;
		sf::VertexArray vertices{ sf::PrimitiveType::Triangles };
	};

	void clear();

private:
	sf::Texture m_tilesetTexture;
	std::vector<TileLayer> m_layers;
	std::vector<sf::FloatRect> m_colliders;

	sf::Vector2u m_mapTileCount{ 0, 0 };
	sf::Vector2u m_tileSize{ 0, 0 };
	sf::Vector2u m_pixelSize{ 0, 0 };

	unsigned int m_firstGid = 1;
	unsigned int m_tilesetColumns = 0;
};