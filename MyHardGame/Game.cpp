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
		std::cerr << "Failed to load player texture! fix the clanker\n";

	if (!m_world.loadFromTmx("Assets/Maps/level01.tmx"))
	{
		std::cerr << "Failed to load TMX map: Assets/Maps/level01.tmx\n";
	}
	else
	{
		m_worldPixelSize = m_world.getPixelSize();
		m_platforms = m_world.getColliders();
	}

	if (!m_backgroundTexture.loadFromFile("Assets/background.png"))
		std::cerr << "Failed to load background texture!\n";

	m_background.emplace(m_backgroundTexture);
	const auto bgSize = m_backgroundTexture.getSize();
	const float worldWidth = static_cast<float>(m_worldPixelSize.x);
	const float worldHeight = static_cast<float>(m_worldPixelSize.y);

	const float scaleX = worldWidth / static_cast<float>(bgSize.x);
	const float scaleY = worldHeight / static_cast<float>(bgSize.y);
	const float scale = std::max(scaleX, scaleY);

	m_background->setScale({ scale, scale });

	const float bgWidth = static_cast<float>(bgSize.x) * scale;
	const float bgHeight = static_cast<float>(bgSize.y) * scale;
	m_background->setPosition({ (worldWidth - bgWidth) * 0.5f, (worldHeight - bgHeight) * 0.5f });
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

void Game::updateCamera(float deltaTime)
{
	const sf::FloatRect playerBounds = m_player.getGlobalBounds();

	float targetX = playerBounds.position.x + playerBounds.size.x * 0.5f;
	float targetY = playerBounds.position.y + playerBounds.size.y * 0.5f;

	const float halfViewW = m_view.getSize().x * 0.5f;
	const float halfViewH = m_view.getSize().y * 0.5f;
	const float worldW = static_cast<float>(m_worldPixelSize.x);
	const float worldH = static_cast<float>(m_worldPixelSize.y);

	// Clamp target to world bounds first
	if (worldW <= m_view.getSize().x)
		targetX = worldW * 0.5f;
	else
		targetX = std::clamp(targetX, halfViewW, worldW - halfViewW);

	if (worldH <= m_view.getSize().y)
		targetY = worldH * 0.5f;
	else
		targetY = std::clamp(targetY, halfViewH, worldH - halfViewH);

	sf::Vector2f center = m_view.getCenter();

	// Dead-zone to remove tiny jitter
	if (std::abs(targetX - center.x) <= m_cameraDeadZone)
		targetX = center.x;
	if (std::abs(targetY - center.y) <= m_cameraDeadZone)
		targetY = center.y;

	// Exponential smoothing (frame-rate independent)
	const float alpha = 1.f - std::exp(-m_cameraFollowSpeed * deltaTime);
	center.x += (targetX - center.x) * alpha;
	center.y += (targetY - center.y) * alpha;

	// Clamp final center
	if (worldW <= m_view.getSize().x)
		center.x = worldW * 0.5f;
	else
		center.x = std::clamp(center.x, halfViewW, worldW - halfViewW);

	if (worldH <= m_view.getSize().y)
		center.y = worldH * 0.5f;
	else
		center.y = std::clamp(center.y, halfViewH, worldH - halfViewH);

	// Pixel snap helps pixel-art shimmer
	center.x = std::round(center.x);
	center.y = std::round(center.y);

	m_view.setCenter(center);
	m_window.setView(m_view);
}

void Game::run()
{
	sf::Clock clock;

	while (m_window.isOpen())
	{
		float deltaTime = clock.restart().asSeconds();
		deltaTime = std::min(deltaTime, 1.f / 30.f); // avoid huge spikes

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
			updateView({ resized->size.x, resized->size.y });
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
		m_player.jump();
}

void Game::update(float deltaTime)
{
	int inputDir = 0;
	const bool isRunning = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift);

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) inputDir = -1;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) inputDir = 1;

	m_player.setHorizontalInput(inputDir, isRunning);
	m_player.applyGravity(deltaTime);
	m_player.update(deltaTime, m_worldPixelSize);
	m_player.resolveCollisions(m_platforms);

	updateCamera(deltaTime);
}

void Game::render()
{
	m_window.clear();

	if (m_background)
		m_window.draw(*m_background);

	m_world.drawBack(m_window);
	m_player.draw(m_window);
	m_world.drawFront(m_window);

	m_window.display();
}



















