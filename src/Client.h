#ifndef CLIENT_H
#define CLIENT_H

#include <SFML/Network.hpp>
#include <memory>
#include <vector>
#include "Player.h" // Include Player header
#include <unordered_map>
#include <queue>
#include <list>
#include <string>
#include <utility>
#include <chrono>


class Client {
  std::list<std::pair<std::string, std::chrono::steady_clock::time_point>>& messageQueue;
 public:
  sf::Font font;

  Client(std::list<std::pair<std::string, std::chrono::steady_clock::time_point>>& mq);
  std::unique_ptr<Player>& getLocalPlayer() { return localPlayer; }
  bool windowFocused = true;
  void connect();
  void addMessageToRenderQueue(const std::string& message);
  sf::TcpSocket& getClientSocket();
  std::queue<std::string> renderQueue;
  void input();
  void runThread();
  void run();
  void sendPosition();
  void receivePositions();
  void update();
  void receiveChatMessages();

  Player* createOrUpdateRemotePlayer(int playerId, const sf::Vector2f& position);

  void render(sf::RenderWindow& window);
  const std::vector<Player>& getPlayers() const;


  std::string currentTextInput;
  void sendChatMessage(const std::string& message);
  void networkActivity();
  sf::RenderWindow window;
  const std::unordered_map<int, sf::Vector2f>& getPlayerPositions() const { return playerPositions; }
  const std::unordered_map<int, std::vector<std::string>>& getPlayerChatMessages() const {
    return playerChatMessages;
  }
  void clearReceivedMessages();
  void displayChatMessage(int senderId, const std::string& message);
  void updatePlayerPosition(int playerId, sf::Vector2f position);
  void attemptReconnect();
  bool connectToServer();

  void sendPlayerPosition(const sf::Vector2f& position);
  std::vector<Player> receiveUpdates();
  void sendUpdate();

 private:
  std::unique_ptr<sf::TcpSocket> socket;
  bool running;
  bool connected;
  int localPlayerId;
  std::unique_ptr<Player> localPlayer;
  bool isServer;
  std::vector<Client> clients;
  std::unique_ptr<Client> client;
  std::vector<Player> players;
  std::unordered_map<int, sf::Vector2f> playerPositions;
  std::unordered_map<int, std::vector<std::string>> playerChatMessages;
};

#endif // CLIENT_H