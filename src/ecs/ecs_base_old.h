#ifndef __ECS_BASE_H__
#define __ECS_BASE_H__

#include "../pch.h"

namespace Ths::ecs
{
  struct ComponentSpecifier
  {
    // virtual constexpr const std::string strComponentName() const;
    const std::string componentName;
    const std::string name;
    const size_t id;

    inline ComponentSpecifier(const std::string componentName, const std::string name, const size_t id)
      : componentName(componentName), name(name), id(id)
    {

    }

    constexpr const size_t hash() const
    {
      size_t res = 17;
      for (size_t i = 0; i < componentName.size(); i++)
      {
        res = res * 31 + componentName.c_str()[i];
      }
      for (size_t i = 0; i < name.size(); i++)
      {
        res = res * 31 + name.c_str()[i];
      }
      return res;
    }
  };

  struct BaseComponent
  {
    virtual constexpr const size_t getId();
    virtual constexpr void changeId(const size_t newId);
    virtual constexpr const char* getName();
    virtual constexpr const size_t getLen();
  };

  template<
    volatile size_t id,
    char const* name, const size_t len>
  struct Component : BaseComponent
  {
  public:
    virtual constexpr const std::string strBaseName() const;

    // virtual constexpr const ComponentSpecifier ComponentSpecifier() const
    // {
    //   return ComponentSpecifier(strBaseName(), name, id);
    // }

    constexpr const size_t getId() override
    {
      return id;
    }

    constexpr void changeId(const size_t newId) override
    {
      //id = newId;
    }

    constexpr const char* getName() override
    {
      return name;
    }

    constexpr const size_t getLen() override
    {
      return len;
    }

  private:
    // const std::string name;
  };

  template<
    volatile size_t id,
    char const* name, const size_t len>
  struct TestComponent : Component<id, name, len>
  {
  public:
    virtual constexpr const std::string strBaseName() const override
    {
      return std::string("TestComponent");
    }
  };

  char tname[] = "TestComp";

  struct Entity
  {
  public:
    constexpr const BaseComponent* components()
    {
      return componentsVec.data();
    }

    inline void addComponent(size_t index)
    {

    }

    template<typename T, typename... Args>
    inline void addComponent(size_t index, T& thing, Args&... things)
    {
      if (std::is_base_of<BaseComponent, T>::value == false)
      {
        LOG_ERROR("Tried to add component, which doesn't inherit BaseComponent, continuing without adding...");
        addComponent<Args...>(index, things...);
        return;
      }
      T temp {};
      components()[index] = static_cast<T>(temp);
      componentAssociations[components()[index].getName()] = index;
      addComponent<Args...>(index++, things...);
    }

    template<typename... Args>
    inline Entity(const size_t amt, Args&... args)
    {
      this->componentsVec = std::vector(amt);
      addComponent<Args...>(0);
    }

    inline Entity()
    {
      this->componentsVec = {};
      this->componentAssociations = {};
      this->componentsVec.push_back(TestComponent<0, tname, 8>());
      componentAssociations.insert({static_cast<const std::string>("TestComp"), 0});
    }

    inline const BaseComponent& operator[](const size_t idx)
    {
      if (idx >= componentsVec.size())
      {
        LOG_ERROR("Tried to access non-existent component with index \"", idx, "\", returning empty...");
        return BaseComponent();
      }
      return components()[idx];
    }

    inline const BaseComponent& operator[](const std::string name)
    {
      const size_t id = componentAssociations[name];
      return operator[](id);
    }
  private:
    std::vector<BaseComponent> componentsVec;
    std::unordered_map<const std::string, const size_t> componentAssociations;
  };
}

#endif // __ECS_BASE_H__