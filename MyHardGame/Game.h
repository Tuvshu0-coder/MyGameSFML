#pragma once
#include <SFML/Graphics.hpp>
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
};