#ifndef __ECS_BASE_H__
#define __ECS_BASE_H__

#include "../pch.h"
#include <cmath>
using json = nlohmann::json;

// ecs built from Austin Morlan's Tutorial (https://austinmorlan.com/posts/entity_component_system/) & modded

namespace Ths::ecs
{
  using Entity = uint32_t;
  const Entity MAX_ENTITIES = 10000;

  using ComponentType = uint32_t;
  const ComponentType MAX_COMPONENTS = 1000;

  using Signature = std::bitset<MAX_COMPONENTS>;


  class IComponentArray
  {
  public:
    //virtual ~IComponentArray() = default;
    virtual void entityDestroyed(Entity entity) = 0;

    virtual json getJson() = 0;
  };

  template<typename T>
  class ComponentArray : IComponentArray
  {
  public:
    // inline ComponentArray()
    //  : mComponentArray(std::array<T, MAX_ENTITIES>{})
    // {};

    inline void insertData(Entity entity, T component)
    {
      if (mEntityToIndexMap.find(entity) != mEntityToIndexMap.end())
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
      if (mEntityToIndexMap.find(entity) == mEntityToIndexMap.end())
      {
        LOG_ERROR("Tried removing non-existent entity - removing nothing");
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
      if (mEntityToIndexMap.find(entity) == mEntityToIndexMap.end())
      {
        LOG_ERROR("Tried to retrieve data of non-existent entity");
        assert(false);
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

    inline json getJson() override
    {
      json j;
      json compJson;
      for (auto& comp : mComponentArray)
      {
        size_t size = static_cast<size_t>(std::ceil(sizeof(comp)/static_cast<float>(sizeof(unsigned int))));
        unsigned int* ints = reinterpret_cast<unsigned int*>(&comp);
        std::stringstream stream;
        stream << "0x" << std::setfill('0') << std::setw(sizeof(unsigned int)*2);
        for (int i = 0; i < size; i++)
        {
          stream << std::hex << ints[i];
        }
        compJson.push_back(stream.str());
      }
      j["mComponentArray"] = compJson;
      json etiJson;
      for (auto& eti : mEntityToIndexMap)
      {
        json temp;
        temp[std::to_string(eti.first)] = eti.second;
        etiJson.push_back(temp);
      }
      j["mEntityToIndexMap"] = etiJson;
      json iteJson;
      for (auto& ite : mIndexToEntityMap)
      {
        etiJson[ite.second] = ite.first;
      }
      j["mIndexToEntityMap"] = iteJson;
      j["mSize"] = mSize;
      return j;
    }

  public:
    std::array<T, MAX_ENTITIES> mComponentArray{};

    absl::flat_hash_map<Entity, size_t> mEntityToIndexMap{};

    absl::flat_hash_map<size_t, Entity> mIndexToEntityMap{};

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
      getComponentArray<T>()->removeData(entity);
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

    inline json getJson()
    {
      json j;
      json comptJson;
      for (auto& comp : mComponentTypes)
      {
        comptJson[comp.first] = comp.second;
      }
      j["mComponentTypes"] = comptJson;
      json compaJson;
      for (auto& comp : mComponentArrays)
      {
        compaJson[comp.first] = comp.second->getJson();
      }
      j["mComponentArrays"] = compaJson;
      j["mNextComponentType"] = mNextComponentType;
      return j;
    }

    inline ~ComponentManager()
    {
      for (auto const& pair : mComponentArrays)
      {
        delete pair.second;
      }
    }
  public:
    absl::flat_hash_map<const char*, ComponentType> mComponentTypes{};

    absl::flat_hash_map<const char*, IComponentArray*> mComponentArrays{};

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
        assert(false);
      }
      Entity id = mAvailableEntities.front();
      mAvailableEntities.pop();
      ++mLivingEntityCount;
      mExistingEntities.insert(id);
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
      mExistingEntities.erase(entity);

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

    inline json getJson()
    {
      json j;
      std::queue<Entity> q (mAvailableEntities);
      json availableJson;
      for (uint32_t i = 0; i < MAX_ENTITIES-mLivingEntityCount; i++)
      {
        availableJson.push_back(q.front());
        q.pop();
      }
      j["mAvailableEntites"] = availableJson;
      json sigsJson;
      for (auto& sig : mSignatures)
      {
        sigsJson.push_back(sig.to_string());
      }
      j["mSignatures"] = sigsJson;
      j["mLivingEntityCount"] = mLivingEntityCount;
      return j;
    }

  public:
    std::queue<Entity> mAvailableEntities {}; // Unused entity ID's
    std::set<Entity> mExistingEntities {}; // Uesd entity ID's
    std::array<Signature, MAX_ENTITIES> mSignatures {}; // Signatures corresponding to Entities
    uint32_t mLivingEntityCount {};
  };

  class System
  {
  public:
    std::set<Entity> mEntities;

    virtual void entityRegistered(Entity entity)
    {

    }

    virtual void entityErased(Entity entity)
    {

    }

    virtual json getJson()
    {
      return json {};
    }
  };

  class SystemManager
  {
  public:
    template<typename T, typename... Args>
    inline T* registerSystem(Args... args)
    {
      const char* typeName = typeid(T).name();

      if (mSystems.find(typeName) != mSystems.end())
      {
        LOG_ERROR("Tried registering a system multiple times!");
        assert(false);
      }

      T* system = new T(args...);
      mSystems.insert({typeName, (System*)(system)}); // ? delete cast
      return system;
    }

    template<typename T>
    inline void setSignature(Signature signature)
    {
      const char* typeName = typeid(T).name();
      if (mSystems.find(typeName) == mSystems.end())
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
          system->entityRegistered(entity);
        }
        else
        {
          system->mEntities.erase(entity);
          system->entityErased(entity);
        }
      }
    }

