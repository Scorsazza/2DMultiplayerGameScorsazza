#include "Server.h"
#include <iostream>
#include <thread>
#include <algorithm>

Server::Server() : listener(std::make_unique<sf::TcpListener>()), nextPlayerId(1) {

}

void Server::updatePlayerPosition(int playerId, sf::Vector2f newPosition) {
  for (auto& player : players) {
    if (player.getId() == playerId) {
      player.setPosition(newPosition);
      break;
    }
  }
  broadcastPlayerPositions(nullptr);
}

void Server::init() {
  if (listener->listen(53000) != sf::Socket::Done) {
    std::cerr << "Error listening on port 53000\n";
  } else {
    std::cout << "Listening on port 53000\n";
  }
}

void Server::run() {
  while (true) {
    sf::TcpSocket* cSock = new sf::TcpSocket();
    if (listener->accept(*cSock) == sf::Socket::Done) {
      std::cout << "Client connected, assigning ID: " << nextPlayerId << std::endl;

      sf::Packet idPacket;
      idPacket << nextPlayerId;
      if (cSock->send(idPacket) != sf::Socket::Done) {
        std::cerr << "Failed to send player ID to client." << std::endl;
        delete cSock;
        continue;
      }

      connectedClients.push_back(cSock);
      sf::Vector2f startingPosition(100.0f, 100.0f);
      Player newPlayer(nextPlayerId++, startingPosition);
      players.push_back(newPlayer);

      std::thread clientThread(&Server::handleClient, this, cSock);
      clientThread.detach();
    } else {
      delete cSock;
    }
  }
}

void Server::handleClient(sf::TcpSocket* cSocket) {
  while (true) {
    sf::Packet packet;
    if (cSocket->receive(packet) == sf::Socket::Done) {
      int packetType;
      packet >> packetType;

      if (packetType == 0) {
        int receivedPlayerId;
        sf::Vector2f position;
        packet >> receivedPlayerId >> position.x >> position.y;
        updatePlayerPosition(receivedPlayerId, position);
        broadcastPlayerPositions(cSocket);
      } else if (packetType == 1) {
        int senderId;
        std::string message;
        packet >> senderId >> message;

        if (!message.empty()) {
          std::cout << "Received chat message from Player ID " << senderId << ": " << message << std::endl;
        }

        broadcastChatMessage(senderId, message);
      }
    } else {
      std::cerr << "Client disconnected or encountered an error." << std::endl;
      auto it = std::find(connectedClients.begin(), connectedClients.end(), cSocket);
      if (it != connectedClients.end()) {
        connectedClients.erase(it);
      }

      break;
    }
  }
}

void Server::broadcastPlayerPositions(sf::TcpSocket* sender) {
  for (const auto& player : players) {
    sf::Packet packet;
    packet << 0 << player.getId() << player.getPosition().x << player.getPosition().y;

    for (auto& client : connectedClients) {
      if (client != sender) {
        if (client->send(packet) != sf::Socket::Done) {
          std::cerr << "Failed to broadcast player positions to a client." << std::endl;
        }
      }
    }
  }
}

void Server::broadcastChatMessage(int senderId, const std::string& message) {
  if (!message.empty()) {
    sf::Packet chatPacket;
    chatPacket << 1 << senderId << message;

    std::cout << "Broadcasting message from Player ID " << senderId << ": " << message << std::endl;

    for (auto& client : connectedClients) {
      if (client->send(chatPacket) != sf::Socket::Done) {
        std::cerr << "Failed to send chat message to client connected at address: " << client->getRemoteAddress() << std::endl;
      }
    }
  }
}