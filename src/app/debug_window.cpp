#include "../pch.h"
#include "debug_window.hpp"

namespace Ths
{
  void DebugGui::addManagedVariable(const char* name, std::vector<std::tuple<void*, std::string, ManagedVariableDataType>> data)
  {
    for (auto& d : data)
    {
      managedVariables.vars[name].add(std::get<0>(d), std::get<1>(d), std::get<2>(d));
    }
  }

  void DebugGui::showMenuBar()
  {
    if (ImGui::BeginMenuBar())
    {
      if (ImGui::BeginMenu("File"))
      {
        if (ImGui::MenuItem("N/A", "WIP")) {};
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Edit"))
      {
        if (ImGui::BeginMenu("Window options"))
        {
          ImGui::Checkbox("Show titlebar", &window.titlebar);
          ImGui::Checkbox("Show scrollbar", &window.scrollbar);
          ImGui::Checkbox("Show background", &window.background);
          ImGui::Checkbox("Bring to front", &window.bringToFront);
          ImGui::EndMenu();
        }
        ImGui::Checkbox("Show metrics", &metrics);
        if (ImGui::BeginMenu("ECS"))
        {
          ImGui::Checkbox("Hide empty Entities", &hideEntitiesWithoutComponents);
          ImGui::EndMenu();
        }
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }
  }

  void DebugGui::displayTransform(Ths::Vk::Transform& transform, [[maybe_unused]] Entity e, [[maybe_unused]] Crd* crd)
  {
    ImGui::DragFloat3(
      "Translation",
      glm::value_ptr(transform.translation),
      .01f
    );
    ImGui::DragFloat3(
      "Rotation",
      glm::value_ptr(transform.rotation),
      .01f
    );
    ImGui::DragFloat3(
      "Scale",
      glm::value_ptr(transform.scale),
      .01f
    );
  }

  void DebugGui::displayMaterial(Ths::Vk::Material& material, [[maybe_unused]] Entity e, [[maybe_unused]] Crd* crd)
  {
    ImGui::Text("Path");
    ImGui::Indent(16.0f);
    ImGui::TextColored(ImColorGrey, material.path);
    ImGui::Unindent(16.0f);

    if (crd->pComponentManager->mComponentArrays.contains(typeid(Ths::Vk::OContext).name()))
    {
      Ths::Vk::OContext& context = crd->getComponent<Ths::Vk::OContext>(e);
      ImGui::Text("Texture image");
      // ImGui::Image((ImTextureID)context.descriptorSets[0], );
    }
  }

  void DebugGui::displayMesh(Ths::Vk::Mesh& mesh, [[maybe_unused]] Entity e, [[maybe_unused]] Crd* crd)
  {
    ImGui::Text("Base path");
    ImGui::Indent(16.0f);
    ImGui::TextColored(ImColorGrey, mesh.basepath);
    ImGui::Unindent(16.0f);

    ImGui::Text("Path");
    ImGui::Indent(16.0f);
    ImGui::TextColored(ImColorGrey, mesh.path);
    ImGui::Unindent(16.0f);
  }

  void DebugGui::displayObjectContext(Ths::Vk::OContext& context, [[maybe_unused]] Entity e, [[maybe_unused]] Crd* crd)
  {
    displayComponent<Ths::Vk::Transform>(
      crd,
      crd->pComponentManager->mComponentArrays.contains(typeid(Ths::Vk::Transform).name()),
      "Transform",
      e,
      &displayTransform
    );
    // displayComponent<Ths::Vk::Material>(
    //   crd,
    //   crd->pComponentManager->mComponentArrays.contains(typeid(Ths::Vk::Material).name()),
    //   "Material",
    //   e,
    //   &displayMaterial
    // );
    // displayComponent<Ths::Vk::Mesh>(
    //   crd,
    //   crd->pComponentManager->mComponentArrays.contains(typeid(Ths::Vk::Mesh).name()),
    //   "Mesh",
    //   e,
    //   &displayMesh
    // );
  }

  template <typename T>
  void DebugGui::displayComponent(Crd* crd, bool condition, const char* treeName, Ths::ecs::Entity e, void (*displayFunc)(T&, Entity, Crd*))
  {
    if (!condition) return;
    if (!ImGui::TreeNode(treeName)) return;

    displayFunc(crd->getComponent<T>(e), e, crd);

    ImGui::TreePop();
  }

  void DebugGui::showECSControls()
  {
    if (ImGui::CollapsingHeader("ECS"))
    {
      if (!crd)
      {
        ImGui::Text("No ECS registered - no information to display");
        return;
      }
      if (ImGui::TreeNode("Entities"))
      {
        for (auto& e : crd->pEntityManager->mExistingEntities)
        {
          if (ImGui::TreeNode(std::to_string(e).c_str()))
          {
            if (ImGui::TreeNode("Formatted Data"))
            {
              displayComponent<Ths::Vk::Transform>(
                crd,
                crd->pComponentManager->mComponentArrays.contains(typeid(Ths::Vk::Transform).name()),
                "Transform",
                e,
                &displayTransform
              );
              // displayComponent<Ths::Vk::Material>(
              //   crd,
              //   crd->pComponentManager->mComponentArrays.contains(typeid(Ths::Vk::Material).name()),
              //   "Material",
              //   e,
              //   &displayMaterial
              // );
              // displayComponent<Ths::Vk::Mesh>(
              //   crd,
              //   crd->pComponentManager->mComponentArrays.contains(typeid(Ths::Vk::Mesh).name()),
              //   "Mesh",
              //   e,
              //   &displayMesh
              // );
              displayComponent<Ths::Vk::OContext>(
                crd,
                crd->pComponentManager->mComponentArrays.contains(typeid(Ths::Vk::OContext).name()),
                "ObjectContext",
                e,
                &displayObjectContext
              );
              ImGui::TreePop();
            }
            ImGui::TreePop();
          }
        }
        ImGui::TreePop();
        ImGui::Separator();
      }
      if (ImGui::TreeNode("Components"))
      {
        ImGui::TreePop();
      }
    }
  }

  void DebugGui::showVariableMonitor()
  {
    if (!ImGui::CollapsingHeader("Variable monitor")) return;
    if (ImGui::TreeNode("Constant"))
    {
      if (ImGui::TreeNode("Ints"))
      {
        for (auto& item : monitoredVariables.ints)
        {
          ImGui::DragInt(item.first, &item.second, 1.0f);//, 0, 0, "%d", ImGuiSliderFlags_NoInput);
        }
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("Floats"))
      {
        for (auto& item : monitoredVariables.floats)
        {
          ImGui::DragFloat(item.first, &item.second, 0.01f);//, 0.0f, 0.0f, "%d", ImGuiSliderFlags_NoInput);
        }
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("Vec2"))
      {
        for (auto& item : monitoredVariables.vec2s)
        {
          ImGui::DragFloat2(item.first, glm::value_ptr(item.second), 1.0f);//, 0.0f, 0.0f, "%.3f", ImGuiSliderFlags_NoInput);
        }
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("Vec3"))
      {
        for (auto& item : monitoredVariables.vec3s)
        {
          ImGui::DragFloat3(item.first, glm::value_ptr(item.second), 1.0f);//, 0.0f, 0.0f, "%.3f", ImGuiSliderFlags_NoInput);
        }
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("Vec4"))
      {
        for (auto& item : monitoredVariables.vec4s)
        {
          ImGui::DragFloat4(item.first, glm::value_ptr(item.second), 1.0f);//, 0.0f, 0.0f, "%.3f", ImGuiSliderFlags_NoInput);
        }
        ImGui::TreePop();
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Volatile"))
    {
      if (ImGui::TreeNode("Ints"))
      {
        for (auto& item : monitoredVariables.volInts)
        {
          ImGui::DragInt(item.first, item.second, 1.0f);
        }
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("Floats"))
      {
        for (auto& item : monitoredVariables.volFloats)
        {
          ImGui::DragFloat(item.first, item.second, 0.01f);
        }
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("Vec2"))
      {
        for (auto& item : monitoredVariables.volVec2s)
        {
          ImGui::DragFloat2(item.first, glm::value_ptr(*item.second), 0.01f);
        }
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("Vec3"))
      {
        for (auto& item : monitoredVariables.volVec3s)
        {
          ImGui::DragFloat3(item.first, glm::value_ptr(*item.second), .01f);
        }
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("Vec4"))
      {
        for (auto& item : monitoredVariables.volVec4s)
        {
          ImGui::DragFloat4(item.first, glm::value_ptr(*item.second), .01f);
        }
        ImGui::TreePop();
      }
      ImGui::TreePop();
    }

    if (ImGui::TreeNode("Managed"))
    {
      for (auto& var : managedVariables.vars)
      {
        if (!ImGui::TreeNode(var.first)) continue;
        for (int i = 0; i < var.second.types.size(); i++)
        {
          void* item = var.second.vars[i];
          const char* name = var.second.varNames[i].c_str();
          switch (var.second.types[i])
          {
          case ManagedVariableDataType::INT:
            ImGui::DragInt(name, reinterpret_cast<int*>(item), 1.0f);
            break;
          case ManagedVariableDataType::FLOAT:
            ImGui::DragFloat(name, reinterpret_cast<float*>(item), 0.01f);
            break;
          case ManagedVariableDataType::VEC2:
            ImGui::DragFloat2(name, glm::value_ptr(*reinterpret_cast<glm::vec2*>(item)), 0.01f);
            break;
          case ManagedVariableDataType::VEC3:
            ImGui::DragFloat3(name, glm::value_ptr(*reinterpret_cast<glm::vec3*>(item)), 0.01f);
            break;
          case ManagedVariableDataType::VEC4:
            ImGui::DragFloat4(name, glm::value_ptr(*reinterpret_cast<glm::vec4*>(item)), 0.01f);
            break;
          case ManagedVariableDataType::COL3:
            ImGui::ColorEdit3(name, glm::value_ptr(*reinterpret_cast<glm::vec3*>(item)));
            break;
          case ManagedVariableDataType::COL4:
            ImGui::ColorEdit4(name, glm::value_ptr(*reinterpret_cast<glm::vec4*>(item)));
            break;
          }
        }
        ImGui::TreePop();
      }
      ImGui::TreePop();
    }
  }

  void DebugGui::showDebugGui(bool* show)
  {
    IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context!");

    if (metrics) ImGui::ShowMetricsWindow();

    ImGuiWindowFlags windowFlags = 0;
    windowFlags |= ImGuiWindowFlags_MenuBar;
    if (!window.titlebar) windowFlags |= ImGuiWindowFlags_NoTitleBar;
    if (!window.scrollbar) windowFlags |= ImGuiWindowFlags_NoScrollbar;
    if (!window.background) windowFlags |= ImGuiWindowFlags_NoBackground;
    if (!window.bringToFront) windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x +650, viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Debug Window", show, windowFlags))
    {
      ImGui::End();
      return;
    }
    
    showMenuBar();
    showECSControls();
    showVariableMonitor();

    ImGui::End();
  }
}