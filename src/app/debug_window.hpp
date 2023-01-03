#ifndef __DEBUG_WINDOW_H__
#define __DEBUG_WINDOW_H__

#include "../pch.h"
#include "../vk/vulkan_base.h"
#include "../ecs/ecs_base.hpp"

namespace Ths
{
  class DebugGui
  {
  public:
    using Entity = Ths::ecs::Entity;
    using Crd = Ths::ecs::Coordinator;
    
#define ImColorGrey ImVec4(.4f, .4f, .4f, 1.0f)

    enum ManagedVariableDataType
    {
      INT = 0,
      FLOAT = 1,
      VEC2 = 2,
      VEC3 = 3,
      VEC4 = 4
    };

    Crd* crd;

    inline void addMonitoredVariable(const char* name, int val) {
      monitoredVariables.ints[name] = val;
    }
    inline void addMonitoredVariable(const char* name, float val) {
      monitoredVariables.floats[name] = val;
    }
    inline void addMonitoredVariable(const char* name, glm::vec2 val) {
      monitoredVariables.vec2s[name] = val;
    }
    inline void addMonitoredVariable(const char* name, glm::vec3 val) {
      monitoredVariables.vec3s[name] = val;
    }
    inline void addMonitoredVariable(const char* name, glm::vec4 val) {
      monitoredVariables.vec4s[name] = val;
    }

    inline void removeMonitoredInt(const char* name) {
      if (monitoredVariables.ints.find(name) == monitoredVariables.ints.end()) return;
      monitoredVariables.ints.erase(name);
    }
    inline void removeMonitoredFloat(const char* name) {
      if (monitoredVariables.floats.find(name) == monitoredVariables.floats.end()) return;
      monitoredVariables.floats.erase(name);
    }
    inline void removeMonitoredVec2(const char* name) {
      if (monitoredVariables.vec2s.find(name) == monitoredVariables.vec2s.end()) return;
      monitoredVariables.vec2s.erase(name);
    }
    inline void removeMonitoredVec3(const char* name) {
      if (monitoredVariables.vec3s.find(name) == monitoredVariables.vec3s.end()) return;
      monitoredVariables.vec3s.erase(name);
    }
    inline void removeMonitoredVec4(const char* name) {
      if (monitoredVariables.vec4s.find(name) == monitoredVariables.vec4s.end()) return;
      monitoredVariables.vec4s.erase(name);
    }

    void addManagedVariable(const char* name, std::vector<std::tuple<void*, std::string, ManagedVariableDataType>> data);

    void showMenuBar();
    static void displayMaterial(Ths::Vk::Material& material, [[maybe_unused]] Entity e, [[maybe_unused]] Crd* crd);
    static void displayTransform(Ths::Vk::Transform& transform, [[maybe_unused]] Entity e, [[maybe_unused]] Crd* crd);
    static void displayMesh(Ths::Vk::Mesh& mesh, [[maybe_unused]] Entity e, [[maybe_unused]] Crd* crd);
    static void displayObjectContext(Ths::Vk::OContext& context, [[maybe_unused]] Entity e, [[maybe_unused]] Crd* crd);
    template <typename T>
    static void displayComponent(Crd* crd, bool condition, const char* treeName, Entity e, void (*displayFunc)(T&, Entity, Crd*));
    void showECSControls();
    void showVariableMonitor();

    void showDebugGui(bool* show);

  private:
    bool metrics = true;
    bool hideEntitiesWithoutComponents = true;
    bool components = false;
    bool systems = false;
    bool variableMonitor = true;
    bool console = false;

    struct MonitoredVariables
    {
      std::map<const char*, int> ints;
      std::map<const char*, float> floats;
      std::map<const char*, glm::vec2> vec2s;
      std::map<const char*, glm::vec3> vec3s;
      std::map<const char*, glm::vec4> vec4s;
      std::map<const char*, int*> volInts;
      std::map<const char*, float*> volFloats;
      std::map<const char*, glm::vec2*> volVec2s;
      std::map<const char*, glm::vec3*> volVec3s;
      std::map<const char*, glm::vec4*> volVec4s;
    } monitoredVariables;

    struct ManagedVariable
    {
      std::vector<void*> vars;
      std::vector<std::string> varNames;
      std::vector<ManagedVariableDataType> types;

      inline void add(void* var, std::string name, ManagedVariableDataType type)
      {
        vars.push_back(var);
        varNames.push_back(name);
        types.push_back(type);
      }
    };
    struct ManagedVariables
    {
      std::map<const char*, ManagedVariable> vars;
    } managedVariables;

    struct Window
    {
      bool titlebar = true;
      bool scrollbar = true;
      bool background = true;
      bool bringToFront = true;
    } window;
  };
}


#endif // __DEBUG_WINDOW_H__