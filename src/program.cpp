#include "program.hpp"

#include <cassert>

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

  init_pair(1, -1, COLOR_RED);
  init_pair(2, -1, COLOR_BLACK);
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
    for(int column{}; column < m_alienFormation.x; ++column) {
      m_sprites.aliens.push_back(std::make_shared<Sprite>(path / ("alien" + std::to_string(line))));
    }
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
  m_entities.insert({m_entityIDs.ship, ship});

  // aliens
  YX<int> alienPos{m_alienStartingPoint};
  for(int line{}; line < m_alienFormation.y; ++line) {
    for(int column{}; column < m_alienFormation.x; ++column) {
      // create an entity
      pos = {static_cast<float>(alienPos.y), static_cast<float>(alienPos.x)};
      vel = {0, 1};
      health = 1;
      Entity alien{pos, vel, health, m_sprites.aliens[line]};
      alien.position().x += alien.sprite().size().x + 2;

      // registers it
      m_entities.insert({alien.id(), alien});
      m_entityIDs.aliens.push_back(alien.id());
    }

    // shifts positions for the next line
    alienPos.y += m_sprites.aliens[line]->size().y + 1;
    alienPos.x = m_alienStartingPoint.x;
  }
}

void Program::setCollisions()
{
  m_collisionBuffer.resize(m_arenaSize);

  // ship
  m_collisionBuffer.setCollision(m_entityIDs.ship);

  // aliens
  for(Entity::ID e : m_entityIDs.aliens) {
    m_collisionBuffer.setCollision(e);
  }

  // wall
  int my{getmaxy(m_arenaWin) - 1};
  int mx{getmaxx(m_arenaWin) - 1};
  m_collisionBuffer.setCollision(YX<int>{0, 0}, {0, mx}, -1);   // top
  m_collisionBuffer.setCollision(YX<int>{my, 0}, {my, mx}, -1); // bottom
  m_collisionBuffer.setCollision(YX<int>{0, 0}, {my, 0}, -1);   // left
  m_collisionBuffer.setCollision(YX<int>{0, mx}, {my, mx}, -1); // right
}

