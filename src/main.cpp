#define SDL_MAIN_HANDLED
#include "app/application.h"
#include "memory_mger.h"
#include "ecs/ecs_base.hpp"
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
  // Ths::SDLApp* app = new Ths::SDLApp();
  // app->name = "Test App";
  // app->version = VK_MAKE_API_VERSION(1,0,0,0);
  // app->run();

  crd.init();

  crd.registerComponent<Gravity>();
  crd.registerComponent<RigidBody>();
  crd.registerComponent<Transform>();

  auto physicsSystem = crd.registerSystem<PhysicsSystem>();

  Ths::ecs::Signature signature;
  signature.set(crd.getComponentType<Gravity>());
  signature.set(crd.getComponentType<RigidBody>());
  signature.set(crd.getComponentType<Transform>());

  std::vector<Ths::ecs::Entity> entities(Ths::ecs::MAX_ENTITIES);

  entities[0] = crd.createEntity();
  entities[1] = crd.createEntity();

  crd.addComponent(entities[0], Gravity{glm::vec3{0.0f, 1.0f, 0.0f}});
  crd.addComponent(entities[0], RigidBody{glm::vec3{1.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 1.0f}});
  crd.addComponent(entities[0], Transform{glm::vec3{}, glm::vec3{}, glm::vec3{}});

  crd.addComponent(entities[1], Gravity{glm::vec3{0.0f, 1.0f, 0.0f}});
  crd.addComponent(entities[1], RigidBody{glm::vec3{1.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 1.0f}});
  crd.addComponent(entities[1], Transform{glm::vec3{}, glm::vec3{}, glm::vec3{}});

  while (true)
  {
    physicsSystem->update(0.3f);
  }

  LOG_INFO_IV("|--------------------------------|");
  return 0;
}