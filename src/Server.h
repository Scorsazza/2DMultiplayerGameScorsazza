// Server.h
#ifndef SERVER_H
#define SERVER_H

#include <SFML/Network.hpp>
#include <vector>
#include <memory>
#include <mutex>
#include "Player.h"



class Server {
 public:
  Server();
  void init();
  void run();
  void handleClient(sf::TcpSocket* cSocket);
  void broadcastPlayerPositions(sf::TcpSocket* sender);
  void updatePlayerPosition(int playerId, sf::Vector2f position);
  void broadcastChatMessage(int senderId, const std::string& message);

 private:
  std::unique_ptr<sf::TcpListener> listener;
  std::vector<sf::TcpSocket*> connectedClients;
  std::mutex clientsMutex;
  std::vector<Player> players;
  int nextPlayerId;
};

#endif // SERVER_H