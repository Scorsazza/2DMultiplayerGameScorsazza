#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <vector>
#include <string>
#include "tinyxml2.h"

class Player {
 public:
  Player(int id, const sf::Vector2f& startPosition);
  void handleInput(const sf::RenderWindow& window);
  void update(float deltaTime);
  void draw(sf::RenderWindow& window, const sf::Font& font) const;
  void addChatMessage(const std::string& message, float duration);
  void updateChat(float deltaTime);
  std::vector<std::string> getChatMessages() const;
  sf::Vector2f getPosition() const;
  void setPosition(const sf::Vector2f& newPosition);
  int getId() const;
  void handleCollision(const sf::Vector2f& tilePosition, const sf::Vector2f& tileSize);
  sf::Vector2f getSpriteSize() const;

 private:
  struct ChatMessage {
    std::string message;
    float displayTime;
  };

  struct Frame {
    sf::IntRect rect;
    std::string name;
  };

  int id;
  sf::Vector2f position;
  sf::Vector2f velocity;
  float speed;
  std::vector<ChatMessage> chatMessages;
  sf::Texture playerTexture;
  sf::Sprite playerSprite;
  std::vector<Frame> frames;
  float frameTime;
  int currentFrame;
  float animationSpeed;

  void loadFramesFromXML(const std::string& xmlFile);
};

#endif // PLAYER_H
