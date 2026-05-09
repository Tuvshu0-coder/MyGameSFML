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

	if (!m_uiFont.openFromFile("Assets/Fonts/PressStart2P-Regular.ttf"))
		std::cerr << "Failed to load UI font!\n";

	if (!m_player.load("Assets/playerSheet.png", m_baseResolution))
		std::cerr << "Failed to load player texture! fix the clanker\n";

	if (!m_world.loadFromTmx("Assets/Maps/map1.tmx"))
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
	updateBackgroundLayout(m_window.getSize());

	if (!m_mainMenuTexture.loadFromFile("Assets/main_menu.png"))
		std::cerr << "Failed to load main menu texture!\n";
	else
	{
		m_mainMenuBackground.emplace(m_mainMenuTexture);
		updateMainMenuLayout(m_window.getSize());
	}

	// Enemy spawn near right side, on ground top line (adjust if needed)
	const sf::Vector2f enemySpawn{ static_cast<float>(m_worldPixelSize.x) - 40.f, 256.f };
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

	updateBackgroundLayout(windowSize);
	updateMainMenuLayout(windowSize);
}

void Game::updateBackgroundLayout(sf::Vector2u windowSize)
{
	if (!m_background)
		return;

	const auto bgSize = m_backgroundTexture.getSize();
	if (bgSize.x == 0 || bgSize.y == 0)
		return;

	const float scaleX = static_cast<float>(windowSize.x) / static_cast<float>(bgSize.x);
	const float scaleY = static_cast<float>(windowSize.y) / static_cast<float>(bgSize.y);

	m_background->setScale({ scaleX, scaleY });
	m_background->setPosition({ 0.f, 0.f });
}

void Game::updateMainMenuLayout(sf::Vector2u windowSize)
{
	if (!m_mainMenuBackground)
		return;

	const auto bgSize = m_mainMenuTexture.getSize();
	if (bgSize.x == 0 || bgSize.y == 0)
		return;

	const float scaleX = static_cast<float>(windowSize.x) / static_cast<float>(bgSize.x);
	const float scaleY = static_cast<float>(windowSize.y) / static_cast<float>(bgSize.y);

	m_mainMenuBackground->setScale({ scaleX, scaleY });
	m_mainMenuBackground->setPosition({ 0.f, 0.f });
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
			const auto key = keyPressed->code;

			if (m_gameState == GameState::MainMenu)
			{
				if (key == sf::Keyboard::Key::Up || key == sf::Keyboard::Key::W)
					m_mainMenuSelection = MainMenuOption::Start;
				else if (key == sf::Keyboard::Key::Down || key == sf::Keyboard::Key::S)
					m_mainMenuSelection = MainMenuOption::Quit;
				else if (key == sf::Keyboard::Key::Enter)
				{
					if (m_mainMenuSelection == MainMenuOption::Start)
					{
						resetGame();
						m_gameState = GameState::Playing;
					}
					else
					{
						m_window.close();
					}
				}
				else if (key == sf::Keyboard::Key::Escape || key == sf::Keyboard::Key::Q)
				{
					m_window.close();
				}
			}
			else if (m_gameState == GameState::Playing)
			{
				if (key == sf::Keyboard::Key::J)
					m_player.attack();
				else if (key == sf::Keyboard::Key::P || key == sf::Keyboard::Key::Escape)
					m_gameState = GameState::Paused;
			}
			else if (m_gameState == GameState::Paused)
			{
				if (key == sf::Keyboard::Key::P || key == sf::Keyboard::Key::Escape)
					m_gameState = GameState::Playing;
				else if (key == sf::Keyboard::Key::Q)
					m_window.close();
			}
			else if (m_gameState == GameState::DeathMenu)
			{
				if (key == sf::Keyboard::Key::R || key == sf::Keyboard::Key::Enter)
				{
					resetGame();
					m_gameState = GameState::Playing;
				}
				else if (key == sf::Keyboard::Key::Q || key == sf::Keyboard::Key::Escape)
				{
					m_window.close();
				}
			}
		}
	}

	if (m_gameState == GameState::Playing && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
		m_player.jump();
}