    inline json getJson()
    {
      json j;
      json sigJson;
      for (auto& sig : mSignatures)
      {
        sigJson[sig.first] = sig.second.to_string();
      }
      j["mSignatures"] = sigJson;
      json sysJson;
      for (auto& sys : mSystems)
      {
        sysJson[sys.first] = sys.second->getJson();
      }
      j["mSystems"] = sysJson;
      return j;
    }

    inline ~SystemManager()
    {
      for (auto const& pair : mSystems)
      {
        delete pair.second;
      }
    }
  public:
    absl::flat_hash_map<const char*, Signature> mSignatures{};
    absl::flat_hash_map<const char*, System*> mSystems{};
  };

  class IResourceArray
  {

  };

  template <typename T>
  class ResourceArray : IResourceArray
  {
  public:
    inline T*& getResource(const char* key)
    {
      absl::string_view view = key;
      return data[view];
    }
    inline T*& getResource(std::string key)
    {
      return getResource(key.c_str());
    }

    inline void setResource(const char* key, T* value)
    {
      absl::string_view view = key;
      data[view] = value;
    }
    inline void setResource(std::string key, T* value)
    {
      setResource(key.c_str(), value);
    }

    inline void deleteResource(const char* key)
    {
      absl::string_view view = key;
      // delete data[view];
      data.erase(view);
    }
    inline void deleteResource(std::string key)
    {
      deleteResource(key.c_str());
    }
    
    inline void deleteAll()
    {
      // for (auto& d : data)
      // {
      //   // T*& temp;
      //   if(d.second) delete d.second;
      //   // absl::string_view view = d.first;
      //   // data.erase(view);
      //   // delete temp;
      // }
      data.clear();
    }

    inline T*& operator[](const char* key)
    {
      absl::string_view view = key;
      return data[view];
    }
    inline T*& operator[](std::string key)
    {
      return this[key.c_str()];
    }
  private:
    absl::flat_hash_map<std::string, T*> data;
  };

  class ResourceManager
  {
  public:
    template<typename T>
    inline void registerResourceType()
    {
      const char* name = typeid(T).name();
      if (resourceArrays.find(name) != resourceArrays.end())
      {
        LOG_ERROR("Tried to register resource multiple times - registering nothing");
        return;
      }
      resourceArrays.insert({name, reinterpret_cast<IResourceArray*>(new ResourceArray<T>())});
      // resourceArrays[name] = reinterpret_cast<IResourceArray*>(new ResourceArray<T>());
    }

