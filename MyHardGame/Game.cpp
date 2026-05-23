#include "Game.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <random>
#include <cstdint>

Game::Game() : m_window(sf::VideoMode({ 854, 480 }), "Get your bugatti back!")
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

	if (!m_backgroundMusic.openFromFile("Assets/music/bg.mp3"))
		std::cerr << "Failed to load background music!\n";
	else
	{
		m_backgroundMusic.setLooping(true);
		m_backgroundMusic.play();
	}

	if (!m_endingMusic.openFromFile("Assets/music/ending_music.mp3"))
		std::cerr << "Failed to load ending music!\n";

	if (!m_levelFinishMusic.openFromFile("Assets/music/level_finish.mp3"))
		std::cerr << "Failed to load level finish music!\n";

	if (!m_enemyDeathBuffer.loadFromFile("Assets/music/combat/enemyDeath.wav"))
		std::cerr << "Failed to load enemy death sound!\n";

	if (!m_swordSwingBuffer.loadFromFile("Assets/music/combat/SwordSwing.wav"))
		std::cerr << "Failed to load sword swing sound!\n";

	if (!m_parryBuffer.loadFromFile("Assets/music/combat/parry.wav"))
		std::cerr << "Failed to load parry sound!\n";

	for (int i = 1; i <= 7; ++i)
	{
		sf::SoundBuffer buffer;
		const std::string path = "Assets/music/combat/enemyHurt" + std::to_string(i) + ".wav";
		if (buffer.loadFromFile(path))
			m_enemyHurtBuffers.push_back(std::move(buffer));
		else
			std::cerr << "Failed to load enemy hurt sound: " << path << "\n";
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

	if (!m_swordsmanTexture.loadFromFile("Assets/swordsman.png"))
		std::cerr << "Failed to load swordsman texture!\n";
	else
	{
		m_swordsmanSprite.emplace(m_swordsmanTexture);
		m_swordsmanSprite->setPosition({ 194.f, 237.f });
	}

	if (!m_bugattiTexture.loadFromFile("Assets/bugatti.png"))
		std::cerr << "Failed to load bugatti texture!\n";
	else
	{
		m_bugattiSprite.emplace(m_bugattiTexture);
		m_bugattiSprite->setScale({ 0.5f, 0.5f });
	}

	m_dialogLines = {
		"Stranger \n\nHello traveler.",
		"Me \n\nHi, Have you seen my Bugatti",
		"Me \n\nI was doin' something real important, y'know, \ntakin' a nap and then... poof.",
		"Me \n\nIt's gone",
		"Stranger \n\nUh, I saw three huge monsters driving \nBugatti at like, 100 miles per hour. They went that way.",
		"Me \n\nThree of em huh?",
		"Me \n\nI'm gonna catch 'em",
		"Stranger \n\nTake these swords with you",
		"Stranger \n\nYou might need it traveller",
	};

	m_tutorialText =
		"Press J to swing your sword\n"
		"A/D - Move Hold LSHIFT to Run\n"
		"Space - Jump\n"
		"K - Block";

	m_levelPaths = {
		"Assets/Maps/map1.tmx",
		"Assets/Maps/map2.tmx",
		"Assets/Maps/map3.tmx"
	};

	m_endingCreditsText =
		"You've gotten your bugatti back!\n"
		"\n"
		"Developer\n"
		"E.Tuvshinzaya (CSE-2, 11th generation)\n"
		"\n"
		"Design\n"
		"Bugatti Retrieval Department (Now Hiring)\n"
		"\n"
		"Level Design\n"
		"E.Tuvshinzaya (Chief Map Scribbler)\n"
		"\n"
		"Combat Choreography\n"
		"Very Serious Sword Club (Mostly Serious)\n"
		"\n"
		"Monsters & Vehicle Handling\n"
		"Street Racing Division (No License Required)\n"
		"\n"
		"Audio Direction\n"
		"Turbo Vinyl Records (Volume at 11)\n"
		"\n"
		"Art & Animation asset\n"
		"Penusbmic - The DARK series\n"
		"\n"
		"QA & Playtesting\n"
		"Bugatti Recovery Task Force (Tested. A lot.)\n"
		"\n"
		"Special Thanks\n"
		"You, for getting the car back\n"
		"Everyone who actually watched the credits\n"
		"That one monster who parked my bugatti properly\n"
		"\n"
		"THE END";

	loadLevel(0, true);
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

void Game::updateSwordsmanInteraction(float deltaTime)
{
	m_showInteractPrompt = false;

	if (m_gameState != GameState::Playing || m_currentLevelIndex != 0 || !m_swordsmanSprite)
		return;

	const sf::Vector2f playerCenter = m_player.getCenter();
	const sf::FloatRect swordBounds = m_swordsmanSprite->getGlobalBounds();
	const sf::Vector2f swordCenter{
		swordBounds.position.x + swordBounds.size.x * 0.5f,
		swordBounds.position.y + swordBounds.size.y * 0.5f
	};

	const float dx = playerCenter.x - swordCenter.x;
	const float dy = playerCenter.y - swordCenter.y;
	const float distance = std::sqrt((dx * dx) + (dy * dy));

	if (distance <= m_interactDistance)
		m_showInteractPrompt = !m_dialogOpen && !m_showTutorialPrompt;

	if (m_dialogOpen && !m_dialogLines.empty())
	{
		const std::string& line = m_dialogLines[m_dialogLineIndex];
		if (m_dialogVisibleChars < line.size())
		{
			m_dialogCharTimer += deltaTime;
			while (m_dialogCharTimer >= m_dialogCharDelay && m_dialogVisibleChars < line.size())
			{
				m_dialogCharTimer -= m_dialogCharDelay;
				++m_dialogVisibleChars;
			}
		}
	}
}

void Game::startDialog()
{
	if (m_dialogLines.empty())
		return;

	m_dialogOpen = true;
	m_showTutorialPrompt = false;
	m_dialogLineIndex = 0;
	m_dialogVisibleChars = 0;
	m_dialogCharTimer = 0.f;
}

void Game::advanceDialog()
{
	if (!m_dialogOpen || m_dialogLines.empty())
		return;

	const std::string& line = m_dialogLines[m_dialogLineIndex];
	if (m_dialogVisibleChars < line.size())
	{
		m_dialogVisibleChars = line.size();
		return;
	}

	if (m_dialogLineIndex + 1 < m_dialogLines.size())
	{
		++m_dialogLineIndex;
		m_dialogVisibleChars = 0;
		m_dialogCharTimer = 0.f;
	}
	else
	{
		m_dialogOpen = false;
		m_dialogLineIndex = 0;
		m_dialogVisibleChars = 0;
		m_dialogCharTimer = 0.f;
		m_showTutorialPrompt = true;
	}
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
				if (key == sf::Keyboard::Key::E)
				{
					if (m_dialogOpen)
						advanceDialog();
					else if (m_showInteractPrompt)
						startDialog();
				}
				else if (key == sf::Keyboard::Key::J)
				{
					if (m_player.attack())
						playSwordSwing();

					m_showTutorialPrompt = false;
				}
				else if (key == sf::Keyboard::Key::P || key == sf::Keyboard::Key::Escape)
				{
					m_gameState = GameState::Paused;
				}
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
			else if (m_gameState == GameState::LevelComplete)
			{
				if (key == sf::Keyboard::Key::Enter)
				{
					const int nextLevel = m_currentLevelIndex + 1;
					if (nextLevel < static_cast<int>(m_levelPaths.size()))
					{
						m_currentLevelIndex = nextLevel;
						loadLevel(m_currentLevelIndex, false);
						m_gameState = GameState::Playing;
					}
					else
					{
						m_gameState = GameState::MainMenu;
						m_mainMenuSelection = MainMenuOption::Start;
					}
				}
			}
		}

		if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
		{
			if (mousePressed->button == sf::Mouse::Button::Left && m_gameState == GameState::LevelComplete)
			{
				const sf::Vector2f mousePos{
					static_cast<float>(mousePressed->position.x),
					static_cast<float>(mousePressed->position.y)
				};

				if (m_levelCompleteButtonBounds.contains(mousePos))
				{
					const int nextLevel = m_currentLevelIndex + 1;
					if (nextLevel < static_cast<int>(m_levelPaths.size()))
					{
						m_currentLevelIndex = nextLevel;
						loadLevel(m_currentLevelIndex, false);
						m_gameState = GameState::Playing;
					}
					else
					{
						m_gameState = GameState::MainMenu;
						m_mainMenuSelection = MainMenuOption::Start;
					}
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
	if (m_parryFreezeTimer > 0.f)
	{
		m_parryFreezeTimer = std::max(0.f, m_parryFreezeTimer - deltaTime);
		m_parryFlashTimer += deltaTime;
		drawParryFlash();
		return;
	}
	if (m_hitStopTimer > 0.f)
	{
		m_hitStopTimer = std::max(0.f, m_hitStopTimer - deltaTime);
		return;
	}

	int inputDir = 0;
	const bool isRunning = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift);

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) inputDir = -1;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) inputDir = 1;

	m_player.setHorizontalInput(inputDir, isRunning);
	m_player.applyGravity(deltaTime);
	m_player.update(deltaTime, m_worldPixelSize);
	m_player.resolveCollisions(m_platforms);

	const sf::FloatRect playerBounds = m_player.getGlobalBounds();
	const float voidY = static_cast<float>(m_worldPixelSize.y) + 60.f;
	if (playerBounds.position.y > voidY)
	{
		m_player.takeDamage(m_player.getMaxHealth());
		m_gameState = GameState::DeathMenu;
		return;
	}

	const float playerRight = playerBounds.position.x + playerBounds.size.x;
	if (playerRight >= static_cast<float>(m_worldPixelSize.x) - 1.f)
	{
		if (m_currentLevelIndex < 2)
		{
			if (!m_levelFinishPlayed && m_levelFinishMusic.getStatus() != sf::SoundSource::Status::Playing)
			{
				m_levelFinishMusic.play();
				m_levelFinishPlayed = true;
			}

			m_gameState = GameState::LevelComplete;
			return;
		}
	}

	m_enemy.update(deltaTime, m_player.getCenter());

	if (m_currentLevelIndex == 2 && m_enemy.isDead() && !m_bugattiTriggered && !m_bugattiPending)
	{
		m_bugattiPending = true;
		m_bugattiDelayTimer = 2.f;

		if (!m_endingMusicPlayed && m_endingMusic.getStatus() != sf::SoundSource::Status::Playing)
		{
			m_backgroundMusic.stop();
			m_endingMusic.play();
			m_endingMusicPlayed = true;
		}
	}

	if (m_bugattiPending)
	{
		m_bugattiDelayTimer -= deltaTime;
		if (m_bugattiDelayTimer <= 0.f && m_bugattiSprite)
		{
			m_bugattiPending = false;
			m_bugattiTriggered = true;
			m_bugattiFalling = true;
			m_bugattiSprite->setPosition({ m_player.getCenter().x - 59.f, playerBounds.position.y - 150.f });
		}
	}

	// Player -> Enemy hit
	if (!m_enemy.isDead() && m_player.consumeAttackHit())
	{
		const float dist = std::abs(m_player.getCenter().x - m_enemy.getCenter().x);
		if (dist <= 50.f)
		{
			const bool willDie = (m_enemy.getHealth() - 15) <= 0;
			m_enemy.takeDamage(15);
			m_hitStopTimer = m_hitStopDuration;

			if (willDie)
				playEnemyDeath();
			else
				playEnemyHurt();
		}
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

				if (m_parryFreezeTimer <= 0.f)
				{
					m_parryFreezeTimer = m_parryFreezeDuration;
					m_parryFlashTimer = 0.f;
					playParry();
				}
			}
			else
			{
				m_hitStopTimer = m_hitStopDuration;
				m_player.takeDamage(20);
			}
		}
	}

	if (m_bugattiFalling && m_bugattiSprite)
	{
		sf::FloatRect bounds = m_bugattiSprite->getGlobalBounds();
		const float nextY = bounds.position.y + (m_bugattiFallSpeed * deltaTime);

		float groundY = static_cast<float>(m_worldPixelSize.y);
		for (const auto& platform : m_platforms)
		{
			const float right = bounds.position.x + bounds.size.x;
			const float platformRight = platform.position.x + platform.size.x;

			if (right > platform.position.x && bounds.position.x < platformRight)
				groundY = std::min(groundY, platform.position.y);
		}

		if (nextY + bounds.size.y >= groundY)
		{
			m_bugattiSprite->setPosition({ bounds.position.x, groundY - bounds.size.y });
			m_bugattiFalling = false;
		}
		else
		{
			m_bugattiSprite->setPosition({ bounds.position.x, nextY });
		}
	}

	m_player.setBlocking(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::K));

	if (m_player.isDead())
	{
		m_gameState = GameState::DeathMenu;
		return;
	}

	updateSwordsmanInteraction(deltaTime);
	updateCamera(deltaTime);

	if (m_gameState == GameState::Playing && m_currentLevelIndex == 2 && m_bugattiTriggered && !m_bugattiFalling)
	{
		if (!m_showEndingCredits)
		{
			m_showEndingCredits = true;
			m_endingCreditsOffset = static_cast<float>(m_window.getSize().y);
		}

		updateEndingCredits(deltaTime);
	}
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

