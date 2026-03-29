#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Player
{
public:
    enum class Direction {Left, Right};
    enum class State {Idle, Walk, Run};

    Player();
    
    bool load(const std::string& path , sf::Vector2u windowSize);

    // Add the move declaration here
    void move(float dx, float dy);

    void jump();
    void applyGravity(float deltaTime);
    void update(float deltaTime, sf::Vector2u windowSize);
    void resolveCollisions(const std::vector<sf::FloatRect>& platforms);
    void setDirection(Direction dir);
    void setState(State state);
    void draw(sf::RenderWindow& window);

    // --- ADD THIS AND REMOVE m_speed STUFF ---
    void setHorizontalInput(int inputDir, bool isRunning);

    sf::FloatRect getGlobalBounds() const;

private:
    void updateAnimation(float deltaTime);
    float getScaleFactor() const;

    sf::Texture m_texture;
    sf::Sprite m_sprite;

    // --- INPUT / RUNTIME STATE ---
    float m_velocityX = 0.f;
    float m_velocityY = 0.f;
    int m_inputDir = 0;
    bool m_isRunning = false;
    bool m_isGrounded = false;

    // --- TUNING (edit these only) ---
    float m_scale = 1.f;                // visual/player size now
    float m_referenceScale = 2.f;       // old "feel" scale to preserve behavior

    float m_walkSpeed = 100.f;          // at reference scale
    float m_runSpeed = 200.f;           // at reference scale
    float m_airAcceleration = 600.f;    // at reference scale
    float m_gravity = 800.f;            // at reference scale
    float m_jumpStrength = -300.f;      // at reference scale

    Direction m_direction = Direction::Right;
    State m_state = State::Idle;

    int m_currentFrame = 0;
    float m_animationTimer = 0.f;
    float m_frameDuration = 0.15f;
    sf::Vector2i m_frameSize{ 90, 37 };
};
