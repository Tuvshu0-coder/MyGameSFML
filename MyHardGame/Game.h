#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include "Player.h"

class Game
{
public:
	Game();
	void run(); //main loop

private:
	void processEvents();
	void update(float deltaTime);
	void render();
private:
	sf::RenderWindow m_window;
	Player m_player;

	sf::Texture m_backgroundTexture;
	std::optional<sf::Sprite> m_background;

	sf::Texture m_groundTexture;
	sf::RectangleShape m_groundGrass;
	sf::RectangleShape m_groundDirt;
	std::vector<sf::FloatRect> m_platforms;
};