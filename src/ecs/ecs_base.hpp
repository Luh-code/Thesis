#ifndef __ECS_BASE_H__
#define __ECS_BASE_H__

#include "../pch.h"

// ecs build from Austin Morlans Tutorial (https://austinmorlan.com/posts/entity_component_system/) & modded

namespace Ths::ecs
{
  using Entity = uint32_t;
  const Entity MAX_ENTITIES = 10000;

  using ComponentType = uint32_t;
  const ComponentType MAX_COMPONENTS = 10000;

  using Signature = std::bitset<MAX_COMPONENTS>;

  class IComponentArray
  {
  public:
    virtual ~IComponentArray() = default;
    virtual void entityDestroyed(Entity entity) = 0;
  };

  template<typename T>
  class ComponentArray : IComponentArray
  {
  public:
    inline void insertData(Entity entity, T component)
    {
      if (mEntityToIndexMap.find(entity) == mEntityToIndexMap.end())
      {
        LOG_ERROR("Tried adding same component to entity multiple times - adding nothing");
        return;
      }

      size_t newIndex = mSize;
      mEntityToIndexMap[entity] = newIndex;
      mIndexToEntityMap[newIndex] = entity;
      mComponentArray[newIndex] = component;
      ++mSize;
    }

    inline void removeData(Entity entity)
    {
      if (mEntityToIndexMap.find(entity) != mEntityToIndexMap.end())
      {
        LOG_ERROR("Tried removing non-exitent entity - removing nothing");
        return;
      }

      size_t indexOfRemovedEntity = mEntityToIndexMap[entity];
      size_t indexOfLastElement = mSize - 1;
      mComponentArray[indexOfRemovedEntity] = mComponentArray[indexOfLastElement];

      Entity entityOfLastElement = mIndexToEntityMap[indexOfLastElement];
      mEntityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
      mIndexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

      mEntityToIndexMap.erase(entity);
      mIndexToEntityMap.erase(indexOfLastElement);

      --mSize;
    }

    inline T& getData(Entity entity)
    {
      if (mEntityToIndexMap.find(entity) != mEntityToIndexMap.end())
      {
        LOG_ERROR("Tried to retrieve data of non-existent entity");
        assert(true);
      }

      return mComponentArray[mEntityToIndexMap[entity]];
    }

    inline void entityDestroyed(Entity entity) override
    {
      if (mEntityToIndexMap.find(entity) != mEntityToIndexMap.end())
      {
        removeData(entity);
      }
    }
  private:
    std::array<T, MAX_ENTITIES> mComponentArray;

    std::unordered_map<Entity, size_t> mEntityToIndexMap;

    std::unordered_map<size_t, Entity> mIndexToEntityMap;

    size_t mSize;
  };

  class ComponentManager
  {
  public:
    template<typename T>
    inline void registerComponent()
    {
      const char* typeName = typeid(T).name();

      if (mComponentTypes.find(typeName) != mComponentTypes.end())
      {
        LOG_ERROR("Tried to register already registered component type - not registering anything");
        return;
      }

      mComponentTypes.insert({typeName, mNextComponentType});

      mComponentArrays.insert({typeName, reinterpret_cast<IComponentArray *>(new ComponentArray<T>())});

      ++mNextComponentType;
    }

    template<typename T>
    inline ComponentType getComponentType()
    {
      const char* typeName = typeid(T).name();

      if (mComponentTypes.find(typeName) == mComponentTypes.end())
      {
        LOG_ERROR("Tried to access unregistered component!");
        assert(false);
      }

      return mComponentTypes[typeName];
    }

    template<typename T>
    inline void addComponent(Entity entity, T component)
    {
      getComponentArray<T>()->insertData(entity, component);
    }

    template<typename T>
    inline void removeComponent(Entity entity)
    {
      getComponentArray<T>->removeData(entity);
    }

    template<typename T>
    inline T& getComponent(Entity entity)
    {
      return getComponentArray<T>()->getData(entity);
    }

    inline void entityDestroyed(Entity entity)
    {
      for(auto const& pair : mComponentArrays)
      {
        auto const& component = pair.second;
        component->entityDestroyed(entity);
      }
    }

    inline ~ComponentManager()
    {
      for (auto const& pair : mComponentArrays)
      {
        delete pair.second;
      }
    }
  private:
    std::unordered_map<const char*, ComponentType> mComponentTypes{};

    std::unordered_map<const char*, IComponentArray*> mComponentArrays{};

    ComponentType mNextComponentType{};

    template<typename T>
    ComponentArray<T>* getComponentArray()
    {
      const char* typeName = typeid(T).name();

      if(mComponentTypes.find(typeName) == mComponentTypes.end())
      {
        LOG_ERROR("Tried to use unregistered component!");
        assert(false);
      }

      // return static_cast<ComponentArray<T>*>(mComponentArrays[typeName]);
      // ComponentArray<T>* temp = mComponentArrays[typeName];
      // return temp;
      return reinterpret_cast<ComponentArray<T>*>(mComponentArrays[typeName]);
    }
  };

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