    template<typename T>
    inline T*& getResource(const char* key)
    {
      const char* name = typeid(T).name();
      return getResourceArray<T>()->getResource(key);
    }
    template<typename T>
    inline T*& getResource(std::string key)
    {
      return getResource<T>(key.c_str());
    }

    template<typename T>
    inline void setResource(const char* key, T* value)
    {
      const char* name = typeid(T).name();
      getResourceArray<T>()->setResource(key, value);
    }
    template<typename T>
    inline void setResource(std::string key, T* value)
    {
      setResource<T>(key.c_str(), value);
    }

    template<typename T>
    inline void deleteResource(const char* key)
    {
      getResourceArray<T>()->deleteResource(key);
    }
    template<typename T>
    inline void deleteResource(std::string key)
    {
      deleteResource<T>(key.c_str());
    }

    template<typename... Args>
    struct RecursiveDelete;

    template<typename First, typename... Args>
    struct RecursiveDelete<First, Args...>
    {
      static inline void deleteAll(ResourceManager& r)
      {
        r.getResourceArray<First>()->deleteAll();
        RecursiveDelete<Args...>::deleteAll(r);
      }
    };
    template<>
    struct RecursiveDelete<>
    {
      static inline void deleteAll(ResourceManager& r) { }
    };
    template<typename... Args>
    void deleteAll()
    {
      RecursiveDelete<Args...>::deleteAll(*this);
    }

    inline ~ResourceManager()
    {
      for (auto& a : resourceArrays)
      {
        delete a.second;
      }
    }
  private:
    absl::flat_hash_map<const char*, IResourceArray*> resourceArrays;

    template<typename T>
    ResourceArray<T>* getResourceArray()
    {
      const char* typeName = typeid(T).name();

      if(resourceArrays.find(typeName) == resourceArrays.end())
      {
        LOG_ERROR("Tried to use unregistered component!");
        assert(false);
      }

      // return static_cast<ComponentArray<T>*>(mComponentArrays[typeName]);
      // ComponentArray<T>* temp = mComponentArrays[typeName];
      // return temp;
      return reinterpret_cast<ResourceArray<T>*>(resourceArrays[typeName]);
    }
  };

  class Coordinator
  {
  public:
    inline void init()
    {
      pComponentManager = new ComponentManager();
      pEntityManager = new EntityManager();
      pSystemManager = new SystemManager();
      pResourceManager = new ResourceManager();
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

    template<typename T, typename... Args>
    inline T* registerSystem(Args... args)
    {
      return pSystemManager->registerSystem<T>(args...);
    }

    template<typename T>
    inline void setSystemSignature(Signature signature)
    {
      pSystemManager->setSignature<T>(signature);
    }

    template<typename T>
    inline void registerResourceType()
    {
      pResourceManager->registerResourceType<T>();
    }

    template<typename T>
    inline T*& getResource(const char* key)
    {
      return pResourceManager->getResource<T>(key);
    }
    template<typename T>
    inline T*& getResource(std::string key)
    {
      return getResource<T>(key.c_str());
    }

    template<typename T>
    inline void setResource(const char* key, T* value)
    {
      pResourceManager->setResource<T>(key, value);
    }
    template<typename T>
    inline void setResource(std::string key, T* value)
    {
      setResource<T>(key.c_str(), value);
    }

    template<typename T>
    inline void deleteResource(const char* key)
    {
      pResourceManager->deleteResource<T>(key);
    }
    template<typename T>
    inline void deleteResource(std::string key)
    {
      deleteResource<T>(key.c_str());
    }
    template<typename... Args>
    inline void deleteAllResources()
    {
      pResourceManager->deleteAll<Args...>();
    }

    inline nlohmann::json getJson()
    {
      nlohmann::json j;
      j["pComponentManager"] = pComponentManager->getJson();
      j["pEntityManager"] = pEntityManager->getJson();
      j["pSystemManager"] = pSystemManager->getJson();
      return j;
    }

    inline ~Coordinator()
    {
      delete pComponentManager;
      delete pEntityManager;
      delete pSystemManager;
      delete pResourceManager;
    }
  public:
    ComponentManager* pComponentManager;
    EntityManager* pEntityManager;
    SystemManager* pSystemManager;
    ResourceManager* pResourceManager;
  };
}

#endif // __ECS_BASE_H__