#define SDL_MAIN_HANDLED
#include "app/application.h"
#include "memory_mger.h"
#include "ecs/ecs_base.hpp"
#include "ecs/rendering_ecs.hpp"
#include "pch.h"

int main()
{
  LOG_INFO_IV("|-----------< Thesis >-----------|");
  Ths::SDLApp* app = new Ths::SDLApp();

  app->initEcs();

  auto& pContext = app->vContext;
  auto& entities = app->entities;
  auto& crd = app->crd;
  entities[0] = crd.createEntity();
  entities[1] = crd.createEntity();

  crd.addComponent(entities[0], Ths::Vk::Mesh{
    .basepath = "D:/Projects/Thesis/assets/models/",
    .path = "viking.obj",
  });
  crd.addComponent(entities[0], Ths::Vk::Material{
    .path = "D:/Projects/Thesis/assets/textures/viking.png",
    .vertexShader = new Ths::Vk::Shader{
      Ths::Vk::readFile("D:/Projects/Thesis/src/vk/shaders/vert.spv")
    },
    .fragmentShader = new Ths::Vk::Shader{
      Ths::Vk::readFile("D:/Projects/Thesis/src/vk/shaders/frag.spv")
    },
  });
  crd.addComponent(entities[0], Ths::Vk::OContext{
    .material = &crd.getComponent<Ths::Vk::Material>(entities[0]),
    .mesh = &crd.getComponent<Ths::Vk::Mesh>(entities[0]),
  });
  // crd.addComponent(entities[0], Ths::Vk::Transform{
  //   .position = glm::vec3{0.0f, 0.0f, 0.0f},
  //   .rotation = glm::vec3{0.0f, 0.0f, 0.0f},
  //   .scale = glm::vec3{1.0f, 1.0f, 1.0f},
  // });

  crd.addComponent(entities[1], Ths::Vk::Mesh{
    .basepath = "D:/Projects/Thesis/assets/models/",
    .path = "AK47.obj",
  });
  crd.addComponent(entities[1], Ths::Vk::Material{
    .path = "D:/Projects/Thesis/assets/textures/AK47.png",
    .vertexShader = new Ths::Vk::Shader{
      Ths::Vk::readFile("D:/Projects/Thesis/src/vk/shaders/vert.spv")
    },
    .fragmentShader = new Ths::Vk::Shader{
      Ths::Vk::readFile("D:/Projects/Thesis/src/vk/shaders/frag.spv")
    },
  });
  crd.addComponent(entities[1], Ths::Vk::OContext{
    .material = &crd.getComponent<Ths::Vk::Material>(entities[1]),
    .mesh = &crd.getComponent<Ths::Vk::Mesh>(entities[1]),
  });
  // crd.addComponent(entities[1], Ths::Vk::Transform{
  //   .position = glm::vec3{0.0f, 0.0f, 0.0f},
  //   .rotation = glm::vec3{0.0f, 0.0f, 0.0f},
  //   .scale = glm::vec3{100.0f, 100.0f, 100.0f},
  // });

  app->name = "Test App";
  app->version = VK_MAKE_API_VERSION(1,0,0,0);
  app->run();

  LOG_INFO_IV("|--------------------------------|");
  return 0;
}