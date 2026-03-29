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

float Player::getScaleFactor() const
{
	return m_scale / m_referenceScale;
}

void Player::jump()
{
	if (m_isGrounded)
	{
		m_velocityY = m_jumpStrength * getScaleFactor();
		m_isGrounded = false;
	}
}

void Player::applyGravity(float deltaTime)
{
	m_velocityY += (m_gravity * getScaleFactor()) * deltaTime;
	m_sprite.move({ 0.f, m_velocityY * deltaTime });
}

void Player::update(float deltaTime, sf::Vector2u windowSize)
{
	const float scaleFactor = getScaleFactor();
	const float currentMaxSpeed = (m_isRunning ? m_runSpeed : m_walkSpeed) * scaleFactor;

	// Always allow changing facing direction based on input
	if (m_inputDir == -1) setDirection(Direction::Left);
	if (m_inputDir == 1) setDirection(Direction::Right);

	if (m_isGrounded)
	{
		// ON GROUND: Tight control. Instantly match target speed.
		m_velocityX = static_cast<float>(m_inputDir) * currentMaxSpeed;

		// Handle animation states cleanly
		if (m_inputDir != 0)
			setState(m_isRunning ? State::Run : State::Walk);
		else
			setState(State::Idle);
	}
	else
	{
		// IN AIR: Air strafing!
		// Allow them to move, but cap it at the max speed so they don't go flying too fast.
		if (m_inputDir != 0)
		{
			// Give them control in the air...
			m_velocityX += static_cast<float>(m_inputDir) * (m_airAcceleration * scaleFactor) * deltaTime;
		}

		// Clamp the velocity so they never exceed currentMaxSpeed mid-air!
		if (m_velocityX > currentMaxSpeed) m_velocityX = currentMaxSpeed;
		if (m_velocityX < -currentMaxSpeed) m_velocityX = -currentMaxSpeed;
	}

	// Actually move the sprite horizontally based on momentum!
	m_sprite.move({ m_velocityX * deltaTime, 0.f });

	// 2. --- EXISTING SCREEN BOUNDS LOGIC ---
	sf::Vector2f pos = m_sprite.getPosition();
	float winWidth = static_cast<float>(windowSize.x);
	float winHeight = static_cast<float>(windowSize.y);

	// floor and ceiling bounds check
	if (pos.y > winHeight) {
		pos.y = winHeight;
		m_velocityY = 0.f;
		m_isGrounded = true;
	}

	// Because of origin, bounds checking uses body width
	const float bodyHalfWidth = 5.f * m_scale;
	if (pos.x - bodyHalfWidth < 0.f)
	{
		pos.x = bodyHalfWidth;
		m_velocityX = 0.f;		
	}
	if (pos.x + bodyHalfWidth > winWidth)
	{
		pos.x = winWidth - bodyHalfWidth;
		m_velocityX = 0.f;		
	}
	
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

	const float bodyHalfWidth = 5.f * m_scale;   // keep consistent with your movement bounds
	const float bodyHeight = 28.f * m_scale;     // tune if needed (try 24..32)

	auto getBodyBounds = [&]() -> sf::FloatRect
	{
		const sf::Vector2f p = m_sprite.getPosition(); // feet-ish anchor
		return sf::FloatRect(
			{ p.x - bodyHalfWidth, p.y - bodyHeight },
			{ bodyHalfWidth * 2.f, bodyHeight }
		);
	};

	sf::FloatRect bounds = getBodyBounds();

	for (const auto& platform : platforms)
	{
		const auto intersection = bounds.findIntersection(platform);
		if (!intersection)
			continue;

		const float overlapX = intersection->size.x;
		const float overlapY = intersection->size.y;

		// Prefer vertical resolution for platformer feel
		if (overlapY <= overlapX)
		{
			const float boundsBottom = bounds.position.y + bounds.size.y;
			const float platformTop = platform.position.y;

			// Landing from above
			if (m_velocityY >= 0.f && boundsBottom - overlapY <= platformTop + 1.f)
			{
				m_sprite.move({ 0.f, -overlapY });
				m_velocityY = 0.f;
				m_isGrounded = true;
			}
			else
			{
				// Ceiling hit / fallback
				m_sprite.move({ 0.f, overlapY });
				if (m_velocityY < 0.f)
					m_velocityY = 0.f;
			}
		}
		else
		{
			// Wall collision
			if (bounds.position.x < platform.position.x)
				m_sprite.move({ -overlapX, 0.f });
			else
				m_sprite.move({ overlapX, 0.f });

			m_velocityX = 0.f;
		}

		bounds = getBodyBounds();
	}
}


void Player::setHorizontalInput(int dir, bool running)
{
	m_inputDir = dir;
	m_isRunning = running;
}


