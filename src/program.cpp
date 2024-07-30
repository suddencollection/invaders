#include "program.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <thread>

Program::Program(std::filesystem::path sprites_path)
{
  initCurses();
  createWindows();
  loadSprites(sprites_path);
  createEntities();
}

Program::~Program()
{
  endCurses();
}

void Program::initCurses()
{
  initscr();
  noecho();
  cbreak();
  nodelay(stdscr, true);
  start_color();
  use_default_colors();
  curs_set(0);

  init_pair(0, COLOR_BLACK, -1);
  init_pair(1, COLOR_RED, -1);
  init_pair(2, COLOR_GREEN, -1);
  init_pair(3, COLOR_YELLOW, -1);
  init_pair(4, COLOR_BLUE, -1);
  init_pair(5, COLOR_MAGENTA, -1);
  init_pair(6, COLOR_CYAN, -1);
  init_pair(7, COLOR_WHITE, -1);
}

void Program::createWindows()
{
  m_arenaWin = newwin(m_arenaSize.y,
    m_arenaSize.x,
    (LINES - m_arenaSize.y) / 2,
    (COLS - m_arenaSize.x) / 2);
  m_arenaBorderWin = newwin(m_arenaSize.y + 2,
    m_arenaSize.x + 2,
    (LINES - m_arenaSize.y) / 2 - 1,
    (COLS - m_arenaSize.x) / 2 - 1);

  m_framebuffer.resize(getmaxx(m_arenaWin) * getmaxy(m_arenaWin));
}

void Program::loadSprites(Path path)
{
  m_sprites.ship = std::make_shared<Sprite>(path / "ship");
  m_sprites.shipBullet = std::make_shared<Sprite>(path / "shipBullet");

  for(int line{}; line < m_alienFormation.y; ++line) {
    m_sprites.aliens.push_back(std::make_shared<Sprite>(path / ("alien" + std::to_string(line))));
  }
}

void Program::createEntities()
{
  // ship
  YX<float> pos{
    .y = static_cast<float>(getmaxy(m_arenaWin) - m_sprites.ship->size().y - 2),
    .x = static_cast<float>((getmaxx(m_arenaWin) - m_sprites.ship->size().x) / 2.f),
  };
  int health = 8;
  YX<float> vel = {0, 0};
  m_entityIDs.ship = spawnEntity(pos, vel, health, m_sprites.ship);

  // aliens
  YX<int> alienPos{m_alienStartingPoint};
  for(int y = 0; y < m_alienFormation.y; ++y) {
    for(int x = 0; x < m_alienFormation.x; ++x) {
      // create an entity and registers it
      pos = {static_cast<float>(alienPos.y), static_cast<float>(alienPos.x)};
      vel = {0, 1};
      health = 3;
      Entity::ID id = spawnEntity(pos, vel, health, m_sprites.aliens[y]);
      m_entityIDs.aliens.push_back(id);

      // shifts positions for the next column
      alienPos.x += m_sprites.aliens[y]->size().x + 2;
    }
    // shifts positions for the next line
    alienPos.y += m_sprites.aliens[y]->size().y + 2;
    alienPos.x = m_alienStartingPoint.x;
  }
}

void Program::endCurses()
{
  endwin();
}

//////////////////

Entity::ID Program::spawnEntity(YX<float> pos, YX<float> vel, int health, std::shared_ptr<Sprite>& sprite)
{
  Entity e{pos, vel, health, sprite};
  auto id = e.id();
  m_collisionBuffer.add(id);
  m_entities.emplace(id, std::move(e));
  return id;
}

bool Program::updateFramebuffer()
{
  // Output is buffered. If there are any changes from the previous one, it is printed
  // This is done to avoid flickering and unneded screen updates
  std::vector<chtype> buffer{};
  buffer.resize(m_framebuffer.size());
  for(auto& e : m_entities) {
    drawSprite(buffer, e.second);
  }

  if(buffer != m_framebuffer) {
    m_framebuffer = buffer;
    return true;
  }
  return false;
}

