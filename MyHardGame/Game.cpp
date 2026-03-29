#include "Game.h"
#include <iostream>
#include <algorithm>
#include <cmath>

Game::Game() : m_window(sf::VideoMode({ 854, 480 }), "Learning SFML game dev")
{
	m_window.setFramerateLimit(120);

	const float cameraZoom = 0.75f; // 1.0 = current, 0.75 = zoom in, 0.5 = very zoomed in
	m_view.setSize({
		static_cast<float>(m_baseResolution.x) * cameraZoom,
		static_cast<float>(m_baseResolution.y) * cameraZoom
	});
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

	// Enemy spawn near right side, on ground top line (adjust if needed)
	const sf::Vector2f enemySpawn{ worldWidth - 40.f, 256.f };
	if (!m_enemy.load("Assets/enemySheet.png", enemySpawn))
		std::cerr << "Failed to load enemy sheet!\n";
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

		if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
		{
			if (keyPressed->code == sf::Keyboard::Key::J)
				m_player.attack();
		}
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

	m_enemy.update(deltaTime, m_player.getCenter());

	// Player -> Enemy hit
	if (!m_enemy.isDead() && m_player.consumeAttackHit())
	{
		const float dist = std::abs(m_player.getCenter().x - m_enemy.getCenter().x);
		if (dist <= 50.f)
			m_enemy.takeDamage(15);
	}

	// Enemy -> Player hit
	if (!m_player.isDead() && m_enemy.consumeAttackHit())
	{
		const float dist = std::abs(m_player.getCenter().x - m_enemy.getCenter().x);
		if (dist <= 35.f)
		{
			if (m_player.isBlocking())
			{
				m_enemy.stun(0.7f);
			}
			else
			{
				m_player.takeDamage(20);
			}
		}
	}

	m_player.setBlocking(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::K));

	updateCamera(deltaTime);
}

void Game::drawUI()
{
	m_window.setView(m_window.getDefaultView());

	const float barW = 140.f;
	const float barH = 12.f;
	const float margin = 12.f;

	const sf::Vector2u ws = m_window.getSize();
	const sf::FloatRect vp = m_view.getViewport();

	// Pixel rectangle of the active gameplay viewport (letterboxed area)
	const float uiX = vp.position.x * static_cast<float>(ws.x);
	const float uiY = vp.position.y * static_cast<float>(ws.y);
	const float uiW = vp.size.x * static_cast<float>(ws.x);
	const float uiH = vp.size.y * static_cast<float>(ws.y);

	// Player HP bar (top-left inside gameplay viewport)
	sf::RectangleShape playerBg({ barW, barH });
	playerBg.setPosition({ uiX + margin, uiY + margin });
	playerBg.setFillColor(sf::Color(40, 40, 40));
	playerBg.setOutlineThickness(1.f);
	playerBg.setOutlineColor(sf::Color::White);

	const float playerRatio = static_cast<float>(m_player.getHealth()) / static_cast<float>(std::max(1, m_player.getMaxHealth()));
	sf::RectangleShape playerHp({ barW * std::clamp(playerRatio, 0.f, 1.f), barH });
	playerHp.setPosition({ uiX + margin, uiY + margin });
	playerHp.setFillColor(sf::Color(80, 220, 90));

	m_window.draw(playerBg);
	m_window.draw(playerHp);

	// Enemy HP bar (top-right inside gameplay viewport)
	const int enemyHpValue = std::max(0, m_enemy.getHealth());
	const int enemyMaxValue = std::max(1, m_enemy.getMaxHealth());
	const float enemyRatio = static_cast<float>(enemyHpValue) / static_cast<float>(enemyMaxValue);

	const float enemyBarX = uiX + uiW - barW - margin;
	const float enemyBarY = uiY + margin;

	sf::RectangleShape enemyBg({ barW, barH });
	enemyBg.setPosition({ enemyBarX, enemyBarY });
	enemyBg.setFillColor(sf::Color(25, 25, 25, 220));
	enemyBg.setOutlineThickness(2.f);
	enemyBg.setOutlineColor(sf::Color::White);

	float enemyFillW = barW * std::clamp(enemyRatio, 0.f, 1.f);
	if (enemyHpValue > 0 && enemyFillW < 2.f)
		enemyFillW = 2.f;

	sf::RectangleShape enemyHp({ enemyFillW, barH });
	enemyHp.setPosition({ enemyBarX, enemyBarY });
	enemyHp.setFillColor(sf::Color(220, 70, 70));

	m_window.draw(enemyBg);
	m_window.draw(enemyHp);

	m_window.setView(m_view);
}

void Game::render()
{
	m_window.clear();

	if (m_background)
		m_window.draw(*m_background);

	m_world.drawBack(m_window);
	m_enemy.draw(m_window);
	m_player.draw(m_window);
	m_world.drawFront(m_window);

	drawUI();

	m_window.display();
}


