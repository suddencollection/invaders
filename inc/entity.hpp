#pragma once

#include "sprite.hpp"

#include <cassert>
#include <stack>

class Entity
{
public:
  using ID = int;
  Entity() = delete;
  Entity(Entity& other) = delete;

  Entity(YX<float> pos, YX<float> vel, int health, std::shared_ptr<Sprite> sprite);
  Entity(Entity&& other);
  ~Entity();


  YX<float>& position() { return m_position; }
  YX<float>& velocity() { return m_velocity; }
  int& health() { return m_health; }
  Sprite& sprite() { return *m_sprite; }
  ID id() const
  {
    assert(m_id != -1 && "Invalid ID");
    return m_id;
  }

  // unused (unneeded)
  void setSprite(std::shared_ptr<Sprite>& _) { _->size(); }

private:
  ID m_id{-1};
  YX<float> m_position{0, 0};
  YX<float> m_velocity{0, 0};
  int m_health{1};
  std::shared_ptr<Sprite> m_sprite; // the easy way

  static std::stack<ID> m_availableIDs;
  static int genID()
  {
    static int IDcount = 0;
    IDcount += 1;
    return IDcount;
  }
};