void Program::render(float frameDuration)
{
  wclear(stdscr);
  wclear(m_arenaWin);
  box(m_arenaBorderWin, 0, 0);
  // box(arenaWin, 0, 0);

  // Debug
  bool checkerboard = false;
  if(m_debugMode) {
    // Draw Collisions
    for(int y{}; y < getmaxy(m_arenaWin); ++y) {
      for(int x{}; x < getmaxx(m_arenaWin); ++x) {
        wmove(m_arenaWin, y, x);
        if(m_collisionBuffer.at(YX<int>{y, x}) != CollisionBuffer::Empty) {
          waddch(m_arenaWin, ACS_CKBOARD | COLOR_PAIR(COLOR_BLACK));
        } else {
          waddch(m_arenaWin, ACS_CKBOARD | COLOR_PAIR(COLOR_CYAN + checkerboard));
        }

        checkerboard = !checkerboard;
      }
      checkerboard = !checkerboard;
    }
    int framerate = 1.f / frameDuration;
    mvprintw(0, 0, "ts[%f]", frameDuration);
    mvprintw(1, 0, "shipYX[%f, %f]", m_entities.at(m_entityIDs.ship).position().y, m_entities.at(m_entityIDs.ship).position().x);
    mvprintw(2, 0, "bulletCount[%lu]", m_entityIDs.bullets.size());
    mvprintw(3, 0, "framerate[%i]", framerate);
    mvprintw(4, 0, "alienCount[%i]", m_entityIDs.aliens.size());
    auto it = m_entityIDs.aliens.begin();
    mvprintw(5, 0, "alienIDs[%i, %i, %i]", *it, *(++it), *(++it));
    mvprintw(6, 0, "spriteSize[%i]", m_sprites.aliens[0]->size().x);
  } else {
    // Draw sprites
    for(auto& e : m_entities) {
      auto& entity = e.second;
      YX<int> drawingPoint{
        .y = drawingPoint.y = std::round(entity.position().y),
        .x = drawingPoint.x = std::round(entity.position().x),
      };
      wmove(m_arenaWin, drawingPoint.y, drawingPoint.x);

      for(int i{}; i < entity.sprite().bufferSize(); ++i) {
        if(entity.sprite()[i] == '\n') {
          ++drawingPoint.y;
          wmove(m_arenaWin, drawingPoint.y, drawingPoint.x);
          continue;
        }

        waddch(m_arenaWin, entity.sprite()[i]);
      }
      // flick = !flick;
      // drawSprite(m_arenaWin, e.second);
    }
  }

  refresh();
  wrefresh(m_arenaBorderWin);
  wrefresh(m_arenaWin);
}

void Program::drawSprite(std::vector<chtype>& buffer, Entity& entity)
{
  YX<int> drawingPoint{
    .y = drawingPoint.y = std::round(entity.position().y),
    .x = drawingPoint.x = std::round(entity.position().x),
  };

  for(int i{}; i < entity.sprite().bufferSize(); ++i) {
    if(entity.sprite()[i] == '\n') {
      ++drawingPoint.y;
      buffer[drawingPoint.y * getmaxy(m_arenaWin) + drawingPoint.x] = (entity.sprite()[i]);
      continue;
    }
  }
  // flick = !flick;
}

// void Program::drawSprite(WINDOW* win, Entity& entity)
// {
//   static bool flick = 0;
//   YX<int> drawingPoint{
//     .y = drawingPoint.y = std::round(entity.position().y),
//     .x = drawingPoint.x = std::round(entity.position().x),
//   };
//   wmove(win, drawingPoint.y, drawingPoint.x);
//
//   for(int i{}; i < entity.sprite().bufferSize(); ++i) {
//     if(entity.sprite()[i] == '\n') {
//       ++drawingPoint.y;
//       wmove(win, drawingPoint.y, drawingPoint.x);
//       continue;
//     }
//
//     init_pair(1, COLOR_RED + flick, -1);
//     waddch(win, entity.sprite()[i] | COLOR_PAIR(1));
//   }
//   // flick = !flick;
// }

void Program::paintBorders()
{
  int my{getmaxy(m_arenaWin) - 1};
  int mx{getmaxx(m_arenaWin) - 1};
  m_collisionBuffer.paint(YX<int>{0, 0}, YX<int>{0, mx});
  m_collisionBuffer.paint(YX<int>{my, 0}, YX<int>{my, mx});
  m_collisionBuffer.paint(YX<int>{0, 0}, YX<int>{my, 0});
  m_collisionBuffer.paint(YX<int>{0, mx}, YX<int>{my, mx});
}