void Game::drawInteractPrompt()
{
	if (!m_swordsmanSprite)
		return;

	m_window.setView(m_view);

	const sf::FloatRect bounds = m_swordsmanSprite->getGlobalBounds();
	sf::Text prompt(m_uiFont);
	prompt.setString("Press E to Interact");
	prompt.setCharacterSize(10);
	prompt.setFillColor(sf::Color::White);
	const sf::FloatRect textBounds = prompt.getLocalBounds();
	prompt.setOrigin({ textBounds.position.x + textBounds.size.x * 0.5f, textBounds.position.y + textBounds.size.y });
	prompt.setPosition({ bounds.position.x + bounds.size.x * 0.5f, bounds.position.y - 6.f });

	const float padX = 5.f;
	const float padY = 3.f;
	sf::RectangleShape backdrop({
		textBounds.size.x + (padX * 2.f),
		textBounds.size.y + (padY * 2.f)
	});
	backdrop.setOrigin({ (textBounds.size.x * 0.5f) + padX, textBounds.size.y + padY });
	backdrop.setPosition(prompt.getPosition());
	backdrop.setFillColor(sf::Color(0, 0, 0, 180));

	m_window.draw(backdrop);
	m_window.draw(prompt);
	m_window.setView(m_view);
}

void Game::drawDialogBox()
{
	if (!m_dialogOpen || m_dialogLines.empty())
		return;

	m_window.setView(m_window.getDefaultView());

	const sf::Vector2u ws = m_window.getSize();
	const float margin = 16.f;
	const float boxHeight = 86.f;

	sf::RectangleShape box({
		static_cast<float>(ws.x) - (margin * 2.f),
		boxHeight
	});
	box.setPosition({ margin, static_cast<float>(ws.y) - boxHeight - margin });
	box.setFillColor(sf::Color(10, 10, 10, 210));
	box.setOutlineThickness(2.f);
	box.setOutlineColor(sf::Color::White);
	m_window.draw(box);

	const std::string& line = m_dialogLines[m_dialogLineIndex];
	const std::string visible = line.substr(0, std::min(m_dialogVisibleChars, line.size()));

	sf::Text dialogText(m_uiFont);
	dialogText.setString(visible);
	dialogText.setCharacterSize(14);
	dialogText.setFillColor(sf::Color::White);
	dialogText.setPosition({ margin + 12.f, static_cast<float>(ws.y) - boxHeight - margin + 12.f });
	m_window.draw(dialogText);

	m_window.setView(m_view);
}

