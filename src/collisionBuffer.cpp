#include "collisionBuffer.hpp"

#include <cmath>

CollisionBuffer::CollisionBuffer(std::unordered_map<Entity::ID, Entity>& entities) :
  m_entities{entities}
{
}

void CollisionBuffer::resize(YX<int> newSize)
{
  if(newSize.y > static_cast<int>(m_rows.size())) {
    m_rows.resize(newSize.y);
  }

  for(auto& row : m_rows) {
    if(newSize.x > static_cast<int>(row.size())) {
      row.resize(newSize.x);
    }
  }

  clear({0, 0}, newSize);
}

void CollisionBuffer::setCollision(Entity::ID e)
{
  // if(m_entities.find(e) == m_entities.end())
  //   throw std::runtime_error("Invalid Entity ID");

  YX<int> pos1{static_cast<int>(std::round(m_entities.at(e).position().y)), static_cast<int>(std::round(m_entities.at(e).position().x))};
  YX<int> pos2{pos1.y + m_entities.at(e).sprite().size().y - 1, pos1.x + m_entities.at(e).sprite().size().x - 1};

  setCollision(pos1, pos2, e);
}

void CollisionBuffer::setCollision(YX<int> pos1, YX<int> pos2, Entity::ID e)
{
  for(int y{pos1.y}; y <= pos2.y; ++y)
    for(int x{pos1.x}; x <= pos2.x; ++x) {
      m_rows[y][x] = e;
    }
}

void CollisionBuffer::setCollision(YX<float> pos1, YX<float> pos2, Entity::ID e)
{
  YX<int> p1{static_cast<int>(std::round(pos1.y)), static_cast<int>(std::round(pos1.x))};
  YX<int> p2{static_cast<int>(std::round(pos2.y)), static_cast<int>(std::round(pos2.x))};
  setCollision(p1, p2, e);
}

void CollisionBuffer::clear(YX<float> pos1, YX<float> pos2)
{
  YX<int> p1{static_cast<int>(std::round(pos1.y)), static_cast<int>(std::round(pos1.x))};
  YX<int> p2{static_cast<int>(std::round(pos2.y)), static_cast<int>(std::round(pos2.x))};
  clear(p1, p2);
}

void CollisionBuffer::clear(Entity::ID e)
{
  YX<int> p1{static_cast<int>(std::round(m_entities.at(e).position().y)), static_cast<int>(std::round(m_entities.at(e).position().x))};
  YX<int> p2{p1.y + m_entities.at(e).sprite().size().y, p1.x + m_entities.at(e).sprite().size().x};
  clear(p1, p2);
}

void CollisionBuffer::clear(YX<int> pos1, YX<int> pos2)
{
  for(int y{pos1.y}; y < pos2.y; ++y)
    for(int x{pos1.x}; x < pos2.x; ++x) {
      m_rows[y][x] = 0;
    }
}

int CollisionBuffer::at(YX<int> index)
{
  // check if out of bounds
  if(index.x < 0 || index.x > static_cast<int>(m_rows[0].size()) || index.y < 0 || index.y > static_cast<int>(m_rows.size()))
    return -1;

  return m_rows[index.y][index.x];
}

int CollisionBuffer::at(YX<float> index)
{
  return at(YX<int>{static_cast<int>(std::round(index.y)), static_cast<int>(std::round(index.x))});
}
