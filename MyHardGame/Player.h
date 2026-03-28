#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Player
{
public:
    enum class Direction {Left, Right};

    Player();
    
    bool load(const std::string& path , sf::Vector2u windowSize);

    void move(float dx, float dy);
    void jump();
    void applyGravity(float deltaTime);
    void update(sf::Vector2u windowSize);
    void resolveCollisions(const std::vector<sf::FloatRect>& platforms);
    void setDirection(Direction dir);
    void draw(sf::RenderWindow& window);

    sf::FloatRect getGlobalBounds() const;
    float getSpeed() const { return m_speed; } //speed

private:
    void updateTextureRect();

    sf::Texture m_texture;
    sf::Sprite m_sprite;
    float m_speed = 500.f;
    float m_velocityY = 0.f;
    float m_gravity = 2000.f;
    float m_jumpStrength = -700.f;
    bool m_isGrounded = false;

    Direction m_direction = Direction::Right;
};
