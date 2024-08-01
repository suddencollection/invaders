#include "collisionBuffer.hpp"

#include "yx.hpp"

#include <algorithm>
#include <cmath>

CollisionBuffer::CollisionBuffer(YX<int> gridSize, std::unordered_map<Entity::ID, Entity>& entities) :
  m_entities{entities},
  m_gridSize{gridSize}
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

Entity::ID CollisionBuffer::raycast(YX<float> rayStart, YX<float> rayDir)
{
  // Normalize
  float width = std::sqrt(rayDir.y * rayDir.y + rayDir.x + rayDir.x);
  rayDir.x /= width;
  rayDir.y /= width;

  YX<float> rayUnitStepSize = {
    .y = std::sqrt((rayDir.x / rayDir.y) * (rayDir.x / rayDir.y) + 1),
    .x = std::sqrt(1 + (rayDir.y / rayDir.x) * (rayDir.y / rayDir.x)),
  };

  YX<int> mapCheck = {
    .y = static_cast<int>(rayStart.y),
    .x = static_cast<int>(rayStart.x),
  };
  YX<int> rayLenght1D;
  YX<int> step;
  Entity::ID collision = CollisionBuffer::Empty;

  if(rayDir.x < 0) {
    step.x = -1;
    // If the ray is going backywards in an axis
    rayLenght1D.x = (rayStart.x - float(mapCheck.x)) * rayUnitStepSize.x;
  } else {
    step.x = 1;
    // If the ray is going in the positive direction
    rayLenght1D.x = (float(mapCheck.x + 1) - rayStart.x) * rayUnitStepSize.x;
  }

  if(rayDir.y < 0) {
    step.y = -1;
    rayLenght1D.y = (rayStart.y - float(mapCheck.y)) * rayUnitStepSize.y;
  } else {
    step.y = 1;
    rayLenght1D.y = (float(mapCheck.y + 1) - rayStart.y) * rayUnitStepSize.y;
  }

  bool tileFound = false;
  float maxDistance = 100.f;
  float distance = 0.0f;
  while(!tileFound && distance < maxDistance) {
    // Whichever the smallest is the current collision edge point
    if(rayLenght1D.x < rayLenght1D.y) {
      mapCheck.x += step.x;
      distance = rayLenght1D.x;
      rayLenght1D.x += rayUnitStepSize.x;
    } else {
      mapCheck.y += step.y;
      distance = rayLenght1D.y;
      rayLenght1D.y += rayUnitStepSize.y;
    }

    if(mapCheck.x >= 0 &&
       mapCheck.x < m_gridSize.x &&
       mapCheck.y >= 0 &&
       mapCheck.y < m_gridSize.y) {
      // Check for collisions
      YX<int> pos{
        .y = mapCheck.y * m_gridSize.x,
        .x = mapCheck.x,
      };
      if(m_cells.contains(pos)) {
        if(m_cells.at(pos) != CollisionBuffer::Empty) {
          tileFound = true;
          collision = m_cells.at(pos);
        }
      }
    }

    YX<float> intersection;
    if(tileFound) {
      intersection = rayStart + rayDir * distance;
    }
  }
  return collision;
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
