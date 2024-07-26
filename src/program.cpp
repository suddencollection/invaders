#include "program.hpp"

#include <cassert>
#include <cmath>
#include <thread>

Program::Program(std::filesystem::path sprites_path)
{
  initCurses();
  createWindows();
  loadSprites(sprites_path);
  createEntities();
  setCollisions();
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

  init_pair(1, -1, COLOR_MAGENTA);
  init_pair(2, -1, COLOR_GREEN);
  init_pair(3, -1, COLOR_WHITE);
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
  Entity ship{pos, vel, health, m_sprites.ship};
  m_entityIDs.ship = ship.id();
  m_entities.emplace(m_entityIDs.ship, std::move(ship));

  // aliens
  YX<int> alienPos{m_alienStartingPoint};
  for(int y = 0; y < m_alienFormation.y; ++y) {
    for(int x = 0; x < m_alienFormation.x; ++x) {
      // create an entity and registers it
      pos = {static_cast<float>(alienPos.y), static_cast<float>(alienPos.x)};
      vel = {0, 1};
      health = 1;
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

void Program::setCollisions()
{
  // ship
  m_collisionBuffer.add(m_entityIDs.ship);

  // aliens
  for(Entity::ID e : m_entityIDs.aliens) {
    m_collisionBuffer.add(e);
  }

  // wall
  paintBorders();
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
  m_entities.insert({e.id(), std::move(e)});
  return id;
}

void Program::render(float frameDuration)
{
  wclear(stdscr);
  wclear(m_arenaWin);
  box(m_arenaBorderWin, 0, 0);
  // box(arenaWin, 0, 0);

  // Draw sprites
  for(auto& e : m_entities) {
    drawSprite(m_arenaWin, e.second);
  }

  // Draw Collisions
  bool checkerboard = false;
  if(m_drawCollisions) {
    for(int y{}; y < getmaxy(m_arenaWin); ++y) {
      for(int x{}; x < getmaxx(m_arenaWin); ++x) {
        wmove(m_arenaWin, y, x);
        if(m_collisionBuffer.at(YX<int>{y, x}) != CollisionBuffer::Empty) {
          waddch(m_arenaWin, ACS_CKBOARD | COLOR_PAIR(1));
        } else {
          waddch(m_arenaWin, ACS_CKBOARD | COLOR_PAIR(2 + checkerboard));
        }

        checkerboard = !checkerboard;
      }
      checkerboard = !checkerboard;
    }
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

  refresh();
  wrefresh(m_arenaBorderWin);
  wrefresh(m_arenaWin);
}

void Program::drawSprite(WINDOW* win, Entity& entity)
{
  YX<int> drawingPoint{
    .y = drawingPoint.y = std::round(entity.position().y),
    .x = drawingPoint.x = std::round(entity.position().x),
  };
  wmove(win, drawingPoint.y, drawingPoint.x);

  for(int i{}; i < entity.sprite().bufferSize(); ++i) {
    if(entity.sprite()[i] == '\n') {
      ++drawingPoint.y;
      wmove(win, drawingPoint.y, drawingPoint.x);
      continue;
    }

    waddch(win, entity.sprite()[i]);
  }
}

void Program::paintBorders()
{
  int my{getmaxy(m_arenaWin) - 1};
  int mx{getmaxx(m_arenaWin) - 1};
  m_collisionBuffer.paint(YX<int>{0, 0}, YX<int>{0, mx});
  m_collisionBuffer.paint(YX<int>{my, 0}, YX<int>{my, mx});
  m_collisionBuffer.paint(YX<int>{0, 0}, YX<int>{my, 0});
  m_collisionBuffer.paint(YX<int>{0, mx}, YX<int>{my, mx});
}

void Program::logic(int input, float timeStep)
{
  auto& ts = timeStep;

  // Show Collisions
  switch(input) {
    case '1':
      m_drawCollisions = false;
      break;
    case '2':
      m_drawCollisions = true;
      break;
  }

  // Move ship and Spawn bullets
  Entity& shipEntity = m_entities.at(m_entityIDs.ship);
  // leftShipCollider.x -= 16.f * ts;
  // shipEntity.position().x -= 16.f * ts;
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
          .y = shipEntity.position().y - (shipEntity.sprite().size().y / 2.0f) - 1,
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

  // mvprintw(7, 0, "groupMovement[%f]", groupMovement);
  // mvprintw(8, 0, "vel[%f, %f]", m_alienVelocity.y, m_alienVelocity.x);


  // Move bullets
  for(auto& bulletID : m_entityIDs.bullets) {
    auto& bullet = m_entities.at(bulletID);

    bullet.position().x += bullet.velocity().x * ts;
    bullet.position().y -= bullet.velocity().y * ts;

    // YX<int> bulletPos{static_cast<int>(bullet.pos.y), static_cast<int>(bullet.pos.x)};
    int hit = m_collisionBuffer.at(bullet.position());
    if(hit != CollisionBuffer::Empty) {
      if(hit != CollisionBuffer::Invalid) {
        auto it = m_entities.find(hit);
        it->second.health() -= 1;
      }

      bullet.health() = 0;
    }
  }

  // Erase Dead Entities
  std::vector<Entity::ID> bulletsToErase{};
  std::vector<Entity::ID> aliensToErase{};

  for(auto& bulletID : m_entityIDs.bullets) // destroy bullets with health 0
  {
    auto& bullet = m_entities.at(bulletID);
    if(bullet.health() == 0) {
      m_entities.erase(bulletID);
      bulletsToErase.push_back(bulletID);
    }
  }

  for(auto& alienID : m_entityIDs.aliens) // destroy aliens with health 0
  {
    auto& alien = m_entities.at(alienID);
    if(alien.health() == 0) {
      m_collisionBuffer.remove(alienID);
      m_entities.erase(alienID);

      // Small alien groups should be faster
      aliensToErase.push_back(alienID);
      if(m_alienVelocity.x > 0)
        m_alienVelocity.x += 0.125;
      else
        m_alienVelocity.x -= 0.125;
    }
  }

  for(auto& id : aliensToErase)
    m_entityIDs.aliens.remove(id);

  for(auto& id : bulletsToErase)
    m_entityIDs.bullets.remove(id);

  if(m_entityIDs.aliens.size() == 0)
    m_gameState = GameState::won;

  if(m_entities.at(m_entityIDs.ship).health() == 0)
    m_gameState = GameState::lose;

  m_collisionBuffer.update();
  paintBorders();
}

void Program::run()
{
  auto& now{std::chrono::steady_clock::now};
  using Duration = std::chrono::duration<float>;
  auto startTime = now();
  auto endTime = now();
  Duration frameDuration{};
  Duration minimunTimeStep{0.0208333}; //{0.041};

  while(true) {
    // time since the last frame
    endTime = now();
    frameDuration = endTime - startTime;
    startTime = endTime;

    // input and processing
    int input = getch();
    logic(input, frameDuration.count());

    if(frameDuration < minimunTimeStep)
      std::this_thread::sleep_for(minimunTimeStep - frameDuration);

    render(frameDuration.count());
  }

  // int input{};
  // while(gameState == GameState::running) {
  //   endTime = now();
  //   frameDuration = endTime - startTime;
  //   startTime = endTime;
  //
  //   input = getch();
  //   logic(input, frameDuration.count());
  //
  //   if(frameDuration < minimunTimeStep)
  //     std::this_thread::sleep_for(minimunTimeStep - frameDuration);
  //
  //   render(input, frameDuration.count());
  //
  //   if(input == 'q')
  //     gameState = GameState::quitted;
  // }
  //
  // nodelay(stdscr, false);
  //
  // if(m_entities[m_entityIDs.ship].health() > 0 && m_entityIDs.aliens.size() == 0) {
  //   "you won";
  // } else if(entities[entityIDs.ship].health == 0 && alienIDs.size() > 0) {
  //   "you lost";
  // } else {
  //   "draw?";
  // }
  //
  // refresh();
  // wrefresh(resultWin);
  // while(getch() != '\n')
  //   continue;
}
