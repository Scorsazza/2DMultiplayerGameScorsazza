#ifndef TILE_H
#define TILE_H
#include <SFML/Graphics.hpp>
#include <memory>

class Tile
{
 public:
  Tile();
  Tile(const int& ID, const sf::Texture& texture);
  ~Tile() = default;

  std::unique_ptr<sf::Sprite>& GetSprite();

  float GetID() const;

 private:
  float tileID = 0;
  std::unique_ptr<sf::Sprite> tileSprite;
};


#endif // TILE_H