#include "Client.h"
#include <iostream>
#include <thread>

// Constructor with message queue reference
Client::Client(std::list<std::pair<std::string, std::chrono::steady_clock::time_point>>& mq)
  : messageQueue(mq), socket(std::make_unique<sf::TcpSocket>()) {
  socket->setBlocking(false);
}

void Client::connect() {
  socket->setBlocking(true);
  std::cout << "Attempting to connect to server..." << std::endl;

  sf::Socket::Status status = socket->connect("127.0.0.1", 53000);
  if (status == sf::Socket::Done) {
    std::cout << "Connected to server\n";
    connected = true;

    sf::Packet idPacket;
    if (socket->receive(idPacket) == sf::Socket::Done) {
      if (idPacket >> localPlayerId) {
        std::cout << "Assigned Player ID: " << localPlayerId << std::endl;
        localPlayer = std::make_unique<Player>(localPlayerId, sf::Vector2f(100.0f, 100.0f));
      } else {
        std::cerr << "Failed to receive player ID from server." << std::endl;
        connected = false;
      }
    } else {
      std::cerr << "Error receiving data from server." << std::endl;
      connected = false;
    }
  } else {
    std::cerr << "Error connecting to server: " << status << std::endl;
    connected = false;
  }
  socket->setBlocking(false);
}

void Client::input() {
  while (running) {
    if (connected) {
      sendPosition();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
}

void Client::runThread() {
  while (running && connected) {
    receivePositions();
    receiveChatMessages();
  }
}

void Client::run() {
  running = true;
  std::thread inputThread(&Client::input, this);
  std::thread clientRunThread(&Client::runThread, this);
  inputThread.detach(); // Detach the thread since it will run in the background
  clientRunThread.detach();
}

void Client::sendPosition() {
  sf::Vector2f playerPosition = localPlayer->getPosition();
  sf::Packet packet;

  packet << localPlayerId << playerPosition.x << playerPosition.y;

  if (socket->send(packet) != sf::Socket::Done) {
    std::cerr << "Failed to send player position to server" << std::endl;
  }
}

void Client::receivePositions() {
  sf::Packet packet;
  while (socket->receive(packet) == sf::Socket::Done) {
    int playerId;
    sf::Vector2f receivedPosition;
    if (packet >> playerId >> receivedPosition.x >> receivedPosition.y) {
      playerPositions[playerId] = receivedPosition; // Update the position
    }
  }
}

void Client::update() {
  if (localPlayer) {
    localPlayer->handleInput(window);
    localPlayer->update(0.0f);
  }
}

void Client::render(sf::RenderWindow& window) {
  if (localPlayer) {
    localPlayer->draw(window, font);
  }

  // Render other players using updated positions
  for (const auto& player : playerPositions) {
    int playerId = player.first;
    sf::Vector2f position = player.second;

    Player* remotePlayer = createOrUpdateRemotePlayer(playerId, position);
    remotePlayer->draw(window, font);
  }
}

void Client::clearReceivedMessages() {
  playerChatMessages.clear();
}

void Client::displayChatMessage(int senderId, const std::string& message) {
  std::string formattedMessage = (senderId == localPlayerId) ? "You: " : "Player " + std::to_string(senderId) + ": ";
  formattedMessage += message;

  messageQueue.emplace_back(formattedMessage, std::chrono::steady_clock::now());
}

Player* Client::createOrUpdateRemotePlayer(int playerId, const sf::Vector2f& position) {
  auto it = std::find_if(players.begin(), players.end(),
                         [playerId](const Player& p) { return p.getId() == playerId; });

  if (it != players.end()) {
    it->setPosition(position);
  } else {
    Player newPlayer(playerId, position);
    players.push_back(newPlayer);
  }

  return &players.back();
}

void Client::receiveChatMessages() {
  sf::Packet packet;
  while (socket->receive(packet) == sf::Socket::Done) {
    int packetType;
    packet >> packetType;

    if (packetType == 1) { // Chat message type
      int senderId;
      std::string message;
      if (packet >> senderId >> message) {
        displayChatMessage(senderId, message);
      }
    }
  }
}

void Client::sendChatMessage(const std::string& message) {
  sf::Packet packet;
  packet << 1 << localPlayerId << message;

  if (socket->send(packet) != sf::Socket::Done) {
    std::cerr << "[Error] Failed to send chat message." << std::endl;
  }
}
