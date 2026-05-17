#include "Player.h"
#include <iostream>

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

void Player::resetForLevel(sf::Vector2f position)
{
	m_sprite.setPosition(position);
	m_velocityX = 0.f;
	m_velocityY = 0.f;
	m_inputDir = 0;
	m_isRunning = false;
	m_isGrounded = false;
	m_isBlocking = false;
	m_isAttacking = false;
	m_queueNextAttack = false;
	m_attackHitConsumed = false;
	m_attackMissed = false;
	m_comboStep = 0;
	m_comboTimer = 0.f;
	setDirection(Direction::Right);
	setState(State::Idle);
}

float Player::getScaleFactor() const
{
	return m_scale / m_referenceScale;
}

void Player::jump()
{
	if (m_state == State::Death || m_state == State::Hit || isBlocking())
		return;

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

bool Player::attack()
{
	if (m_state == State::Death || m_state == State::Hit || isBlocking())
		return false;

	if (!m_isAttacking)
	{
		const int nextCombo = (m_comboStep <= 0) ? 1 : (m_comboStep < 4 ? m_comboStep + 1 : 1);
		startAttack(nextCombo);
		return true;
	}
	else if (m_comboStep < 4)
	{
		m_queueNextAttack = true;
	}

	return false;
}

void Player::startAttack(int comboIndex)
{
	m_isAttacking = true;
	m_queueNextAttack = false;
	m_comboStep = comboIndex;
	m_comboTimer = 0.f;
	m_attackMissed = false;

	if (comboIndex == 1) setState(State::Attack1);
	else if (comboIndex == 2) setState(State::Attack2);
	else if (comboIndex == 3) setState(State::Attack3);
	else setState(State::Attack4);
}

void Player::update(float deltaTime, sf::Vector2u windowSize)
{
	const float scaleFactor = getScaleFactor();
	const float currentMaxSpeed = (m_isRunning ? m_runSpeed : m_walkSpeed) * scaleFactor;

	if (!m_isAttacking && m_comboStep > 0)
	{
		m_comboTimer += deltaTime;
		if (m_comboTimer >= m_comboTimeout)
			m_comboStep = 0;
	}

	if (m_state == State::Death)
	{
		m_velocityX = 0.f;
		updateAnimation(deltaTime);
		return;
	}

	if (m_state == State::Hit)
	{
		m_velocityX = 0.f;
		updateAnimation(deltaTime);
		return;
	}

	// Always allow changing facing direction based on input (only when alive)
	if (m_inputDir == -1) setDirection(Direction::Left);
	if (m_inputDir == 1) setDirection(Direction::Right);

	// BLOCK STATE
	if (isBlocking() && m_isGrounded && !m_isAttacking)
	{
		m_velocityX = 0.f;
		setState(State::Block);
		updateAnimation(deltaTime);
		return;
	}

	if (!m_isAttacking)
	{
		if (m_isGrounded)
		{
			m_velocityX = static_cast<float>(m_inputDir) * currentMaxSpeed;

			if (m_inputDir != 0)
				setState(m_isRunning ? State::Run : State::Walk);
			else
				setState(State::Idle);
		}
		else
		{
			if (m_inputDir != 0)
				m_velocityX += static_cast<float>(m_inputDir) * (m_airAcceleration * scaleFactor) * deltaTime;

			if (m_velocityX > currentMaxSpeed) m_velocityX = currentMaxSpeed;
			if (m_velocityX < -currentMaxSpeed) m_velocityX = -currentMaxSpeed;

			setState(m_velocityY < 0.f ? State::JumpUp : State::Falling);
		}
	}
	else
	{
		if (m_isGrounded)
			m_velocityX = 0.f;
	}

	m_sprite.move({ m_velocityX * deltaTime, 0.f });

	sf::Vector2f pos = m_sprite.getPosition();
	float winWidth = static_cast<float>(windowSize.x);

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
	if (m_state == State::Death)
		return;

	m_direction = dir;
}

void Player::setState(State state)
{
	if (m_state != state)
	{
		m_state = state;
		m_currentFrame = 0;
		m_animationTimer = 0.f;

		if (m_state == State::Attack1 ||
			m_state == State::Attack2 ||
			m_state == State::Attack3 ||
			m_state == State::Attack4)
		{
			m_attackHitConsumed = false;
		}
	}
}

sf::Vector2f Player::getCenter() const
{
	const sf::FloatRect b = getGlobalBounds();
	return { b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f };
}

bool Player::consumeAttackHit()
{
	if (!m_isAttacking || m_attackHitConsumed)
		return false;

	int hitFrame = 2;
	if (m_state == State::Attack1) hitFrame = 2;
	else if (m_state == State::Attack2) hitFrame = 2;
	else if (m_state == State::Attack3) hitFrame = 2;
	else if (m_state == State::Attack4) hitFrame = 3;
	else return false;

	if (m_currentFrame >= hitFrame)
	{
		m_attackHitConsumed = true;
		return true;
	}

	return false;
}

bool Player::consumeAttackMissed()
{
	if (!m_attackMissed)
		return false;

	m_attackMissed = false;
	return true;
}

void Player::takeDamage(int amount)
{
	if (isDead())
		return;

	m_health = std::max(0, m_health - amount);

	m_isAttacking = false;
	m_queueNextAttack = false;
	m_velocityX = 0.f;

	if (m_health <= 0)
	{
		setState(State::Death);
	}
	else
	{
		setState(State::Hit);
	}
}

int Player::getHealth() const
{
	return m_health;
}

int Player::getMaxHealth() const
{
	return m_maxHealth;
}

bool Player::isDead() const
{
	return m_health <= 0;
}

void Player::updateAnimation(float deltaTime)
{
	m_animationTimer += deltaTime;

	int activeRow = 0;
	int totalFrames = 1;
	float currentFrameDuration = m_frameDuration;
	bool loop = true;

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
	else if (m_state == State::Run)
	{
		activeRow = 4;
		totalFrames = 8;
		currentFrameDuration = 0.08f;
	}
	else if (m_state == State::JumpUp)
	{
		activeRow = 13;
		totalFrames = 3;
		currentFrameDuration = 0.09f;
	}
	else if (m_state == State::Falling)
	{
		activeRow = 15;
		totalFrames = 3;
		currentFrameDuration = 0.09f;
	}
	else if (m_state == State::Block)
	{
		activeRow = 11; // Block row
		totalFrames = 6;
		currentFrameDuration = 0.09f;
	}
	else if (m_state == State::Attack1)
	{
		activeRow = 7;
		totalFrames = 7;
		currentFrameDuration = 0.06f;
		loop = false;
	}
	else if (m_state == State::Attack2)
	{
		activeRow = 8;
		totalFrames = 5;
		currentFrameDuration = 0.06f;
		loop = false;
	}
	else if (m_state == State::Attack3)
	{
		activeRow = 9;
		totalFrames = 5;
		currentFrameDuration = 0.07f;
		loop = false;
	}
	else if (m_state == State::Attack4)
	{
		activeRow = 10;
		totalFrames = 6;
		currentFrameDuration = 0.07f;
		loop = false;
	}
	else if (m_state == State::Hit)
	{
		activeRow = 25;
		totalFrames = 2;
		currentFrameDuration = 0.09f;
		loop = false;
	}
	else if (m_state == State::Death)
	{
		activeRow = 26;
		totalFrames = 6;
		currentFrameDuration = 0.10f;
		loop = false;
	}

	bool finished = false;
	while (m_animationTimer >= currentFrameDuration)
	{
		m_animationTimer -= currentFrameDuration;

		if (loop)
		{
			m_currentFrame = (m_currentFrame + 1) % totalFrames;
		}
		else
		{
			if (m_currentFrame < totalFrames - 1)
				++m_currentFrame;
			else
			{
				finished = true;
				break;
			}
		}
	}

	if (finished)
	{
		const bool isAttackState =
			(m_state == State::Attack1 || m_state == State::Attack2 ||
			 m_state == State::Attack3 || m_state == State::Attack4);

		if (isAttackState)
		{
			if (!m_attackHitConsumed && !m_queueNextAttack)
				m_attackMissed = true;

			if (m_queueNextAttack && m_comboStep < 4)
				startAttack(m_comboStep + 1);
			else
			{
				m_isAttacking = false;
				m_queueNextAttack = false;
				m_comboTimer = 0.f;

				if (m_inputDir != 0)
					setState(m_isRunning ? State::Run : State::Walk);
				else
					setState(State::Idle);
			}

			updateAnimation(0.f);
			return;
		}
		else if (m_state == State::Hit)
		{
			if (m_inputDir != 0)
				setState(m_isRunning ? State::Run : State::Walk);
			else
				setState(State::Idle);

			updateAnimation(0.f);
			return;
		}
		else if (m_state == State::Death)
		{
			m_currentFrame = totalFrames - 1;
		}
	}

	const int leftOffset = m_currentFrame * m_frameSize.x;
	const int topOffset = activeRow * m_frameSize.y;
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

void Player::setBlocking(bool blocking)
{
	m_isBlocking = blocking;
}

bool Player::isBlocking() const
{
	return m_isBlocking && !isDead() && m_state != State::Hit;
}



