#pragma once

#include "entity.hpp"
#include "yx.hpp"

#include <unordered_map>
#include <vector>

class CollisionBuffer
{
public:
  CollisionBuffer(std::unordered_map<Entity::ID, Entity>& entities);
  void resize(YX<int> size);
  void setCollision(Entity::ID e);
  void setCollision(YX<int> pos1, YX<int> pos2, Entity::ID e);
  void setCollision(YX<float> pos1, YX<float> pos2, Entity::ID e);
  void clear(Entity::ID e);
  void clear(YX<int> pos1, YX<int> pos2);
  void clear(YX<float> pos1, YX<float> pos2);
  auto at(YX<int> index) -> int;
  auto at(YX<float> index) -> int;

private:
  std::unordered_map<Entity::ID, Entity>& m_entities;
  std::vector<std::vector<int>> m_rows;
};
