#include "Player.h"

Player::Player() : m_sprite(m_texture) {} // wut

bool Player::load(const std::string& path)
{
	if (!m_texture.loadFromFile(path))
		return false;
	m_sprite.setTexture(m_texture, true);
	m_sprite.setPosition({ 300.f, 250.f });
	updateTextureRect(); //what
	return true;
}

void Player::move(float dx, float dy)
{
	m_sprite.move({ dx, dy });
}

void Player::jump()
{
	if (m_isGrounded)
	{
		m_velocityY = m_jumpStrength;
		m_isGrounded = false;
	}
}

void Player::update(sf::Vector2u windowSize)
{
	sf::Vector2f pos = m_sprite.getPosition();
	sf::FloatRect bounds = m_sprite.getGlobalBounds();
	float winWidth = static_cast<float>(windowSize.x);
	float winHeight = static_cast<float>(windowSize.y);

	//floor and ceiling
	if (pos.y + bounds.size.y > winHeight) {
		pos.y = winHeight - bounds.size.y;
		m_velocityY = 0.f;
		m_isGrounded = true;
	}
	else m_isGrounded = false;

	if (pos.x < 0.f) pos.x = 0.f;
	if (pos.x + bounds.size.x > winWidth) pos.x = winWidth - bounds.size.x;

	m_sprite.setPosition(pos);
	updateTextureRect();

}

void Player::setDirection(Direction dir)
{
	m_direction = dir;
}

void Player::updateTextureRect()
{
	if (m_direction == Direction::Left)
		m_sprite.setTextureRect(sf::IntRect({ 0, 0 }, { 80, 80 }));
	else
		m_sprite.setTextureRect(sf::IntRect({ 80, 0 }, { 80, 80 }));

}

void Player::draw(sf::RenderWindow& window)
{
	window.draw(m_sprite);
}

sf::FloatRect  Player::getGlobalBounds() const
{
	return m_sprite.getGlobalBounds();
}


