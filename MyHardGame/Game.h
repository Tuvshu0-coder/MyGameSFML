#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include "Player.h"
#include "World.h"

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

private:
	const sf::Vector2u m_baseResolution{ 426, 240 };
	sf::View m_view;

	sf::Vector2u m_worldPixelSize{ 426, 240 };

	// Camera tuning
	float m_cameraFollowSpeed = 12.f; // higher = snappier, lower = smoother
	float m_cameraDeadZone = 2.f;     // pixels around center where camera doesn't move

	sf::RenderWindow m_window;
	Player m_player;
	World m_world;

	sf::Texture m_backgroundTexture;
	std::optional<sf::Sprite> m_background;

	std::vector<sf::FloatRect> m_platforms;
};