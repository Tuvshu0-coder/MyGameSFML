#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>



class Player
{
private:
	sf::Texture m_texture;
	sf::Sprite m_sprite;
	float m_speed = 5.0f;

	float m_velocityY = 0.0f;
	float m_gravity = 0.5f;
	float m_jumpStrength = -12.f;
	bool m_isGrounded = false;
	//bool m_isMovingX = false;
	
	enum class Direction
	{
		left,
		right
	};
	Direction m_direction = Direction::right;

public:
	Player() : m_sprite(m_texture) {}
	// initialize the texture and sprite
	bool load(const std::string& path) {
		if (!m_texture.loadFromFile(path)) {
			return false;
		}
		m_sprite.setTexture(m_texture, true);
		m_sprite.setPosition({ 300.f, 250.f });
		return true;
	}


	void handleInput() {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
			m_sprite.move({ -m_speed, 0.f });
			m_direction = Direction::left;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
		{
			m_sprite.move({ m_speed, 0.f });
			m_direction = Direction::right;
		}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && m_isGrounded)
		{
			m_velocityY = m_jumpStrength;
			m_isGrounded = false;
		}
			updateDirection();

	}

	void updateDirection() {
		if (m_direction == Direction::left) {
			m_sprite.setTextureRect(sf::IntRect({ 0, 0 }, { 80, 80 }));
		}
		else if (m_direction == Direction::right) {
			m_sprite.setTextureRect(sf::IntRect({ 80, 0 }, { 80, 80 }));
		}
	}

	void update(sf::Vector2u windowSize) {
		// apply gravity to Velocity
		m_velocityY += m_gravity;

		//Move the sprite by the velocity
		m_sprite.move({ 0.f, m_velocityY });

		sf::Vector2f pos = m_sprite.getPosition();
		sf::FloatRect bounds = m_sprite.getGlobalBounds();

		float winWidth = static_cast<float>(windowSize.x);
		float winHeight = static_cast<float>(windowSize.y);

		if (pos.y + bounds.size.y > winHeight) {
			pos.y = winHeight - bounds.size.y;
			m_velocityY = 0.0f;
			m_isGrounded = true;
		}
		else {
			m_isGrounded = false;
		}

		if (pos.x < 0.f) pos.x = 0.f;
		if (pos.x + bounds.size.x > winWidth) {
			pos.x = winWidth - bounds.size.x;
		}
		if (pos.y < 0.f) pos.y = 0.f;
		if (pos.y + bounds.size.y > winHeight) {
			pos.y = winHeight - bounds.size.y;
		}
		m_sprite.setPosition(pos);

	}

	void draw(sf::RenderWindow& window) {
		window.draw(m_sprite);
	}
};

int main() {
	
	sf::RenderWindow window(sf::VideoMode({700, 500}), "SFML is working wtf");
	window.setFramerateLimit(120);
	//sf::Vector2f windowSize = window.getSize();


	Player player;
	if (!player.load("Assets/player.png")) {
		std::cerr << "Failed to load sprite! You may fix this clanker" << std::endl;
		return -1;
	}

	sf::Clock clock;
	int frameCount = 0;

	while (window.isOpen()) {
		while (const std::optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>())
				window.close();
		}
		// Input A D handling
		player.handleInput();
		// Bound check
		player.update(window.getSize());
		
		//FPS counter
		frameCount++;
		if (clock.getElapsedTime().asSeconds() >= 1.0f) {
			std::cout << "FPS " << frameCount << std::endl;
			frameCount = 0;
			clock.restart();
		}
		//std::cout << "X: " << position.x << "Y: " << position.y << std::endl;

		window.clear();
		player.draw(window);
		window.display();
	}

}