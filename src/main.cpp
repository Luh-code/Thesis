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
  entities[2] = crd.createEntity();

  auto& camera = app->renderSystem->camera;

  camera.fov = 60.0f;

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
  crd.addComponent(entities[0], Ths::Vk::Transform{
    .translation = glm::vec3{0.0f, -1.0f, 0.0f},
    .rotation = glm::vec3{0.0f, 0.0f, glm::radians(270.0f)},
    .scale = glm::vec3{1.0f, 1.0f, 1.0f},
  });
  crd.addComponent(entities[0], Ths::Vk::OContext{});
  
  crd.addComponent(entities[1], Ths::Vk::Mesh{
    .basepath = "D:/Projects/Thesis/assets/models/",
    .path = "Neck_Mech_Walker_by_3DHaupt-(Wavefront OBJ).obj",
  });
  crd.addComponent(entities[1], Ths::Vk::Material{
    .path = "D:/Projects/Thesis/assets/textures/walker_color.jpg",
    .vertexShader = new Ths::Vk::Shader{
      Ths::Vk::readFile("D:/Projects/Thesis/src/vk/shaders/vert.spv")
    },
    .fragmentShader = new Ths::Vk::Shader{
      Ths::Vk::readFile("D:/Projects/Thesis/src/vk/shaders/frag.spv")
    },
  });
  crd.addComponent(entities[1], Ths::Vk::Transform{
    .translation = glm::vec3{0.0f, 1.0f, 0.0f},
    .rotation = glm::vec3{glm::radians(90.0f), 0.0f, 0.0f},
    .scale = glm::vec3{0.2f, 0.2f, 0.2f},
  });
  crd.addComponent(entities[1], Ths::Vk::OContext{});

  crd.addComponent(entities[2], Ths::Vk::Mesh{
    .basepath = "D:/Projects/Thesis/assets/models/",
    .path = "Obama.obj",
  });
  crd.addComponent(entities[2], Ths::Vk::Material{
    .path = "D:/Projects/Thesis/assets/textures/Obama.png",
    .vertexShader = new Ths::Vk::Shader{
      Ths::Vk::readFile("D:/Projects/Thesis/src/vk/shaders/vert.spv")
    },
    .fragmentShader = new Ths::Vk::Shader{
      Ths::Vk::readFile("D:/Projects/Thesis/src/vk/shaders/frag.spv")
    },
  });
  crd.addComponent(entities[2], Ths::Vk::Transform{
    .translation = glm::vec3{0.0f, 0.0f, 0.0f},
    .rotation = glm::vec3{0.0f, 0.0f, 0.0f},
    .scale = glm::vec3{50.0f, 50.0f, 50.0f},
  });
  crd.addComponent(entities[2], Ths::Vk::OContext{});

  app->name = "Test App";
  app->version = VK_MAKE_API_VERSION(1,0,0,0);
  app->run();

  LOG_INFO_IV("|--------------------------------|");
  return 0;
}