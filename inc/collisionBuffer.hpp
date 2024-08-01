#pragma once

#include "entity.hpp"

#include <unordered_map>
#include <vector>

class CollisionBuffer
{
public:
  static constexpr int Empty = 0;
  static constexpr int Invalid = -1;

  CollisionBuffer(YX<int> gridSize, std::unordered_map<Entity::ID, Entity>& entities);

  void add(Entity::ID);
  void paint(YX<int> start, YX<int> end);
  void remove(Entity::ID);
  auto at(YX<int>) -> Entity::ID;
  auto at(YX<float>) -> Entity::ID;
  auto collides(Entity::ID id) -> std::vector<Entity::ID>;
  void update();
  auto raycast(YX<float> rayStart, YX<float> rayDir) -> Entity::ID;

  // auto at(YX<int> index) -> int;
  // auto at(YX<float> index) -> int;

private:
  void map_entity(Entity::ID);
  void unmap_entity(Entity::ID);
  // void set_collision(YX<int> pos);
  void for_each_cell(Entity::ID, std::function<void(YX<int>)>);

  std::unordered_map<Entity::ID, Entity>& m_entities;
  std::vector<Entity::ID> m_collidersIDs;
  std::unordered_map<YX<int>, Entity::ID> m_cells;
  YX<int> m_gridSize;
};
