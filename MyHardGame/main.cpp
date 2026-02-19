#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>

int main() {

	sf::RenderWindow window(sf::VideoMode({700, 500}), "SFML works!");
	window.setFramerateLimit(120);
	
	sf::RectangleShape player( sf::Vector2f(100.f, 100.f));
	player.setFillColor(sf::Color::Green);
	player.setPosition(sf::Vector2f(350.f, 250.f));

	sf::Clock clock;
	int frameCount = 0;

	while (window.isOpen()) {
		while (const std::optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>())
				window.close();
		}

		frameCount++;

		if (clock.getElapsedTime().asSeconds() >= 1.0f) {
			std::cout << "FPS " << frameCount << std::endl;
			frameCount = 0;
			clock.restart();
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
			player.move({ -0.5f, 0.f });

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
			player.move({ 0.f, -0.5f });

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
			player.move({ 0.5f, 0.f });

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
			player.move({ 0.f, 0.5f });


		window.clear();
		window.draw(player);
		window.display();
	}

}