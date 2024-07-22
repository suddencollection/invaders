#pragma once

#include "sprite.hpp"
#include "yx.hpp"

#include <stack>

class Entity
{
public:
  using ID = int;
  Entity() = delete;

  Entity(YX<float> pos, YX<float> vel, int health, std::shared_ptr<Sprite> sprite) :
    m_position{pos}, m_velocity{vel}, m_health{health}, m_sprite{sprite}
  {
    if(m_availableIDs.empty())
      m_availableIDs.push(genID());

    m_id = m_availableIDs.top();
    m_availableIDs.pop();
  }

  ~Entity()
  {
    m_availableIDs.push(m_id);
  }

  ID id() const { return m_id; }
  YX<float>& position() { return m_position; }
  YX<float>& velocity() { return m_velocity; }
  int& health() { return m_health; }
  Sprite& sprite() { return *m_sprite; }

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
