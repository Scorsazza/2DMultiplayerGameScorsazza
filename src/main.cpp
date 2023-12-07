#include <iostream>
#include <SFML/Graphics.hpp>
#include "Game.h"

int main()
{
  std::cout << "You should see a window that opens as well as this writing to console..."
            << std::endl;

  sf::RenderWindow window(sf::VideoMode(1920, 1080), "SFML Base Game");
  window.setFramerateLimit(60);

  bool x = *std::getenv("doServer") == '1';
  Game game(window, x );

  if (!game.init())
  {
    return 0;
  }

  sf::Clock clock;

  while (window.isOpen())
  {

    sf::Event event;

    game.handleEvents();
    game.update(0.0f);
    game.render();

    sf::Time time = clock.restart();
    float dt = time.asSeconds();

    while (window.pollEvent(event))
    {

      if (event.type == sf::Event::Closed)
        window.close();

      if (event.type == sf::Event::KeyPressed)
        game.keyPressed(event);
    }

    game.update(dt);

    window.clear(sf::Color::White);

    game.render();
    window.display();
  }

  return 0;
}