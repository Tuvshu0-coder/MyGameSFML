#include "Game.h"
#include <iostream>

Game::Game() : m_window(sf::VideoMode({ 700, 500 }), "Learning SFML game dev")


//sf::RenderWindow window(sf::VideoMode({700, 500}), "Learning SFML");
{

	m_window.setFramerateLimit(120);
	if (!m_player.load("Assets/player.png"))
		std::cerr << "Failed to load player texture!  fix the clanker\n";
}

void Game::run()
{
	sf::Clock clock;

	while (m_window.isOpen())
	{
		float deltaTime = clock.restart().asSeconds();

		processEvents();
		update(deltaTime);
		render();
	}
}

void Game::processEvents()
{
	while (const std::optional event = m_window.pollEvent())
	{
		if (event->is<sf::Event::Closed>())
			m_window.close();
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
		m_player.jump();
}

void Game::update(float deltaTime) 
{
	const float move = m_player.getSpeed() * deltaTime;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
	{
		m_player.move(-move, 0.f);
		m_player.setDirection(Player::Direction::Left);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
	{
		m_player.move(move, 0.f);
		m_player.setDirection(Player::Direction::Right);
	}

	m_player.applyGravity(deltaTime);
	m_player.update(m_window.getSize());
}

void Game::render()
{
	m_window.clear();
	m_player.draw(m_window);
	m_window.display();
}


