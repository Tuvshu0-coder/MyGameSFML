#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class Enemy
{
public:
	enum class Direction { Left, Right };
	enum class State { Idle, Run, Attack, Hit, Death };

	Enemy();

	bool load(const std::string& texturePath, sf::Vector2f spawnFeetPosition);

	void update(float deltaTime, sf::Vector2f playerCenter);
	void draw(sf::RenderWindow& window);

	sf::FloatRect getBodyBounds() const;
	sf::Vector2f getCenter() const;

	bool consumeAttackHit(); // true once when attack reaches hit frame
	void takeDamage(int amount);

	bool isDead() const;
	int getHealth() const;
	int getMaxHealth() const;

	void stun(float seconds);
	bool isStunned() const;

private:
	void setState(State state);
	void updateAnimation(float deltaTime);

private:
	sf::Texture m_texture;
	sf::Sprite m_sprite;

	State m_state = State::Idle;
	Direction m_direction = Direction::Left;

	// Sheet/frame data
	sf::Vector2i m_frameSize{ 192, 70 };
	int m_originX = 60;
	int m_originY = 63;

	// enemySheet.txt:
	// Idle = Row1 (12), Run = Row2 (8), Slice and Dice = Row3 (14), hit = Row4 (2), Death = Row5 (15)
	// Convert to zero-based rows:
	int m_idleRow = 0;
	int m_runRow = 1;
	int m_attackRow = 2;
	int m_hitRow = 3;
	int m_deathRow = 4;

	int m_idleFrames = 12;
	int m_runFrames = 8;
	int m_attackFrames = 14;
	int m_hitFrames = 2;
	int m_deathFrames = 15;

	int m_currentFrame = 0;
	float m_animTimer = 0.f;

	float m_idleFrameDuration = 0.10f;
	float m_runFrameDuration = 0.10f;
	float m_attackFrameDuration = 0.08f;
	float m_hitFrameDuration = 0.08f;
	float m_deathFrameDuration = 0.08f;

	// Combat / movement
	float m_moveSpeed = 80.f;
	float m_attackRange = 35.f;
	bool m_isAttacking = false;
	bool m_attackHitAConsumed = false; // frame 6
	bool m_attackHitBConsumed = false; // frame 10

	// Player said attack hits on frame 6 and 10 (human counting)
	// So zero-based indexes are 5 and 9.
	int m_attackHitFrameA = 5;
	int m_attackHitFrameB = 9;

	// Hitbox from your data
	float m_bodyWidth = 15.f;
	float m_bodyHeight = 28.f;

	int m_health = 200;
	int m_maxHealth = 200;

	float m_stunTimer = 0.f;
	float m_defaultStunDuration = 0.6f;
};