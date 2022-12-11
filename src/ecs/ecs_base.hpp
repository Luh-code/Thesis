#ifndef __ECS_BASE_H__
#define __ECS_BASE_H__

#include "../pch.h"

namespace Ths::ecs
{
  using Entity = uint32_t;
  const Entity MAX_ENTITIES = 10000;

  using ComponentType = uint32_t;
  const ComponentType MAX_COMPONENTS = 10000;

  using Signature = std::bitset<MAX_COMPONENTS>;

  class EntityManager
  {
  public:
    inline EntityManager()
    {
      for (Entity e = 0; e < MAX_ENTITIES; e++)
      {
        mAvailableEntities.push(e);
      }
    }

    inline std::optional<Entity> createEntity()
    {
      if (mLivingEntityCount >= MAX_ENTITIES)
      {
        LOG_ERROR("Tried to create new entity, when no more entities are available - returning empty");
        return std::optional<Entity>();
      }
      Entity id = mAvailableEntities.front();
      mAvailableEntities.pop();
      ++mLivingEntityCount;
      return std::optional<Entity>(id);
    }

    inline void destroyEntity(Entity entity)
    {
      if (entity >= MAX_ENTITIES)
      {
        LOG_ERROR("Tried to delete out-of-range entity - deleting nothing");
        return;
      }

      mSignatures[entity].reset();

      // May want to add check if entity is alive
      mAvailableEntities.push(entity);
      --mLivingEntityCount;
    }

    inline void setSignature(Entity entity, Signature signature)
    {
      if (entity >= MAX_ENTITIES)
      {
        LOG_ERROR("Tried to change signature of out-of-range entity - changing nothing");
        return;
      }

      mSignatures[entity] = signature;
    }

    inline std::optional<Signature> getSignature(Entity entity)
    {
      if (entity >= MAX_ENTITIES)
      {
        LOG_ERROR("Tried to get signature of out-of-range entity - got nothing");
        return std::optional<Signature>();
      }
      return std::optional<Signature>(mSignatures[entity]);
    }

  private:
    std::queue<Entity> mAvailableEntities {}; // Unused entity ID's
    std::array<Signature, MAX_ENTITIES> mSignatures {}; // Signatures corresponding to Entities
    uint32_t mLivingEntityCount {};
  };
}

#endif // __ECS_BASE_H__