    inline Entity createEntity()
    {
      if (mLivingEntityCount >= MAX_ENTITIES)
      {
        LOG_ERROR("Tried to create new entity, when no more entities are available");
        assert(true);
      }
      Entity id = mAvailableEntities.front();
      mAvailableEntities.pop();
      ++mLivingEntityCount;
      return id;
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

    inline Signature getSignature(Entity entity)
    {
      if (entity >= MAX_ENTITIES)
      {
        LOG_ERROR("Tried to get signature of out-of-range entity");
        assert(true);
      }
      return mSignatures[entity];
    }

  private:
    std::queue<Entity> mAvailableEntities {}; // Unused entity ID's
    std::array<Signature, MAX_ENTITIES> mSignatures {}; // Signatures corresponding to Entities
    uint32_t mLivingEntityCount {};
  };

  class System
  {
  public:
    std::set<Entity> mEntities;
  };

  class TestSystem : System
  {

  };

  class SystemManager
  {
  public:
    template<typename T>
    inline T* registerSystem()
    {
      const char* typeName = typeid(T).name();

      if (mSystems.find(typeName) != mSystems.end())
      {
        LOG_ERROR("Tried registering a system multiple times!");
        assert(false);
      }

      T* system = new T();
      mSystems.insert({typeName, (System*)(system)}); // ? delete cast
      return system;
    }

    template<typename T>
    inline void setSignature(Signature signature)
    {
      const char* typeName = typeid(T).name();
      if (mSystems.find(typeName) != mSystems.end())
      {
        LOG_ERROR("Tried setting Signature for unregistered System - setting nothing");
        return;
      }

      mSignatures.insert({typeName, signature});
    }

    inline void entityDestroyed(Entity entity)
    {
      for (auto const& pair : mSystems)
      {
        auto const& system = pair.second;

        system->mEntities.erase(entity);
      }
    }

    inline void entitySignatureChanged(Entity entity, Signature signature)
    {
      for (auto const& pair : mSystems)
      {
        auto const& type = pair.first;
        auto const& system = pair.second;
        auto const& systemSignature = mSignatures[type];

        if ((signature & systemSignature) == systemSignature)
        {
          system->mEntities.insert(entity);
        }
        else
        {
          system->mEntities.erase(entity);
        }
      }
    }

    inline ~SystemManager()
    {
      for (auto const& pair : mSystems)
      {
        delete pair.second;
      }
    }
  private:
    std::unordered_map<const char*, Signature> mSignatures{};
    std::unordered_map<const char*, System*> mSystems{};
  };

  class Coordinator
  {
  public:
    inline void init()
    {
      pComponentManager = new ComponentManager();
      pEntityManager = new EntityManager();
      pSystemManager = new SystemManager();
    }

    inline Entity createEntity()
    {
      return pEntityManager->createEntity();
    }

    inline void destroyEntity(Entity entity)
    {
      pEntityManager->destroyEntity(entity);
      pComponentManager->entityDestroyed(entity);
      pSystemManager->entityDestroyed(entity);
    }

    template<typename T>
    inline void registerComponent()
    {
      pComponentManager->registerComponent<T>();
    }
    
    template<typename T>
    inline void addComponent(Entity entity, T component)
    {
      pComponentManager->addComponent<T>(entity, component);

      auto signature = pEntityManager->getSignature(entity);
      signature.set(pComponentManager->getComponentType<T>(), true);
      pEntityManager->setSignature(entity, signature);

      pSystemManager->entitySignatureChanged(entity, signature);
    }

    template<typename T>
    inline void removeComponent(Entity entity)
    {
      pComponentManager->removeComponent<T>(entity);

      auto signature = pEntityManager->getSignature(entity);
      signature.set(pComponentManager->getComponentType<T>(), false);
      pEntityManager->setSignature(entity, signature);

      pSystemManager->entitySignatureChanged(entity, signature);
    }

    template<typename T>
    inline T& getComponent(Entity entity)
    {
      return pComponentManager->getComponent<T>(entity);
    }

    template<typename T>
    inline ComponentType getComponentType()
    {
      return pComponentManager->getComponentType<T>();
    }

    template<typename T>
    inline T* registerSystem()
    {
      return pSystemManager->registerSystem<T>();
    }

    template<typename T>
    inline void setSystemSignature(Signature signature)
    {
      pSystemManager->setSignature<T>(signature);
    }

    inline ~Coordinator()
    {
      delete pComponentManager;
      delete pEntityManager;
      delete pSystemManager;
    }
  private:
    ComponentManager* pComponentManager;
    EntityManager* pEntityManager;
    SystemManager* pSystemManager;
  };
}

#endif // __ECS_BASE_H__