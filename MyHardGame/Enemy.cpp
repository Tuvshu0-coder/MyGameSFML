#include "Enemy.h"
#include <algorithm>
#include <cmath>

Enemy::Enemy() : m_sprite(m_texture)
{
}

bool Enemy::load(const std::string& texturePath, sf::Vector2f spawnFeetPosition)
{
	if (!m_texture.loadFromFile(texturePath))
		return false;

	m_texture.setSmooth(false);
	m_sprite.setTexture(m_texture, true);
	m_sprite.setOrigin({ static_cast<float>(m_originX), static_cast<float>(m_originY) });
	m_sprite.setPosition(spawnFeetPosition);

	setState(State::Idle);
	updateAnimation(0.f);
	return true;
}

void Enemy::setState(State state)
{
	if (m_state == state)
		return;

	m_state = state;
	m_currentFrame = 0;
	m_animTimer = 0.f;

	if (m_state == State::Attack)
	{
		m_isAttacking = true;
		m_attackHitAConsumed = false;
		m_attackHitBConsumed = false;
	}
	else
	{
		// IMPORTANT: clear attack lock when leaving attack
		m_isAttacking = false;
	}
}

void Enemy::update(float deltaTime, sf::Vector2f playerCenter)
{
	if (isDead())
	{
		setState(State::Death);
		updateAnimation(deltaTime);
		return;
	}

	const sf::Vector2f center = getCenter();
	const float dx = playerCenter.x - center.x;
	const float distance = std::abs(dx);

	if (dx < 0.f) m_direction = Direction::Left;
	if (dx > 0.f) m_direction = Direction::Right;

	if (!m_isAttacking)
	{
		if (distance <= m_attackRange)
		{
			setState(State::Attack);
		}
		else
		{
			setState(State::Run);
			const float dir = (dx < 0.f) ? -1.f : 1.f;
			m_sprite.move({ dir * m_moveSpeed * deltaTime, 0.f });
		}
	}

	updateAnimation(deltaTime);
}

void Enemy::updateAnimation(float deltaTime)
{
	m_animTimer += deltaTime;

	int row = m_idleRow;
	int totalFrames = m_idleFrames;
	float frameDuration = m_idleFrameDuration;
	bool loop = true;

	if (m_state == State::Run)
	{
		row = m_runRow;
		totalFrames = m_runFrames;
		frameDuration = m_runFrameDuration;
	}
	else if (m_state == State::Attack)
	{
		row = m_attackRow;
		totalFrames = m_attackFrames;
		frameDuration = m_attackFrameDuration;
		loop = false;
	}
	else if (m_state == State::Hit)
	{
		row = m_hitRow;
		totalFrames = m_hitFrames;
		frameDuration = m_hitFrameDuration;
		loop = false;
	}
	else if (m_state == State::Death)
	{
		row = m_deathRow;
		totalFrames = m_deathFrames;
		frameDuration = m_deathFrameDuration;
		loop = false;
	}

	bool finished = false;
	while (m_animTimer >= frameDuration)
	{
		m_animTimer -= frameDuration;

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
		if (m_state == State::Attack)
		{
			m_isAttacking = false;
			setState(State::Idle);
		}
		else if (m_state == State::Hit)
		{
			setState(State::Idle);
		}
	}

	const int left = m_currentFrame * m_frameSize.x;
	const int top = row * m_frameSize.y;
	m_sprite.setTextureRect(sf::IntRect({ left, top }, { m_frameSize.x, m_frameSize.y }));

	if (m_direction == Direction::Left)
		m_sprite.setScale({ -1.f, 1.f });
	else
		m_sprite.setScale({ 1.f, 1.f });
}

bool Enemy::consumeAttackHit()
{
	if (m_state != State::Attack || !m_isAttacking)
		return false;

	if (!m_attackHitAConsumed && m_currentFrame >= m_attackHitFrameA)
	{
		m_attackHitAConsumed = true;
		return true;
	}

	if (!m_attackHitBConsumed && m_currentFrame >= m_attackHitFrameB)
	{
		m_attackHitBConsumed = true;
		return true;
	}

	return false;
}

void Enemy::takeDamage(int amount)
{
	if (isDead())
		return;

	m_health = std::max(0, m_health - amount);

	if (m_health <= 0)
	{
		setState(State::Death);
	}
	else
	{
		setState(State::Hit);
	}
}

bool Enemy::isDead() const
{
	return m_health <= 0;
}

int Enemy::getHealth() const
{
	return m_health;
}

int Enemy::getMaxHealth() const
{
	return m_maxHealth;
}

sf::FloatRect Enemy::getBodyBounds() const
{
	const sf::Vector2f p = m_sprite.getPosition(); // feet anchor
	return sf::FloatRect(
		{ p.x - (m_bodyWidth * 0.5f), p.y - m_bodyHeight },
		{ m_bodyWidth, m_bodyHeight });
}

sf::Vector2f Enemy::getCenter() const
{
	const sf::FloatRect b = getBodyBounds();
	return { b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f };
}

void Enemy::draw(sf::RenderWindow& window)
{
	window.draw(m_sprite);
}