void Game::drawMainMenu()
{
	m_window.setView(m_window.getDefaultView());

	const sf::Vector2u ws = m_window.getSize();
	const float centerX = static_cast<float>(ws.x) * 0.5f;
	const float baseY = static_cast<float>(ws.y) * 0.55f;

	sf::Text titleText(m_uiFont);
	titleText.setString("Get your Bugatti back!");
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

void Game::drawLevelComplete()
{
	m_window.setView(m_window.getDefaultView());

	const sf::Vector2u ws = m_window.getSize();
	sf::RectangleShape dim({ static_cast<float>(ws.x), static_cast<float>(ws.y) });
	dim.setFillColor(sf::Color(0, 0, 0, 170));
	m_window.draw(dim);

	const int levelNumber = m_currentLevelIndex + 1;
	const std::string title = "You've completed level " + std::to_string(levelNumber) + "!";
	const std::string buttonLabel =
		(levelNumber < static_cast<int>(m_levelPaths.size())) ? "Proceed to next level" : "Back to main menu";

	sf::Text titleText(m_uiFont);
	titleText.setString(title);
	titleText.setCharacterSize(22);
	titleText.setFillColor(sf::Color::White);
	const sf::FloatRect titleBounds = titleText.getLocalBounds();
	titleText.setOrigin({ titleBounds.position.x + titleBounds.size.x * 0.5f, titleBounds.position.y + titleBounds.size.y * 0.5f });
	titleText.setPosition({ static_cast<float>(ws.x) * 0.5f, static_cast<float>(ws.y) * 0.4f });
	m_window.draw(titleText);

	const float buttonW = 360.f;
	const float buttonH = 42.f;
	const float centerX = static_cast<float>(ws.x) * 0.5f;
	const float centerY = static_cast<float>(ws.y) * 0.58f;

	sf::RectangleShape button({ buttonW, buttonH });
	button.setOrigin({ buttonW * 0.5f, buttonH * 0.5f });
	button.setPosition({ centerX, centerY });
	button.setFillColor(sf::Color(30, 120, 60, 220));
	button.setOutlineThickness(2.f);
	button.setOutlineColor(sf::Color::White);
	m_window.draw(button);

	sf::Text buttonText(m_uiFont);
	buttonText.setString(buttonLabel);
	buttonText.setCharacterSize(16);
	buttonText.setFillColor(sf::Color::White);
	const sf::FloatRect buttonTextBounds = buttonText.getLocalBounds();
	buttonText.setOrigin({ buttonTextBounds.position.x + buttonTextBounds.size.x * 0.5f, buttonTextBounds.position.y + buttonTextBounds.size.y * 0.5f });
	buttonText.setPosition({ centerX, centerY });
	m_window.draw(buttonText);

	m_levelCompleteButtonBounds = sf::FloatRect(
		{ centerX - buttonW * 0.5f, centerY - buttonH * 0.5f },
		{ buttonW, buttonH }
	);

	m_window.setView(m_view);
}

bool Game::loadLevel(int levelIndex, bool reloadPlayer)
{
	if (levelIndex < 0 || levelIndex >= static_cast<int>(m_levelPaths.size()))
		return false;

	if (!m_world.loadFromTmx(m_levelPaths[levelIndex]))
	{
		std::cerr << "Failed to load TMX map: " << m_levelPaths[levelIndex] << "\n";
		return false;
	}

	m_worldPixelSize = m_world.getPixelSize();
	m_platforms = m_world.getColliders();

	m_bugattiTriggered = false;
	m_bugattiFalling = false;
	m_bugattiPending = false;
	m_bugattiDelayTimer = 0.f;
	m_endingMusicPlayed = false;

	m_dialogOpen = false;
	m_showInteractPrompt = false;
	m_showTutorialPrompt = false;
	m_dialogLineIndex = 0;
	m_dialogVisibleChars = 0;
	m_dialogCharTimer = 0.f;
	m_hitStopTimer = 0.f;
	m_parryFreezeTimer = 0.f;
	m_parryFlashTimer = 0.f;

	if (reloadPlayer)
	{
		m_player = Player();
		if (!m_player.load("Assets/playerSheet.png", m_baseResolution))
			std::cerr << "Failed to load player texture! fix the clanker\n";
	}

	const sf::Vector2f playerSpawn{ 20.f, 256.f };
	m_player.resetForLevel(playerSpawn);

	const float worldWidth = static_cast<float>(m_worldPixelSize.x);
	sf::Vector2f enemySpawn{ worldWidth - 40.f, 256.f };

	if (levelIndex == 0)
		enemySpawn = { 1076.f, 256.f };
	else if (levelIndex == 1)
		enemySpawn = { 1203.f, 256.f };

	m_enemy = Enemy();
	if (!m_enemy.load("Assets/enemySheet.png", enemySpawn))
		std::cerr << "Failed to load enemy sheet!\n";

	m_view.setCenter({ static_cast<float>(m_baseResolution.x) * 0.5f, static_cast<float>(m_baseResolution.y) * 0.5f });
	updateCamera(0.f);

	return true;
}

void Game::resetGame()
{
	m_currentLevelIndex = 0;
	loadLevel(m_currentLevelIndex, true);
	m_gameState = GameState::Playing;
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

	if (m_currentLevelIndex == 0 && m_swordsmanSprite)
		m_window.draw(*m_swordsmanSprite);

	if (m_currentLevelIndex == 2 && m_bugattiSprite && m_bugattiTriggered)
		m_window.draw(*m_bugattiSprite);

	m_enemy.draw(m_window);
	m_player.draw(m_window);
	m_world.drawFront(m_window);

	drawUI();

	if (m_gameState == GameState::Paused)
		drawOverlay("Paused", "P or Esc - Resume\nQ - Quit");
	else if (m_gameState == GameState::DeathMenu)
		drawOverlay("You Died", "R or Enter - Retry\nQ or Esc - Quit");
	else if (m_gameState == GameState::LevelComplete)
		drawLevelComplete();
	//else if (m_gameState == GameState::Playing && m_currentLevelIndex == 2 && m_bugattiTriggered && !m_bugattiFalling)
	//	drawOverlay("You've won BUGATTI!", "");

	if (m_gameState == GameState::Playing && m_showInteractPrompt)
		drawInteractPrompt();

	if (m_gameState == GameState::Playing && m_dialogOpen)
		drawDialogBox();

	if (m_gameState == GameState::Playing && m_showTutorialPrompt)
		drawTutorialPrompt();

	if (m_gameState == GameState::Playing && m_showEndingCredits)
		drawEndingCredits();

	m_window.display();
}

void Game::drawTutorialPrompt()
{
	if (!m_showTutorialPrompt)
		return;

	m_window.setView(m_window.getDefaultView());

	sf::Text text(m_uiFont);
	text.setString(m_tutorialText);
	text.setCharacterSize(12);
	text.setFillColor(sf::Color(230, 230, 230));

	const sf::FloatRect textBounds = text.getLocalBounds();
	const float pad = 8.f;
	const float margin = 12.f;
	const sf::Vector2u ws = m_window.getSize();

	const float boxX = margin;
	const float boxY = static_cast<float>(ws.y) - (textBounds.size.y + pad * 2.f) - margin;

	sf::RectangleShape box({
		textBounds.size.x + (pad * 2.f),
		textBounds.size.y + (pad * 2.f)
	});
	box.setPosition({ boxX, boxY });
	box.setFillColor(sf::Color(0, 0, 0, 200));
	box.setOutlineThickness(1.f);
	box.setOutlineColor(sf::Color(255, 255, 255, 120));

	text.setPosition({
		boxX + pad - textBounds.position.x,
		boxY + pad - textBounds.position.y
	});

	m_window.draw(box);
	m_window.draw(text);

	m_window.setView(m_view);
}

void Game::updateEndingCredits(float deltaTime)
{
	if (!m_showEndingCredits)
		return;

	m_endingCreditsOffset -= m_endingCreditsSpeed * deltaTime;
}

void Game::drawEndingCredits()
{
	if (!m_showEndingCredits)
		return;

	m_window.setView(m_window.getDefaultView());

	const sf::Vector2u ws = m_window.getSize();

	sf::RectangleShape dim({ static_cast<float>(ws.x), static_cast<float>(ws.y) });
	dim.setFillColor(sf::Color(0, 0, 0, 200));
	m_window.draw(dim);

	const float centerX = static_cast<float>(ws.x) * 0.5f;
	const float baseY = m_endingCreditsOffset;
	const float lineSpacing = 18.f;

	sf::Text lineText(m_uiFont);
	lineText.setCharacterSize(14);
	lineText.setFillColor(sf::Color(245, 245, 245));
	lineText.setOutlineThickness(1.f);
	lineText.setOutlineColor(sf::Color(0, 0, 0, 180));

	std::vector<std::string> lines;
	std::string current;
	for (char ch : m_endingCreditsText)
	{
		if (ch == '\n')
		{
			lines.push_back(current);
			current.clear();
		}
		else
		{
			current.push_back(ch);
		}
	}
	lines.push_back(current);

	float y = baseY;
	for (const auto& line : lines)
	{
		lineText.setString(line);
		const sf::FloatRect bounds = lineText.getLocalBounds();
		lineText.setOrigin({ bounds.position.x + bounds.size.x * 0.5f, 0.f });
		lineText.setPosition({ centerX, y });

		m_window.draw(lineText);
		y += lineSpacing;
	}

	m_window.setView(m_view);
}

void Game::playEnemyHurt()
{
	if (m_enemyHurtBuffers.empty())
		return;

	std::uniform_int_distribution<int> dist(0, static_cast<int>(m_enemyHurtBuffers.size() - 1));
	int index = dist(m_rng);

	if (m_enemyHurtBuffers.size() > 1)
	{
		while (index == m_lastEnemyHurtIndex)
			index = dist(m_rng);
	}

	m_lastEnemyHurtIndex = index;
	m_enemyHurtSound.emplace(m_enemyHurtBuffers[static_cast<std::size_t>(index)]);
	m_enemyHurtSound->play();
}

void Game::playEnemyDeath()
{
	if (m_enemyDeathBuffer.getSampleCount() == 0)
		return;

	m_enemyDeathSound.emplace(m_enemyDeathBuffer);
	m_enemyDeathSound->play();
}

void Game::playSwordSwing()
{
	if (m_swordSwingBuffer.getSampleCount() == 0)
		return;

	m_swordSwingSound.emplace(m_swordSwingBuffer);
	m_swordSwingSound->play();
}

void Game::drawParryFlash()
{
	if (m_parryFreezeTimer <= 0.f)
		return;

	m_window.setView(m_window.getDefaultView());

	const sf::Vector2u ws = m_window.getSize();
	sf::RectangleShape flash({ static_cast<float>(ws.x), static_cast<float>(ws.y) });

	const float pulse = 0.5f + (0.5f * std::sin(m_parryFlashTimer * m_parryFlashFrequency));
	const float clamped = std::max(0.f, std::min(1.f, pulse));
	const std::uint8_t alpha = static_cast<std::uint8_t>(clamped * 200.f);
	flash.setFillColor(sf::Color(255, 255, 255, alpha));

	m_window.draw(flash);
	m_window.setView(m_view);
}

void Game::playParry()
{
	if (m_parryBuffer.getSampleCount() == 0)
		return;

	m_parrySound.emplace(m_parryBuffer);
	m_parrySound->play();
}
