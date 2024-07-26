#include "collisionBuffer.hpp"

#include <algorithm>
#include <cmath>

CollisionBuffer::CollisionBuffer(std::unordered_map<Entity::ID, Entity>& entities) :
  m_entities{entities}
{
}

void CollisionBuffer::add(Entity::ID id)
{
  m_collidersIDs.push_back(id);
}

void CollisionBuffer::paint(YX<int> start, YX<int> end)
{
  for(int y = start.y; y <= end.y; ++y) {
    for(int x = start.x; x <= end.x; ++x) {
      m_cells.emplace(YX<int>{y, x}, CollisionBuffer::Invalid);
    }
  }
}

void CollisionBuffer::remove(Entity::ID id)
{
  m_collidersIDs.erase(std::find(m_collidersIDs.begin(), m_collidersIDs.end(), id));
}

void CollisionBuffer::update()
{
  m_cells.clear();
  for(auto& e : m_collidersIDs) {
    map_entity(e);
  }
}

auto CollisionBuffer::at(YX<int> pos) -> Entity::ID
{
  return m_cells.contains(pos) ? m_cells.at(pos) : CollisionBuffer::Empty;
}

auto CollisionBuffer::at(YX<float> pos) -> Entity::ID
{
  return at(YX<int>{static_cast<int>(pos.y), static_cast<int>(pos.x)});
}

auto CollisionBuffer::collides(Entity::ID id) -> std::vector<Entity::ID>
{
  std::vector<Entity::ID> collisions;
  for_each_cell(id, [&, this](YX<int> cell) {
    if(m_cells.contains(cell)) {
      collisions.push_back(m_cells.at(cell));
    }
  });
  return collisions;
}

////////

void CollisionBuffer::for_each_cell(Entity::ID id, std::function<void(YX<int>)> fun)
{
  Entity& entity = m_entities.at(id);
  Sprite& sprite = m_entities.at(id).sprite();

  for(int y = 0; y < sprite.size().y; ++y) {
    for(int x = 0; x < sprite.size().x; ++x) {
      auto key = YX<int>{
        .y = y + static_cast<int>(entity.position().y),
        .x = x + static_cast<int>(entity.position().x)};
      fun(key);
    }
  }
}

void CollisionBuffer::unmap_entity(Entity::ID id)
{
  for_each_cell(id, [this](YX<int> cell) { m_cells.erase(cell); });
}

void CollisionBuffer::map_entity(Entity::ID id)
{
  for_each_cell(id, [this, id](YX<int> cell) { m_cells.emplace(cell, id); });
}


// void CollisionBuffer::setCollision(Entity::ID e)
// {
//   // if(m_entities.find(e) == m_entities.end())
//   //   throw std::runtime_error("Invalid Entity ID");
//
//   YX<int> pos1{static_cast<int>(std::round(m_entities.at(e).position().y)), static_cast<int>(std::round(m_entities.at(e).position().x))};
//   YX<int> pos2{pos1.y + m_entities.at(e).sprite().size().y - 1, pos1.x + m_entities.at(e).sprite().size().x - 1};
//
//   setCollision(pos1, pos2, e);
// }
//
// void CollisionBuffer::setCollision(YX<int> pos1, YX<int> pos2, Entity::ID e)
// {
//   for(int y{pos1.y}; y <= pos2.y; ++y)
//     for(int x{pos1.x}; x <= pos2.x; ++x) {
//       m_rows[y][x] = e;
//     }
// }
//
// void CollisionBuffer::setCollision(YX<float> pos1, YX<float> pos2, Entity::ID e)
// {
//   YX<int> p1{static_cast<int>(std::round(pos1.y)), static_cast<int>(std::round(pos1.x))};
//   YX<int> p2{static_cast<int>(std::round(pos2.y)), static_cast<int>(std::round(pos2.x))};
//   setCollision(p1, p2, e);
// }
//
// void CollisionBuffer::clear(YX<float> pos1, YX<float> pos2)
// {
//   YX<int> p1{static_cast<int>(std::round(pos1.y)), static_cast<int>(std::round(pos1.x))};
//   YX<int> p2{static_cast<int>(std::round(pos2.y)), static_cast<int>(std::round(pos2.x))};
//   clear(p1, p2);
// }
//
// void CollisionBuffer::clear(Entity::ID e)
// {
//   YX<int> p1{static_cast<int>(std::round(m_entities.at(e).position().y)), static_cast<int>(std::round(m_entities.at(e).position().x))};
//   YX<int> p2{p1.y + m_entities.at(e).sprite().size().y, p1.x + m_entities.at(e).sprite().size().x};
//   clear(p1, p2);
// }
//
// void CollisionBuffer::clear(YX<int> pos1, YX<int> pos2)
// {
//   for(int y{pos1.y}; y < pos2.y; ++y)
//     for(int x{pos1.x}; x < pos2.x; ++x) {
//       m_rows[y][x] = 0;
//     }
// }
//
// int CollisionBuffer::at(YX<int> index)
// {
//   // check if out of bounds
//   if(index.x < 0 || index.x > static_cast<int>(m_rows[0].size()) || index.y < 0 || index.y > static_cast<int>(m_rows.size()))
//     return -1;
//
//   return m_rows[index.y][index.x];
// }
//
// int CollisionBuffer::at(YX<float> index)
// {
//   return at(YX<int>{static_cast<int>(std::round(index.y)), static_cast<int>(std::round(index.x))});
// }
