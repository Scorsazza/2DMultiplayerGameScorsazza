#include "Player.h"
#include <iostream>

Player::Player(int id, const sf::Vector2f& startPosition)
  : id(id), position(startPosition), speed(100.0f), frameTime(0.0f), currentFrame(0), animationSpeed(0.2f) {

  loadFramesFromXML("Data/Images/Spritesheet.xml");

  if (!playerTexture.loadFromFile("Data/Images/Spritesheet.png")) {
    std::cerr << "Failed to load player sprites." << std::endl;
  }

  playerSprite.setTexture(playerTexture);
  if (!frames.empty()) {
    playerSprite.setTextureRect(frames[0].rect);
  }
  playerSprite.setPosition(position);
  playerSprite.setScale(0.5f, 0.5f);
}

void Player::loadFramesFromXML(const std::string& xmlFile) {
  tinyxml2::XMLDocument doc;
  if (doc.LoadFile(xmlFile.c_str()) != tinyxml2::XML_SUCCESS) {
    std::cerr << "Failed to load XML file: " << xmlFile << std::endl;
    return;
  }

  tinyxml2::XMLElement* root = doc.FirstChildElement("TextureAtlas");
  for (tinyxml2::XMLElement* elem = root->FirstChildElement("SubTexture"); elem; elem = elem->NextSiblingElement("SubTexture")) {
    std::string name = elem->Attribute("name");

    if (name.find("walk") != std::string::npos) {
      Frame frame;
      frame.rect = sf::IntRect(
        elem->IntAttribute("x"),
        elem->IntAttribute("y"),
        elem->IntAttribute("width"),
        elem->IntAttribute("height")
      );
      frames.push_back(frame);
    }
  }
}
void Player::handleInput(const sf::RenderWindow& window) {
  velocity = sf::Vector2f(0.0f, 0.0f);

  if (window.hasFocus()) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
      velocity.y -= speed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
      velocity.y += speed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
      velocity.x -= speed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
      velocity.x += speed;
    }
  }
}

sf::Vector2f Player::getSpriteSize() const
{
  return sf::Vector2f(
    playerSprite.getGlobalBounds().width,
    playerSprite.getGlobalBounds().height);
}
void Player::update(float deltaTime) {
  position += velocity * deltaTime;
  playerSprite.setPosition(position);

  if (velocity.x != 0.0f || velocity.y != 0.0f) {
    frameTime += deltaTime;
    if (frameTime >= animationSpeed) {
      frameTime = 0.f;
      currentFrame = (currentFrame + 1) % frames.size();
      playerSprite.setTextureRect(frames[currentFrame].rect);
    }
  } else {

    currentFrame = 0;
    playerSprite.setTextureRect(frames[currentFrame].rect);
  }

  updateChat(deltaTime);
}

void Player::handleCollision(const sf::Vector2f& tilePosition, const sf::Vector2f& tileSize) {

  sf::Vector2f playerSize = sf::Vector2f(playerSprite.getGlobalBounds().width, playerSprite.getGlobalBounds().height);

  sf::Vector2f playerCenter = position + sf::Vector2f(playerSize.x / 2, playerSize.y / 2);
  sf::Vector2f tileCenter = tilePosition + sf::Vector2f(tileSize.x / 2, tileSize.y / 2);

  float deltaX = playerCenter.x - tileCenter.x;
  float deltaY = playerCenter.y - tileCenter.y;

  if (std::abs(deltaX) > std::abs(deltaY)) {
    if (deltaX > 0) {

      position.x = tilePosition.x + tileSize.x;
    } else {

      position.x = tilePosition.x - playerSize.x;
    }
  } else {
    if (deltaY > 0) {

      position.y = tilePosition.y + tileSize.y;
    } else {

      position.y = tilePosition.y - playerSize.y;
    }
  }
}

void Player::draw(sf::RenderWindow& window, const sf::Font& font) const {
  window.draw(playerSprite);

  float yOffset = -30;
  for (const auto& chatMessage : chatMessages) {
    sf::Text text(chatMessage.message, font, 24);
    text.setPosition(position.x, position.y + yOffset);
    text.setFillColor(sf::Color::White);
    window.draw(text);
    yOffset -= 30;
  }
}

void Player::addChatMessage(const std::string& message, float duration) {
  chatMessages.push_back({message, duration});
}

void Player::updateChat(float deltaTime) {
  for (auto it = chatMessages.begin(); it != chatMessages.end();) {
    it->displayTime -= deltaTime;
    if (it->displayTime <= 0) {
      it = chatMessages.erase(it);
    } else {
      ++it;
    }
  }
}

sf::Vector2f Player::getPosition() const {
  return position;
}

void Player::setPosition(const sf::Vector2f& newPosition) {
  position = newPosition;
}

int Player::getId() const {
  return id;
}