void Game::update(float deltaTime)
{
	if (m_gameState != GameState::Playing)
		return;

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

	if (m_player.isDead())
		m_gameState = GameState::DeathMenu;

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

void Game::drawOverlay(const std::string& title, const std::string& body)
{
	m_window.setView(m_window.getDefaultView());

	const sf::Vector2u ws = m_window.getSize();
	sf::RectangleShape dim({ static_cast<float>(ws.x), static_cast<float>(ws.y) });
	dim.setFillColor(sf::Color(0, 0, 0, 160));
	m_window.draw(dim);

	sf::Text titleText(m_uiFont);
	titleText.setString(title);
	titleText.setCharacterSize(28);
	titleText.setFillColor(sf::Color::White);
	const sf::FloatRect titleBounds = titleText.getLocalBounds();
	titleText.setOrigin({ titleBounds.position.x + titleBounds.size.x * 0.5f, titleBounds.position.y + titleBounds.size.y * 0.5f });
	titleText.setPosition({ static_cast<float>(ws.x) * 0.5f, static_cast<float>(ws.y) * 0.4f });

	sf::Text bodyText(m_uiFont);
	bodyText.setString(body);
	bodyText.setCharacterSize(16);
	bodyText.setFillColor(sf::Color(220, 220, 220));
	const sf::FloatRect bodyBounds = bodyText.getLocalBounds();
	bodyText.setOrigin({ bodyBounds.position.x + bodyBounds.size.x * 0.5f, bodyBounds.position.y + bodyBounds.size.y * 0.5f });
	bodyText.setPosition({ static_cast<float>(ws.x) * 0.5f, static_cast<float>(ws.y) * 0.55f });

	m_window.draw(titleText);
	m_window.draw(bodyText);

	m_window.setView(m_view);
}

void Game::drawMainMenu()
{
	m_window.setView(m_window.getDefaultView());

	const sf::Vector2u ws = m_window.getSize();
	const float centerX = static_cast<float>(ws.x) * 0.5f;
	const float baseY = static_cast<float>(ws.y) * 0.55f;

	sf::Text titleText(m_uiFont);
	titleText.setString("MyHardGame");
	titleText.setCharacterSize(32);
	titleText.setFillColor(sf::Color::White);
	const sf::FloatRect titleBounds = titleText.getLocalBounds();
	titleText.setOrigin({ titleBounds.position.x + titleBounds.size.x * 0.5f, titleBounds.position.y + titleBounds.size.y * 0.5f });
	titleText.setPosition({ centerX, static_cast<float>(ws.y) * 0.28f });
	m_window.draw(titleText);

	const float buttonW = 200.f;
	const float buttonH = 40.f;
	const float buttonSpacing = 14.f;

	const auto drawButton = [&](const std::string& label, float centerY, bool selected)
	{
		sf::RectangleShape button({ buttonW, buttonH });
		button.setOrigin({ buttonW * 0.5f, buttonH * 0.5f });
		button.setPosition({ centerX, centerY });
		button.setFillColor(selected ? sf::Color(60, 120, 240, 220) : sf::Color(20, 20, 20, 180));
		button.setOutlineThickness(2.f);
		button.setOutlineColor(selected ? sf::Color::White : sf::Color(120, 120, 120));

		sf::Text text(m_uiFont);
		text.setString(label);
		text.setCharacterSize(18);
		text.setFillColor(sf::Color::White);
		const sf::FloatRect textBounds = text.getLocalBounds();
		text.setOrigin({ textBounds.position.x + textBounds.size.x * 0.5f, textBounds.position.y + textBounds.size.y * 0.5f });
		text.setPosition({ centerX, centerY });

		m_window.draw(button);
		m_window.draw(text);
	};

	drawButton("Start", baseY, m_mainMenuSelection == MainMenuOption::Start);
	drawButton("Quit", baseY + buttonH + buttonSpacing, m_mainMenuSelection == MainMenuOption::Quit);

	m_window.setView(m_view);
}

void Game::resetGame()
{
	m_player = Player();
	if (!m_player.load("Assets/playerSheet.png", m_baseResolution))
		std::cerr << "Failed to reload player texture!\n";

	const float worldWidth = static_cast<float>(m_worldPixelSize.x);
	const sf::Vector2f enemySpawn{ worldWidth - 40.f, 256.f };

	m_enemy = Enemy();
	if (!m_enemy.load("Assets/enemySheet.png", enemySpawn))
		std::cerr << "Failed to reload enemy sheet!\n";

	m_view.setCenter({ static_cast<float>(m_baseResolution.x) * 0.5f, static_cast<float>(m_baseResolution.y) * 0.5f });
	updateCamera(0.f);
}

void Game::render()
{
	m_window.clear();

	if (m_gameState == GameState::MainMenu)
	{
		m_window.setView(m_window.getDefaultView());
		if (m_mainMenuBackground)
			m_window.draw(*m_mainMenuBackground);
		else if (m_background)
			m_window.draw(*m_background);

		drawMainMenu();
		m_window.display();
		return;
	}

	m_window.setView(m_window.getDefaultView());
	if (m_background)
		m_window.draw(*m_background);

	m_window.setView(m_view);
	m_world.drawBack(m_window);
	m_enemy.draw(m_window);
	m_player.draw(m_window);
	m_world.drawFront(m_window);

	drawUI();

	if (m_gameState == GameState::Paused)
		drawOverlay("Paused", "P or Esc - Resume\nQ - Quit");
	else if (m_gameState == GameState::DeathMenu)
		drawOverlay("You Died", "R or Enter - Retry\nQ or Esc - Quit");

	m_window.display();
}
