#include "Game.h"
#include <chrono>
#include <tmxlite/map.hpp>
#include <math.h>

Game::Game(sf::RenderWindow& game_window, bool server)
  : window(game_window), isServer(server), isTextBoxActive(false),
  player(1, sf::Vector2f(100.0f, 100.0f)), windowFocused(true) {
  srand(time(NULL));

  textBox.setSize(sf::Vector2f(400, 50));
  textBox.setFillColor(sf::Color(0, 0, 255, 150));
  textBox.setPosition(10, window.getSize().y - 60);
  chatOutputBox.setSize(sf::Vector2f(400, 150));
  chatOutputBox.setFillColor(sf::Color(0, 0, 255, 150));
  chatOutputBox.setPosition(10, window.getSize().y - 210);

  if (!font.loadFromFile("Data/Fonts/OpenSans-Bold.ttf")) {
    std::cerr << "Failed to load font." << std::endl;
  }

  textDisplay.setFont(font);
  textDisplay.setCharacterSize(24);
  textDisplay.setFillColor(sf::Color::White);
  textDisplay.setPosition(textBox.getPosition() + sf::Vector2f(5, 5));
}

Game::~Game() {

}

void Game::SetTileWithID(
  const unsigned int MAP_COLUMNS, const unsigned int MAP_ROWS,
  const tmx::Vector2u& tile_size, const tmx::TileLayer::Tile& tile)
{
  float scaleFactor = 3.5f;
  auto& current = *TILE_MAP.back().emplace_back(
    std::make_unique<Tile>(tile.ID, *tileMap));

  int tileID = tile.ID - 1;

  if (tileID < 0) {
    current.GetSprite()->setTextureRect(sf::IntRect(0, 0, 0, 0));
  } else {
    int tilesPerRow = tileMap->getSize().x / tile_size.x;
    sf::IntRect textureRect(
      (tileID % tilesPerRow) * tile_size.x,
      (tileID / tilesPerRow) * tile_size.y,
      tile_size.x,
      tile_size.y
    );
    current.GetSprite()->setTextureRect(textureRect);
  }

  int tileIndex = static_cast<int>(TILE_MAP.back().size() - 1);
  sf::Vector2f position(
    (tileIndex % MAP_COLUMNS) * tile_size.x * scaleFactor,
    (tileIndex / MAP_COLUMNS) * tile_size.y * scaleFactor
  );
  current.GetSprite()->setPosition(position);
  current.GetSprite()->setScale(scaleFactor, scaleFactor);
}

bool Game::init() {
  if (!tileMap->loadFromFile("Data/Map/tilemap.png")) {
    std::cout << "Failed to Load Spritesheet" << std::endl;
    return false;
  }

  tmx::Map map;
  if (!map.load("Data/Map/Map.tmx")) {
    std::cout << "Failed to Load Map Data" << std::endl;
    return false;
  }

  const unsigned int MAP_COLUMNS = map.getTileCount().x;
  const unsigned int MAP_ROWS = map.getTileCount().y;
  auto& tile_size = map.getTileSize();

  sf::Vector2u windowSize = window.getSize();
  float xScale = static_cast<float>(windowSize.x) / (MAP_COLUMNS * tile_size.x);
  float yScale = static_cast<float>(windowSize.y) / (MAP_ROWS * tile_size.y);
  float scaleFactor = std::min(xScale, yScale);

  TILE_MAP.reserve(map.getLayers().size());

  for (const auto& layer: map.getLayers()) {
    TILE_MAP.emplace_back(std::vector<std::unique_ptr<Tile>>());
    const auto& tiles = layer->getLayerAs<tmx::TileLayer>().getTiles();
    TILE_MAP.back().reserve(tiles.size());

    for (const auto& tile : tiles) {
      SetTileWithID(MAP_COLUMNS, MAP_ROWS, tile_size, tile);
    }
  }

  if (isServer) {
    server = std::make_unique<Server>();
    server->init();
    server->run();
  } else {
    client = std::make_unique<Client>(messageQueue);
    client->connect();
    client->run();
    client->update();
  }
  return true;
}

void Game::formatChatMessage(int playerId, const std::string& message) {
  std::string formattedMessage = (playerId == player.getId()) ? "You: " + message : "Player " + std::to_string(playerId) + ": " + message;
  messageQueue.emplace_back(formattedMessage, std::chrono::steady_clock::now());
}

bool Game::checkCollisionWithTile(const sf::Vector2f& playerPosition, const sf::Vector2f& playerSize, const sf::Vector2f& tileSize, const sf::Vector2f& tilePosition) {
  sf::FloatRect playerRect(playerPosition.x, playerPosition.y, playerSize.x, playerSize.y);
  sf::FloatRect tileRect(tilePosition.x, tilePosition.y, tileSize.x, tileSize.y);

  return playerRect.intersects(tileRect);
}

