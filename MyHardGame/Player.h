#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Player
{
public:
    enum class Direction { Left, Right };
    enum class State { Idle, Walk, Run, JumpUp, Falling, Block, Attack1, Attack2, Attack3, Attack4, Hit, Death };

    Player();

    bool load(const std::string& path, sf::Vector2u windowSize);

    void move(float dx, float dy);

    void jump();
    void attack();
    void applyGravity(float deltaTime);
    void update(float deltaTime, sf::Vector2u windowSize);
    void resolveCollisions(const std::vector<sf::FloatRect>& platforms);
    void setDirection(Direction dir);
    void setState(State state);
    void draw(sf::RenderWindow& window);
    void setHorizontalInput(int inputDir, bool isRunning);

    void setBlocking(bool blocking);
    bool isBlocking() const;

    sf::FloatRect getGlobalBounds() const;
    sf::Vector2f getCenter() const;

    bool consumeAttackHit();
    void takeDamage(int amount);
    int getHealth() const;
    int getMaxHealth() const;
    bool isDead() const;

private:
    void updateAnimation(float deltaTime);
    float getScaleFactor() const;
    void startAttack(int comboIndex);

    sf::Texture m_texture;
    sf::Sprite m_sprite;

    float m_velocityX = 0.f;
    float m_velocityY = 0.f;
    int m_inputDir = 0;
    bool m_isRunning = false;
    bool m_isGrounded = false;
    bool m_isBlocking = false;

    bool m_isAttacking = false;
    bool m_queueNextAttack = false;
    bool m_attackHitConsumed = false;
    int m_comboStep = 0;
    float m_comboTimer = 0.f;
    float m_comboTimeout = 0.35f;

    int m_health = 350;
    int m_maxHealth = 350;

    float m_scale = 1.f;
    float m_referenceScale = 2.f;

    float m_walkSpeed = 100.f;
    float m_runSpeed = 200.f;
    float m_airAcceleration = 600.f;
    float m_gravity = 800.f;
    float m_jumpStrength = -300.f;

    Direction m_direction = Direction::Right;
    State m_state = State::Idle;

    int m_currentFrame = 0;
    float m_animationTimer = 0.f;
    float m_frameDuration = 0.3f;
    sf::Vector2i m_frameSize{ 90, 37 };
};
