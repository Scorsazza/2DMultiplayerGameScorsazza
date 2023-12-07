#ifndef GAME_H
#define GAME_H

#include "Client.h" // Include the necessary header for the client
#include "Player.h" // Include the Player class
#include "Server.h" // Include the necessary header for the server
#include "Tile.h"
#include <SFML/Graphics.hpp>
#include <chrono>
#include <iostream>
#include <list>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/Types.hpp>
#include <utility>
#include <memory>
class Game
{
 public:

  Game(sf::RenderWindow& game_window, bool server);
  ~Game();
  void handleInput(sf::TcpSocket& socket);
  bool checkCollisionWithTile(const sf::Vector2f& playerPosition, const sf::Vector2f& playerSize, const sf::Vector2f& tileSize, const sf::Vector2f& tilePosition);

  bool init();
  bool windowFocused = true;
  void SetTileWithID(const unsigned int MAP_COLUMNS, const unsigned int MAP_ROWS, const tmx::Vector2<unsigned int> &tile_size,
                     const tmx::TileLayer::Tile &tile);
  void update(float dt);
  void render();
  void mouseClicked(sf::Event event);
  void updatePlayerPosition(int playerId, sf::Vector2f newPosition);
  void handleEvents();
  void keyPressed(sf::Event event);

  void formatChatMessage(int playerId, const std::string& message);
  std::list<std::pair<std::string, std::chrono::steady_clock::time_point>> messageQueue;

  void setWindowFocused(bool focused) {


    windowFocused = focused;
  }

  bool isWindowFocused() const {
    return windowFocused;
  }

  std::vector<Player> players;

 private:
  sf::RenderWindow& window;
  bool isTextBoxActive;
  std::vector<std::string> chatLog;
  sf::RectangleShape textBox;
  sf::Text textDisplay;
  sf::RectangleShape chatOutputBox;
  bool isServer;
  sf::Font font;
  std::string chatInput;
  std::unique_ptr<sf::Texture> tileMap = std::make_unique<sf::Texture>();
  std::vector<std::vector<std::unique_ptr<Tile>>> TILE_MAP;
  std::unique_ptr<Client> client;
  std::unique_ptr<Server> server;
  std::vector<Client> clients;

  std::vector<sf::Text> chatMessagesDisplay;

  Player player;  // Include the Player object in your Game class
};

#endif // GAME_H
