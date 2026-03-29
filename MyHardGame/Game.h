#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include "Player.h"
#include "World.h"
#include "Enemy.h"

class Game
{
public:
	Game();
	void run(); //main loop

private:
	void processEvents();
	void update(float deltaTime);
	void render();
	void updateView(sf::Vector2u windowSize);
	void updateCamera(float deltaTime);
	void drawUI();

private:
	const sf::Vector2u m_baseResolution{ 426, 240 };
	sf::View m_view;

	sf::Vector2u m_worldPixelSize{ 426, 240 };

	float m_cameraFollowSpeed = 12.f;
	float m_cameraDeadZone = 2.f;

	sf::RenderWindow m_window;
	Player m_player;
	Enemy m_enemy;
	World m_world;

	sf::Texture m_backgroundTexture;
	std::optional<sf::Sprite> m_background;

	std::vector<sf::FloatRect> m_platforms;
};