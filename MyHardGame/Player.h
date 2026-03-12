#pragma once
#include <SFML/Graphics.hpp>

class Player
{
public:
    enum class Direction {Left, Right};

    Player();
    
    bool load(const std::string& path);

    void move(float dx, float dy);
    void jump();
    void applyGravity();
    void update(sf::Vector2u windowSize);
    void setDirection(Direction dir);
    void draw(sf::RenderWindow& window);

    sf::FloatRect getGlobalBounds() const;
    float getSpeed() const { return m_speed; } //speed

private:
    void updateTextureRect();

    sf::Texture m_texture;
    sf::Sprite m_sprite;
    float m_speed = 200.f;
    float m_velocityY = 0.f;
    float m_gravity = 0.5f;
    float m_jumpStrength = -12.f;
    bool m_isGrounded = false;

    Direction m_direction = Direction::Right;

};
