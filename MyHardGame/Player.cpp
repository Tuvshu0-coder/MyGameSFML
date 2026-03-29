#include "Player.h"

Player::Player() : m_sprite(m_texture) {} // wut

bool Player::load(const std::string& path, sf::Vector2u windowSize)
{
	if (!m_texture.loadFromFile(path))
		return false;

	// Disable smoothing to keep my pixel art sharp
	m_texture.setSmooth(false);
	m_sprite.setTexture(m_texture, true);

	//The X and Y between player's feet
	m_sprite.setOrigin({ 27.f, 33.f });

	m_sprite.setPosition({ static_cast<float>(windowSize.x) * 0.5f, static_cast<float>(windowSize.y) * 0.5f });

	// I will update this function later to handle my new sprite sheet logic
	updateAnimation(0.0f);
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

void Player::applyGravity(float deltaTime) {
	m_velocityY += m_gravity * deltaTime;
	m_sprite.move({ 0.f, m_velocityY * deltaTime });
}

void Player::update(float deltaTime, sf::Vector2u windowSize)
{
	sf::Vector2f pos = m_sprite.getPosition();
	//sf::FloatRect bounds = m_sprite.getGlobalBounds();
	float winWidth = static_cast<float>(windowSize.x);
	float winHeight = static_cast<float>(windowSize.y);

	//floor and ceiling
	if (pos.y > winHeight) {
		pos.y = winHeight;
		m_velocityY = 0.f;
		m_isGrounded = true;
	}
	else m_isGrounded = false;

	//Calculate a realistic collision width for the player's body
	// Multiply by m_scale so the hitbox grows with the sprite!
	const float bodyHalfWidth = 5.f * m_scale;

	if (pos.x - bodyHalfWidth < 0.f) pos.x = bodyHalfWidth;
	if (pos.x + bodyHalfWidth > winWidth) pos.x = winWidth - bodyHalfWidth;
	

	m_sprite.setPosition(pos);
	
	updateAnimation(deltaTime); 

}

void Player::setDirection(Direction dir)
{
	m_direction = dir;
}

void Player::setState(State state)
{
	if (m_state != state) 
	{
		m_state = state;
		m_currentFrame = 0;
		m_animationTimer = 0.f;
	}

}

void Player::updateAnimation( float deltaTime)
{
	m_animationTimer += deltaTime;

	int activeRow = 0;
	int totalFrames = 1;

	// Reset frame duration to default every frame. We will override it if we are running.
	float currentFrameDuration = m_frameDuration;

	if (m_state == State::Idle) 
	{
		activeRow = 1;
		totalFrames = 9;
	}
	else if (m_state == State::Walk)
	{
		activeRow = 2;
		totalFrames = 8;
	}
	else if (m_state == State::Run) // ADD THIS
	{
		activeRow = 4;       // Row 3 based on your notes
		totalFrames = 8;     // 8 frames based on your notes
		currentFrameDuration = 0.08f; // Make the run animation play almost twice as fast!
	}

	if (m_animationTimer >= currentFrameDuration)
	{
		m_animationTimer -= currentFrameDuration;
		m_currentFrame = (m_currentFrame + 1) % totalFrames;
	}

	int leftOffset = m_currentFrame * m_frameSize.x;
	int topOffset = activeRow * m_frameSize.y;

	m_sprite.setTextureRect(sf::IntRect({ leftOffset, topOffset }, { m_frameSize.x, m_frameSize.y }));

	if (m_direction == Direction::Left)
		m_sprite.setScale({ -m_scale, m_scale }); 
	else
		m_sprite.setScale({ m_scale, m_scale });  
}

void Player::draw(sf::RenderWindow& window)
{
	window.draw(m_sprite);
}

sf::FloatRect  Player::getGlobalBounds() const
{
	return m_sprite.getGlobalBounds();
}

void Player::resolveCollisions(const std::vector<sf::FloatRect>& platforms)
{
	m_isGrounded = false;
	auto bounds = m_sprite.getGlobalBounds();

	for (const auto& platform : platforms)
	{
		if (const auto intersection = bounds.findIntersection(platform))
		{
			const float platformTop = platform.position.y;
			
			// Because the origin is at his feet, m_sprite.getPosition().y IS his feet line.
			// Compare his feet to the platform top.
			if (m_velocityY >= 0.f && m_sprite.getPosition().y - intersection->size.y <= platformTop)
			{
				m_sprite.setPosition({ m_sprite.getPosition().x, platformTop});
				m_velocityY = 0.f;
				m_isGrounded = true;
				
			}
		}
	}
}


