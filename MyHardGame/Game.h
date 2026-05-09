#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include <string>
#include "Player.h"
#include "World.h"
#include "Enemy.h"

class Game
{
public:
	Game();
	void run(); //main loop

private:
	enum class GameState { MainMenu, Playing, Paused, DeathMenu, LevelComplete };
	enum class MainMenuOption { Start, Quit };

	void processEvents();
	void update(float deltaTime);
	void render();
	void updateView(sf::Vector2u windowSize);
	void updateCamera(float deltaTime);
	void drawUI();
	void drawOverlay(const std::string& title, const std::string& body);
	void drawMainMenu();
	void drawLevelComplete();
	void resetGame();
	bool loadLevel(int levelIndex, bool reloadPlayer);

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

	sf::Texture m_mainMenuTexture;
	std::optional<sf::Sprite> m_mainMenuBackground;

	std::vector<sf::FloatRect> m_platforms;

	sf::Font m_uiFont;
	GameState m_gameState = GameState::MainMenu;
	MainMenuOption m_mainMenuSelection = MainMenuOption::Start;

	std::vector<std::string> m_levelPaths;
	int m_currentLevelIndex = 0;
	sf::FloatRect m_levelCompleteButtonBounds{ { 0.f, 0.f }, { 0.f, 0.f } };

	sf::Texture m_swordsmanTexture;
	std::optional<sf::Sprite> m_swordsmanSprite;

private:
	void updateBackgroundLayout(sf::Vector2u windowSize);
	void updateMainMenuLayout(sf::Vector2u windowSize);
};