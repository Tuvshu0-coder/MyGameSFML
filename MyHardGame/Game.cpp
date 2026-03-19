#include "Game.h"
#include <iostream>

Game::Game() : m_window(sf::VideoMode({ 700, 500 }), "Learning SFML game dev")
{
	m_window.setFramerateLimit(120);
	if (!m_player.load("Assets/player.png"))
		std::cerr << "Failed to load player texture!  fix the clanker\n";

	if (!m_backgroundTexture.loadFromFile("Assets/background.png"))
		std::cerr << "Failed to load background texture!\n";

	m_background.emplace(m_backgroundTexture);
	const auto bgSize = m_backgroundTexture.getSize();
	const auto winSize = m_window.getSize();

	const float scaleX = static_cast<float>(winSize.x) / static_cast<float>(bgSize.x);
	const float scaleY = static_cast<float>(winSize.y) / static_cast<float>(bgSize.y);
	const float scale = std::max(scaleX, scaleY);

	m_background->setScale({ scale, scale });

	const float bgWidth = static_cast<float>(bgSize.x) * scale;
	const float bgHeight = static_cast<float>(bgSize.y) * scale;
	m_background->setPosition({ (winSize.x - bgWidth) * 0.5f, (winSize.y - bgHeight) * 0.5f });

	if (!m_groundTexture.loadFromFile("Assets/ground.png"))
		std::cerr << "Failed to load ground texture!\n";

	m_groundTexture.setRepeated(true);

	const float tileSize = 46.f;
	const float grassHeight = 15.f; // adjust to match your image
	const float groundHeight = 92.f; // total ground height on screen

	const float groundTop = 500.f - groundHeight;

	m_groundGrass.setSize({ 700.f, grassHeight });
	m_groundGrass.setPosition({ 0.f, groundTop });
	m_groundGrass.setTexture(&m_groundTexture);
	m_groundGrass.setTextureRect(sf::IntRect({ 0, 0 }, { 700, static_cast<int>(grassHeight) }));

	m_groundDirt.setSize({ 700.f, groundHeight - grassHeight });
	m_groundDirt.setPosition({ 0.f, groundTop + grassHeight });
	m_groundDirt.setTexture(&m_groundTexture);
	m_groundDirt.setTextureRect(sf::IntRect({ 0, static_cast<int>(grassHeight) }, { 700, static_cast<int>(groundHeight - grassHeight) }));

	m_platforms.push_back(sf::FloatRect({ 0.f, groundTop }, { 700.f, groundHeight }));
}
// </ground>
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
	m_player.resolveCollisions(m_platforms); //ground
}

void Game::render()
{
	m_window.clear();
	if (m_background)
		m_window.draw(*m_background);
	m_window.draw(m_groundGrass);
	m_window.draw(m_groundDirt);
	m_player.draw(m_window);
	m_window.display();
}


