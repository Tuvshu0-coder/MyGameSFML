#include "Game.h"
#include <iostream>
#include <algorithm>
#include <cmath>

Game::Game() : m_window(sf::VideoMode({ 854, 480 }), "Learning SFML game dev")
{
	m_window.setFramerateLimit(120);

	m_view.setSize({ static_cast<float>(m_baseResolution.x), static_cast<float>(m_baseResolution.y) });
	m_view.setCenter({ static_cast<float>(m_baseResolution.x) * 0.5f, static_cast<float>(m_baseResolution.y) * 0.5f });
	updateView(m_window.getSize());

	if (!m_player.load("Assets/playerSheet.png", m_baseResolution))
		std::cerr << "Failed to load player texture!  fix the clanker\n";

	if (!m_backgroundTexture.loadFromFile("Assets/background.png"))
		std::cerr << "Failed to load background texture!\n";

	m_background.emplace(m_backgroundTexture);
	const auto bgSize = m_backgroundTexture.getSize();
	const float baseWidth = static_cast<float>(m_baseResolution.x);
	const float baseHeight = static_cast<float>(m_baseResolution.y);

	const float scaleX = baseWidth / static_cast<float>(bgSize.x);
	const float scaleY = baseHeight / static_cast<float>(bgSize.y);
	const float scale = std::max(scaleX, scaleY);

	m_background->setScale({ scale, scale });

	const float bgWidth = static_cast<float>(bgSize.x) * scale;
	const float bgHeight = static_cast<float>(bgSize.y) * scale;
	m_background->setPosition({ (baseWidth - bgWidth) * 0.5f, (baseHeight - bgHeight) * 0.5f });

	// 1. IMPORTANT: Load ONLY the 16x16 block into the texture using the third argument (second is sRgb)
	if (!m_groundTexture.loadFromFile("Assets/ground.png", false, sf::IntRect({ 71, 16 }, { 16, 16 })))
		std::cerr << "Failed to load ground texture!\n";

	m_groundTexture.setRepeated(true);
	
	// Optional but highly recommended for pixel art:
	m_groundTexture.setSmooth(false); 

	const float groundHeight = 16.f; // total ground height on screen
	const float groundTop = baseHeight - groundHeight;

	m_groundDirt.setSize({ baseWidth, groundHeight });
	m_groundDirt.setPosition({ 0.f, groundTop });
	m_groundDirt.setTexture(&m_groundTexture);
	
	// 2. Set TextureRect to start at 0, 0 and stretch to width (BaseResolution.x) and height.
	// Since the texture IS ONLY 16x16 now, it will repeat that tiny image perfectly.
	m_groundDirt.setTextureRect(sf::IntRect({ 0, 0 }, { static_cast<int>(m_baseResolution.x), static_cast<int>(groundHeight) }));

	m_platforms.push_back(sf::FloatRect({ 0.f, groundTop }, { baseWidth, groundHeight }));
}

void Game::updateView(sf::Vector2u windowSize)
{
	const float windowWidth = static_cast<float>(windowSize.x);
	const float windowHeight = static_cast<float>(windowSize.y);

	const float scaleX = windowWidth / static_cast<float>(m_baseResolution.x);
	const float scaleY = windowHeight / static_cast<float>(m_baseResolution.y);
	const float scale = std::max(1.f, std::floor(std::min(scaleX, scaleY)));

	const float viewWidth = static_cast<float>(m_baseResolution.x) * scale;
	const float viewHeight = static_cast<float>(m_baseResolution.y) * scale;

	const float viewportX = (windowWidth - viewWidth) / windowWidth * 0.5f;
	const float viewportY = (windowHeight - viewHeight) / windowHeight * 0.5f;
	const float viewportW = viewWidth / windowWidth;
	const float viewportH = viewHeight / windowHeight;

	m_view.setViewport(sf::FloatRect({ viewportX, viewportY }, { viewportW, viewportH }));
	m_window.setView(m_view);
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

		if (const auto* resized = event->getIf<sf::Event::Resized>())
		{
			updateView({ resized->size.x, resized->size.y });
		}
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
		m_player.jump();
}

void Game::update(float deltaTime) 
{
	int inputDir = 0;
	bool isRunning = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift);

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) inputDir = -1;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) inputDir = 1;

	// Tell the player the raw output of the keyboard
	m_player.setHorizontalInput(inputDir, isRunning);

	m_player.applyGravity(deltaTime);
	m_player.update(deltaTime, m_baseResolution); 
	m_player.resolveCollisions(m_platforms); 
}

void Game::render()
{
	m_window.clear();
	if (m_background)
		m_window.draw(*m_background);
	m_window.draw(m_groundDirt);
	m_player.draw(m_window);
	m_window.display();
}



















