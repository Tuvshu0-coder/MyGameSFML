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
	sf::FloatRect bounds = m_sprite.getGlobalBounds();
	float winWidth = static_cast<float>(windowSize.x);
	float winHeight = static_cast<float>(windowSize.y);

	//floor and ceiling
	if (pos.y > winHeight) {
		pos.y = winHeight;
		m_velocityY = 0.f;
		m_isGrounded = true;
	}
	else m_isGrounded = false;

	if (pos.x < 0.f) pos.x = 0.f;
	if (pos.x + bounds.size.x > winWidth) pos.x = winWidth - bounds.size.x;

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
	/*if (m_direction == Direction::Left)
		m_sprite.setTextureRect(sf::IntRect({ 0, 0 }, { 80, 80 }));
	else
		m_sprite.setTextureRect(sf::IntRect({ 80, 0 }, { 80, 80 }));*/

	// 1. Advance the timer
	m_animationTimer += deltaTime;

	// 2. Setup variables for which row on the image to read, and how many frames it has
	int activeRow = 0;
	int totalFrames = 1;

	// Set row and frame cound based on the current state.
	// Check your sprute to confirm if Idle is Row 0 and Walk is Row 1.
	// Static Idle = Row0  (1)
	// Idle        = Row1  (9)
	// Walk		   = Row2  (8)
	// Run		   = Row3  (8)
	// Fast Run    = Row4  (8)
	// Roll		   = Row5  (8)
	// Roll attack = Row6  (10)
	// Slash 1     = Row7  (7)
	// Slash 2     = Row8  (5)
	// Slam		   = Row9  (5)
	// Spin Attack = Row10 (6)
	// Block	   = Row11 (6)
	// Dash		   = Row12 (6)
	// JumpUp	   = Row13 (3)
	// JumpMid	   = Row14 (4)
	// Falling	   = Row15 (3)
	// Land		   = Row19 (4)
	// Ledge grab  = Row20 (5)
	// Wall Hold   = Row21 (1)
	// WallTrans   = Row22 (2)
	// WallSLide   = Row23 (3)
	// WallStop    = Row24 (3)
	// Get hit     = Row25 (2)
	// Death	   = Row26 (6)

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

	// 3. Switch frame if enough time has passed
	if (m_animationTimer >= m_frameDuration)
	{
		// Subtract frame if enough time has passed
		m_animationTimer -= m_frameDuration;

		//Move to next frame, and use modulo to loop back to 0 when it hist totalFrames
		m_currentFrame = (m_currentFrame + 1) % totalFrames;
	}
	// 4. Calculate the bounding box for this specific frame
	// X position is (current frame * 90), Y position is (current row * 37)
	int leftOffset = m_currentFrame * m_frameSize.x;
	int topOffset = activeRow * m_frameSize.y;

	m_sprite.setTextureRect(sf::IntRect({ leftOffset, topOffset }, { m_frameSize.x, m_frameSize.y }));

	// 5. Flip the sprite based on direction
	// Because out origin is perfectly positioned underneath him, scaling X by -1 flips him in place!

	if (m_direction == Direction::Left)
		m_sprite.setScale({ -1.f, 1.f });
	else
		m_sprite.setScale({ 1.f, 1.f });
	

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


