#define SDL_MAIN_HANDLED
#include "app/application.h"
#include "memory_mger.h"
#include "ecs/ecs_base.hpp"
#include "ecs/rendering_ecs.hpp"
#include "pch.h"

struct Gravity
{
  glm::vec3 force;
};

struct RigidBody
{
  glm::vec3 velocity;
  glm::vec3 acceleration;
};

struct Transform
{
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;
};

Ths::ecs::Coordinator crd{};

class PhysicsSystem : Ths::ecs::System
{
public:
  void update(float dt)
  {
    for (int i = 0; i < mEntities.size(); i++)
    {
      LOG_DEBUG("Update called for registered entity ", i, " with value ", dt);
    }
  }
};

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
    .path = "viking.obj"
  });
  crd.addComponent(entities[0], Ths::Vk::Material{
    .path = "D:/Projects/Thesis/assets/textures/viking.png",
    // .vertexShader = new Ths::Vk::Shader{Ths::Vk::readFile("D:/Projects/Thesis/src/vk/shaders/vert.spv")},
    // .fragmentShader = new Ths::Vk::Shader{Ths::Vk::readFile("D:/Projects/Thesis/src/vk/shaders/frag.spv")},
  });
  crd.addComponent(entities[0], Ths::Vk::OContext{
    .material = &crd.getComponent<Ths::Vk::Material>(entities[0]),
    .mesh = &crd.getComponent<Ths::Vk::Mesh>(entities[0]),
  });
  // Ths::Vk::loadModel(
  //   pContext,
  //   crd.getComponent<Ths::Vk::OContext>(entities[0]),
  //   "D:/Projects/Thesis/assets/models/viking.obj",
  //   (Ths::Vk::BASE_PATH + "D:/Projects/Thesis/assets/models").c_str()
  // );
  // Ths::Vk::createTextureImage(
  //   pContext,
  //   crd.getComponent<Ths::Vk::OContext>(entities[0]),
  //   "D:/Projects/Thesis/assets/textures/viking.png"
  // );

  crd.addComponent(entities[1], Ths::Vk::Mesh{
    .basepath = "D:/Projects/Thesis/assets/models/",
    .path = "Obama.obj"
  });
  crd.addComponent(entities[1], Ths::Vk::Material{
    .path = "D:/Projects/Thesis/assets/textures/Obama.png",
    // .vertexShader = new Ths::Vk::Shader{Ths::Vk::readFile("D:/Projects/Thesis/src/vk/shaders/vert.spv")},
    // .fragmentShader = new Ths::Vk::Shader{Ths::Vk::readFile("D:/Projects/Thesis/src/vk/shaders/frag.spv")},
  });
  crd.addComponent(entities[1], Ths::Vk::OContext{
    .material = &crd.getComponent<Ths::Vk::Material>(entities[1]),
    .mesh = &crd.getComponent<Ths::Vk::Mesh>(entities[1]),
  });
  // Ths::Vk::loadModel(
  //   pContext,
  //   crd.getComponent<Ths::Vk::OContext>(entities[1]),
  //   "D:/Projects/Thesis/assets/models/Obama.obj",
  //   (Ths::Vk::BASE_PATH + "D:/Projects/Thesis/assets/models").c_str()
  // );
  // Ths::Vk::createTextureImage(
  //   pContext,
  //   crd.getComponent<Ths::Vk::OContext>(entities[1]),
  //   "D:/Projects/Thesis/assets/textures/Obama.png"
  // );

  // app->renderSystem->initEntities();

  app->name = "Test App";
  app->version = VK_MAKE_API_VERSION(1,0,0,0);
  app->run();

  // crd.init();

  // crd.registerComponent<Gravity>();
  // crd.registerComponent<RigidBody>();
  // crd.registerComponent<Transform>();

  // auto physicsSystem = crd.registerSystem<PhysicsSystem>();

  // Ths::ecs::Signature signature;
  // signature.set(crd.getComponentType<Gravity>());
  // signature.set(crd.getComponentType<RigidBody>());
  // signature.set(crd.getComponentType<Transform>());
  // crd.setSystemSignature<PhysicsSystem>(signature);

  // std::vector<Ths::ecs::Entity> entities(Ths::ecs::MAX_ENTITIES);

  // entities[0] = crd.createEntity();
  // entities[1] = crd.createEntity();

  // crd.addComponent(entities[0], Gravity{glm::vec3{0.0f, 1.0f, 0.0f}});
  // crd.addComponent(entities[0], RigidBody{glm::vec3{1.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 1.0f}});
  // crd.addComponent(entities[0], Transform{glm::vec3{}, glm::vec3{}, glm::vec3{}});

  // crd.addComponent(entities[1], Gravity{glm::vec3{0.0f, 1.0f, 0.0f}});
  // crd.addComponent(entities[1], RigidBody{glm::vec3{1.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 1.0f}});
  // crd.addComponent(entities[1], Transform{glm::vec3{}, glm::vec3{}, glm::vec3{}});

  // while (true)
  // {
  //   physicsSystem->update(0.3f);
  // }

  LOG_INFO_IV("|--------------------------------|");
  return 0;
}