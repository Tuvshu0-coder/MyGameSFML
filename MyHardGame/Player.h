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
    void updateAnimation( float deltaTime);

    sf::Texture m_texture;
    sf::Sprite m_sprite;
    
    // --- NEW PHYSICS VARIABLES ---
    float m_velocityX = 0.f;     // Tracks actual horizontal momentum
    int m_inputDir = 0;          // -1 for Left, 1 for Right, 0 for None
    bool m_isRunning = false;    // Are we holding shift?

    float m_velocityY = 0.f;
    float m_gravity = 800.f;
    float m_jumpStrength = -300.f;
    bool m_isGrounded = false;
    float m_scale = 2.f; 

    Direction m_direction = Direction::Right;
    State m_state = State::Idle;

    int m_currentFrame = 0;                 
    float m_animationTimer = 0.f;           
    float m_frameDuration = 0.15f;          
    sf::Vector2i m_frameSize{ 90, 37 };
};