void Program::logic(int input, float timeStep, bool& force)
{
  auto& ts = timeStep;

  // Show Collisions
  if(input == '1') {
    m_debugMode = !m_debugMode;
    force = true;
  }

  // Exit
  if(input == 'q') {
    m_gameState = GameState::quitted;
    return;
  }

  // Move ship and Spawn bullets
  Entity& shipEntity = m_entities.at(m_entityIDs.ship);
  static auto lastShot{std::chrono::steady_clock::now()};
  auto moveShip = [&, this](int direction) {
    shipEntity.position().x += direction * ts * 16.f;
    std::vector<Entity::ID> collisions = m_collisionBuffer.collides(shipEntity.id());
    for(auto& e : collisions) {
      if(e != shipEntity.id()) {
        shipEntity.position().x += -direction * ts * 16.f;
        break;
      }
    }
  };
  switch(input) {
    case ';':
      moveShip(1);
      break;
    case 'j':
      moveShip(-1);
      break;
    case ' ':
      auto now = std::chrono::steady_clock::now();
      if((now - lastShot) >= std::chrono::milliseconds(300)) {
        lastShot = now;

        static bool side{};
        int health = 3;
        YX<float> position{
          .y = shipEntity.position().y - (shipEntity.sprite().size().y / 2.0f) + 1, // (+1) the bullet will be moved latter in this function
          .x = shipEntity.position().x + side + (shipEntity.sprite().size().x / 2.0f - 1),
        };
        YX<float> velocity = {8, 0};
        Entity::ID id = spawnEntity(position, velocity, health, m_sprites.shipBullet);
        m_entityIDs.bullets.push_back(id);
        side = !side;
      }
      break;
  }

  // move aliens
  for(auto& alienID : m_entityIDs.aliens) {
    auto& alien = m_entities.at(alienID);
    alien.position().x += (m_alienVelocity.x) * ts;
    // alien.pos.y += (alienVelocity.y + alienSpeedIntensifier) * ts;
  }
  static int direction{1};
  static float groupMovement{0.f};
  groupMovement += m_alienVelocity.x * ts;
  if(groupMovement <= -4.f || groupMovement >= 4.f) {
    m_alienVelocity.x = m_alienVelocity.x * direction;
    // alienVelocity.y *= -1;
    direction = -direction;
  }

  // Move bullets and handle colors
  for(auto& bulletID : m_entityIDs.bullets) {
    auto& bullet = m_entities.at(bulletID);

    bullet.position().x += bullet.velocity().x * ts;
    bullet.position().y -= bullet.velocity().y * ts;

    // YX<int> bulletPos{static_cast<int>(bullet.pos.y), static_cast<int>(bullet.pos.x)};
    int hit = m_collisionBuffer.at(bullet.position());
    if(hit != CollisionBuffer::Empty && hit != bulletID) {
      if(hit != CollisionBuffer::Invalid) {
        m_entities.at(hit).health() -= 1;
      }
      bullet.health() = 0;
    }
  }

  // Erase Dead Entities (Bullets)
  std::vector<Entity::ID> bulletsToErase{};
  for(auto& bulletID : m_entityIDs.bullets) // destroy bullets with health 0
  {
    auto& bullet = m_entities.at(bulletID);
    if(bullet.health() == 0) {
      bulletsToErase.push_back(bulletID);
    }
  }

  for(auto& id : bulletsToErase) {
    m_collisionBuffer.remove(id);
    m_entities.erase(id);
    m_entityIDs.bullets.remove(id);
  }

  // Erase Dead Entities (Aliens)
  std::vector<Entity::ID> aliensToErase{};
  for(auto& alienID : m_entityIDs.aliens) // destroy aliens with health 0
  {
    auto& alien = m_entities.at(alienID);
    if(alien.health() == 0) {
      // Small alien groups should be faster
      aliensToErase.push_back(alienID);
      if(m_alienVelocity.x > 0)
        m_alienVelocity.x += 0.125;
      else
        m_alienVelocity.x -= 0.125;
    }
  }

  for(auto& id : aliensToErase) {
    m_collisionBuffer.remove(id);
    m_entities.erase(id);
    m_entityIDs.aliens.remove(id);
  }

  if(m_entityIDs.aliens.size() == 0) {
    m_gameState = GameState::won;
  }

  if(m_entities.at(m_entityIDs.ship).health() == 0)
    m_gameState = GameState::lose;

  m_collisionBuffer.update();
  paintBorders();
}

void Program::run()
{
  auto& now{std::chrono::steady_clock::now};
  using Duration = std::chrono::duration<float>;
  auto updateStartTime = now();
  auto renderStartTime = now();
  auto updateEndTime = now();
  auto renderEndTime = now();
  Duration frameCounter{0};
  Duration updateDuration{};
  Duration minimunTimeStep{1 / 48}; // 48 updates per second

  int input{};
  while(input != 'q') {
    // time since the last update
    updateEndTime = now();
    updateDuration = updateEndTime - updateStartTime;
    updateStartTime = updateEndTime;

    // input and processing
    input = getch();
    bool forceRender = false;
    logic(input, updateDuration.count(), forceRender);
    // impose a limit on UPS (updates per second)
    // kinda necessary because only individual keystrokes are registered in the terminal
    if(updateDuration < minimunTimeStep) {
      std::this_thread::sleep_for(minimunTimeStep - updateDuration);
    }

    renderEndTime = now();
    frameCounter += renderEndTime - renderStartTime;
    renderStartTime = renderEndTime;
    if(frameCounter > std::chrono::milliseconds(1000 / 24)) {
      render(frameCounter.count());
      frameCounter = {};
    }
  }
}