void Game::update(float dt) {
  if (windowFocused) {
    player.handleInput(window);
  }

  sf::Vector2f playerSize = player.getSpriteSize();

  bool collisionDetected = false;
  for (const auto& layer : TILE_MAP) {
    for (const auto& tile : layer) {
      if (tile->GetID() == 2) {
        sf::Vector2f tilePosition = tile->GetSprite()->getPosition();
        sf::Vector2f tileSize(tileMap->getSize().x, tileMap->getSize().y);

        if (checkCollisionWithTile(player.getPosition(), playerSize, tileSize, tilePosition)) {
          std::cout << "Collision Detected with Tile ID 2" << std::endl;
          collisionDetected = true;
          player.handleCollision(tilePosition, tileSize);
          break;
        }
      }
    }
    if (collisionDetected) break;
  }

  if (!collisionDetected) {
    player.update(dt);
  }

  if (client) {
    const auto& otherPlayerPositions = client->getPlayerPositions();
    for (const auto& [playerId, position] : otherPlayerPositions) {
      updatePlayerPosition(playerId, position);
    }

    const auto& newMessages = client->getPlayerChatMessages();
    for (const auto& [playerId, messages] : newMessages) {
      for (const auto& message : messages) {
        if (playerId != client->getLocalPlayer()->getId()) {
          formatChatMessage(playerId, message);
        }
      }
    }
    client->clearReceivedMessages();
  }

  for (auto& p : players) {
    p.update(dt);
  }
}

void Game::render() {
  window.clear();

  for (const auto& layer : TILE_MAP) {
    for (const auto& tile : layer) {
      if (tile->GetID() != 0) {
        window.draw(*tile->GetSprite());
      }
    }
  }

  player.draw(window, font);

  for (const auto& p : players) {
    p.draw(window, font);
  }

  auto currentTime = std::chrono::steady_clock::now();
  sf::Vector2f chatPos(10, window.getSize().y - 200);
  for (auto it = messageQueue.begin(); it != messageQueue.end();) {
    const auto& [message, timestamp] = *it;
    if (currentTime - timestamp > std::chrono::seconds(5)) {
      it = messageQueue.erase(it);
    } else {
      sf::Text chatText(message, font, 20);
      chatText.setPosition(chatPos);
      chatText.setFillColor(sf::Color::White);
      window.draw(chatText);
      chatPos.y -= 25;
      ++it;
    }
  }

  textBox.setFillColor(isTextBoxActive ? sf::Color(0, 0, 255, 200) : sf::Color(0, 0, 255, 150));
  window.draw(textBox);
  textDisplay.setString(chatInput);
  window.draw(textDisplay);

  window.display();
}

void Game::updatePlayerPosition(int playerId, sf::Vector2f newPosition) {
  if (playerId == player.getId()) {
    return;
  }

  auto it = std::find_if(players.begin(), players.end(),
                         [playerId](const Player& p) { return p.getId() == playerId; });
  if (it != players.end()) {
    it->setPosition(newPosition);
  } else {
    Player newPlayer(playerId, newPosition);
    players.push_back(newPlayer);
  }
}

void Game::handleEvents() {
  sf::Event event;
  while (window.pollEvent(event)) {
    if (event.type == sf::Event::Closed) {
      window.close();
    }

    if (event.type == sf::Event::GainedFocus) {
      windowFocused = true;
    }

    if (event.type == sf::Event::LostFocus) {
      windowFocused = false;
    }

    if (event.type == sf::Event::MouseButtonPressed) {
      if (event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        if (textBox.getGlobalBounds().contains(mousePos)) {
          isTextBoxActive = true;
        } else {
          isTextBoxActive = false;
        }
      }
    }

    if (event.type == sf::Event::TextEntered && isTextBoxActive) {
      if (event.text.unicode < 128) {
        char typedChar = static_cast<char>(event.text.unicode);
        if (typedChar == '\r' or typedChar == '\n') {
          if (!chatInput.empty()) {
            if (!isServer) {
              client->sendChatMessage(chatInput);
            }
            formatChatMessage(player.getId(), chatInput);
            chatInput.clear();
          }
        } else if (typedChar == '\b' && !chatInput.empty()) {
          chatInput.pop_back();
        } else if (typedChar >= 32) {
          chatInput += typedChar;
        }
      }
    }
  }
}

void Game::keyPressed(sf::Event event) {
  if (windowFocused) {
    player.handleInput(window);
  }
}