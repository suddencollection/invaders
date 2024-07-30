#pragma once

#include "collisionBuffer.hpp"
#include "entity.hpp"
#include "sprite.hpp"

#include <curses.h>
#include <list>
#include <memory>
#include <optional>

class Program
{
  using Path = std::filesystem::path;

public:
  Program(Path sprite_path);
  ~Program();
  void run();

private:
  void initCurses();
  void createWindows();
  void loadSprites(Path path);
  void createEntities();
  void endCurses();

  // void loadArena(Path sprites_path);
  // void drawSprite(WINDOW* win, Entity& entity);
  void drawSprite(std::vector<chtype>& buffer, Entity& entity);
  void render(float frameDuration);
  bool updateFramebuffer();
  void logic(int input, float ts, bool& force);

  auto spawnEntity(YX<float> pos, YX<float> vel, int health, std::shared_ptr<Sprite>& sprite) -> Entity::ID;
  void paintBorders();

private:
  // Windows
  WINDOW* m_arenaWin;
  WINDOW* m_arenaBorderWin;

  // Sprites
  struct
  {
    std::shared_ptr<Sprite> ship;
    std::shared_ptr<Sprite> shipBullet;
    std::vector<std::shared_ptr<Sprite>> aliens{};
    std::shared_ptr<Sprite> alienBullet;
  } m_sprites;

  // Entities
  std::unordered_map<Entity::ID, Entity> m_entities;
  struct
  {
    Entity::ID ship{};
    std::list<Entity::ID> aliens{};
    std::list<Entity::ID> bullets{};
  } m_entityIDs;

  // CollisionBuffer
  CollisionBuffer m_collisionBuffer{m_entities};

  // Miscellaneous
  YX<int> m_arenaSize{32, 64};
  YX<int> m_alienFormation{5, 8};
  YX<float> m_alienPosOffset;
  YX<float> m_alienVelocity{0, 1};
  YX<int> m_alienStartingPoint{3, 9};
  bool m_debugMode = false;
  std::vector<chtype> m_framebuffer{};

  enum class GameState
  {
    idle,
    running,
    won,
    lose,
    quitted,
  } m_gameState{GameState::running};
};