void Program::endCurses()
{
  endwin();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// void Program::drawSprite(WINDOW* win, Entity& entity)
// {
//   YX<int> drawingPoint;
//   drawingPoint.y = std::round(entity.pos.y);
//   drawingPoint.x = std::round(entity.pos.x);
//   wmove(win, drawingPoint.y, drawingPoint.x);
//
//   for(int i{}; i < entity.spriteView->bufferSize(); ++i) {
//     if(entity.spriteView[i] == '\n') {
//       ++drawingPoint.y;
//       wmove(win, drawingPoint.y, drawingPoint.x);
//
//       continue;
//     }
//
//     waddch(win, entity.spriteView[i]);
//   }
// }

// void Program::render(int input, float frameDuration)
// {
//   wclear(stdscr);
//   wclear(arenaWin);
//   box(arenaBorderWin, 0, 0);
//   //  box(arenaWin, 0, 0);
//
//   static bool drawCollisions{};
//
//   switch(input) {
//     case '1':
//       drawCollisions = false;
//       break;
//     case '2':
//       drawCollisions = true;
//       break;
//   }
//
//   switch(static_cast<int>(drawCollisions)) {
//     case false:
//       for(auto& e : entities) {
//         drawSprite(arenaWin, e.second);
//       }
//       break;
//     case true:
//       for(int y{}; y < getmaxy(arenaWin); ++y) {
//         for(int x{}; x < getmaxx(arenaWin); ++x) {
//           wmove(arenaWin, y, x);
//           if(collisionBuffer.at(YX<int>{y, x}) != 0) {
//             waddch(arenaWin, ACS_CKBOARD | COLOR_PAIR(1));
//           } else {
//             waddch(arenaWin, ACS_CKBOARD | COLOR_PAIR(2));
//           }
//         }
//       }
//       break;
//   }
//
//   int framerate = 1.f / frameDuration;
//   mvprintw(0, 0, "ts[%f]", frameDuration);
//   mvprintw(1, 0, "shipYX[%f, %f]", entities[entityIDs.ship].pos.y, entities[entityIDs.ship].pos.x);
//   mvprintw(2, 0, "bulletCount[%lu]", bulletIDs.size());
//   mvprintw(3, 0, "framerate[%i]", framerate);
//
//   refresh();
//   wrefresh(arenaBorderWin);
//   wrefresh(arenaWin);
// }

// void Program::logic(int input, float ts)
// {
//   //  collisionBuffer.clear(YX<int>{1, 1}, YX<int>{static_cast<int>(getmaxy(collisionWin)) - 1, static_cast<int>(getmaxx(collisionWin)) - 1});
//
//   // move ship + spawn bullets
//   auto& shipEntity{entities[entityIDs.ship]};
//   YX<float> leftShipCollider{shipEntity.pos};
//   YX<float> rightShipCollider{shipEntity.pos.y, shipEntity.pos.x + shipEntity.spriteView->size().x - 1};
//   static auto lastShot{std::chrono::steady_clock::now()};
//   switch(input) {
//     case ';':
//       rightShipCollider.x += 16.f * ts;
//       if(collisionBuffer.at(rightShipCollider) == 0) {
//         shipEntity.pos.x += 16.f * ts;
//       }
//       break;
//     case 'j':
//       leftShipCollider.x -= 16.f * ts;
//       if(collisionBuffer.at(leftShipCollider) == 0) {
//         shipEntity.pos.x -= 16.f * ts;
//       }
//       break;
//     case ' ':
//       auto now = std::chrono::steady_clock::now();
//       if((now - lastShot) >= std::chrono::milliseconds(300)) {
//         lastShot = now;
//
//         Entity::ID bullet{Entity::genID()};
//         bulletIDs.push_back(bullet);
//
//         static bool side{};
//         Entity& ship{shipEntity};
//
//         entities[bullet].health = 3;
//         entities[bullet].pos.x = ship.pos.x + side + (ship.spriteView->size().x / 2.0) - 1;
//         entities[bullet].pos.y = ship.pos.y - (ship.spriteView->size().y / 2.0) + 1;
//         entities[bullet].spriteView = shipBulletSprite.value();
//         entities[bullet].velocity = {8, 0};
//
//         side = !side;
//       }
//       break;
//   }
//
//   // move aliens
//   for(auto& alienID : alienIDs) {
//     collisionBuffer.clear(alienID);
//     auto& alien{entities[alienID]};
//     alien.pos.x += (alienVelocity.x) * ts;
//     //    alien.pos.y += (alienVelocity.y + alienSpeedIntensifier) * ts;
//     collisionBuffer.setCollision(alienID);
//   }
//
//   static int direction{1};
//   static float groupMovement{0.f};
//   groupMovement += alienVelocity.x * ts;
//   if(groupMovement <= -4.f || groupMovement >= 4.f) {
//     alienVelocity.x = alienVelocity.x * direction;
//     // alienVelocity.y *= -1;
//     direction = -direction;
//   }
//
//   mvprintw(4, 0, "groupMovement[%f]", groupMovement);
//   mvprintw(5, 0, "vel[%f, %f]", alienVelocity.y, alienVelocity.x);
//
//   // move bullets
//   for(auto& bulletID : bulletIDs) {
//     auto& bullet{entities[bulletID]};
//
//     bullet.pos.x += bullet.velocity.x * ts;
//     bullet.pos.y -= bullet.velocity.y * ts;
//
//     //    YX<int> bulletPos{static_cast<int>(bullet.pos.y), static_cast<int>(bullet.pos.x)};
//     int hit{collisionBuffer.at(bullet.pos)};
//     if(hit != 0) {
//       if(hit != -1) {
//         auto it{entities.find(hit)};
//         it->second.health -= 1;
//       }
//
//       bullet.health = 0;
//     }
//   }
//
//   static std::vector<Entity::ID> bulletsToErase;
//   for(auto& bulletID : bulletIDs) // destroy bullets with health 0
//   {
//     auto& bullet{entities[bulletID]};
//     if(bullet.health == 0) {
//       entities.erase(bulletID);
//       bulletsToErase.push_back(bulletID);
//     }
//   }
//
//   static std::vector<Entity::ID> aliensToErase;
//   for(auto& alienID : alienIDs) // destroy aliens with health 0
//   {
//     auto& alien{entities[alienID]};
//     if(alien.health == 0) {
//       collisionBuffer.clear(alienID);
//       entities.erase(alienID);
//       aliensToErase.push_back(alienID);
//       if(alienVelocity.x > 0)
//         alienVelocity.x += 0.125;
//       else
//         alienVelocity.x -= 0.125;
//     }
//   }
//
//   for(auto& id : aliensToErase)
//     alienIDs.remove(id);
//
//   for(auto& id : bulletsToErase)
//     bulletIDs.remove(id);
//
//   aliensToErase.clear();
//   bulletsToErase.clear();
//
//   if(alienIDs.size() == 0)
//     gameState = GameState::win;
//
//   if(entities[entityIDs.ship].health == 0)
//     gameState = GameState::lose;
// }

void Program::run()
{
  auto& now{std::chrono::steady_clock::now};
  auto startTime{now()};
  auto endTime{now()};
  std::chrono::duration<float> frameDuration{};
  std::chrono::duration<float> minimunTimeStep{0.0208333}; //{0.041};

  int input{};
  while(gameState == GameState::running) {
    endTime = now();
    frameDuration = endTime - startTime;
    startTime = endTime;

    input = getch();
    logic(input, frameDuration.count());

    if(frameDuration < minimunTimeStep)
      std::this_thread::sleep_for(minimunTimeStep - frameDuration);

    render(input, frameDuration.count());

    if(input == 'q')
      gameState = GameState::quitted;
  }

  nodelay(stdscr, false);

  YX<int> size{4, 16};
  WINDOW* resultWin{newwin(size.y, size.x, (getmaxy(stdscr) - size.y) / 2, (getmaxx(stdscr) - size.x) / 2)};

  box(resultWin, 0, 0);
  wmove(resultWin, getmaxy(resultWin) / 2, (getmaxx(resultWin) - 8) / 2);
  if(entities[entityIDs.ship].health > 0 && alienIDs.size() == 0) {
    wprintw(resultWin, "you won");
  } else if(entities[entityIDs.ship].health == 0 && alienIDs.size() > 0) {
    wprintw(resultWin, "you lost");
  } else {
    wprintw(resultWin, "draw?");
  }

  refresh();
  wrefresh(resultWin);
  while(getch() != '\n')
    continue;
}
