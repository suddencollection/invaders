#include "entity.hpp"

std::stack<Entity::ID> Entity::m_availableIDs{};

Entity::Entity(YX<float> pos, YX<float> vel, int health, std::shared_ptr<Sprite> sprite) :
  m_position{pos}, m_velocity{vel}, m_health{health}, m_sprite{sprite}
{
  if(m_availableIDs.empty())
    m_availableIDs.push(genID());

  m_id = m_availableIDs.top();
  m_availableIDs.pop();
}

// only std::move's are allowed
Entity::Entity(Entity&& other) :
  m_id{other.m_id},
  m_position{other.m_position},
  m_velocity{other.m_velocity},
  m_health{other.m_health},
  m_sprite{std::move(other.m_sprite)}
{
  other.m_id = {-1};
  other.m_position = {};
  other.m_velocity = {};
  other.m_health = {};
  other.m_sprite = nullptr;
}

Entity::~Entity()
{
  if(m_id != -1) {
    m_availableIDs.push(m_id);
  }
}
