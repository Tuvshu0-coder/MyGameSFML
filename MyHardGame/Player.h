#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Player
{
public:
    enum class Direction {Left, Right};
    enum class State {Idle, Walk};

    Player();
    
    bool load(const std::string& path , sf::Vector2u windowSize);

    void move(float dx, float dy);
    void jump();
    void applyGravity(float deltaTime);
    void update(float deltaTime, sf::Vector2u windowSize);
    void resolveCollisions(const std::vector<sf::FloatRect>& platforms);
    void setDirection(Direction dir);
    void setState(State state);
    void draw(sf::RenderWindow& window);

    sf::FloatRect getGlobalBounds() const;
    float getSpeed() const { return m_speed; } //speed

private:
    void updateAnimation( float deltaTime);

    sf::Texture m_texture;
    sf::Sprite m_sprite;
    float m_speed = 100.f;
    float m_velocityY = 0.f;
    float m_gravity = 800.f;
    float m_jumpStrength = -300.f;
    bool m_isGrounded = false;

    Direction m_direction = Direction::Right;

    // -- Animation variables --
    State m_state = State::Idle;

    int m_currentFrame = 0;                 // Which frame of the animation we are on
    float m_animationTimer = 0.f;           //Track time passed
    float m_frameDuration = 0.15f;          // How long each frame lasts (0.15 seconds)

    //The width and height of one single GRID CELL in the sprite sheet
    sf::Vector2i m_frameSize{ 90, 37 };
};
