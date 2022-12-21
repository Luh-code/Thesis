#define SDL_MAIN_HANDLED
#include "app/application.h"
#include "memory_mger.h"
#include "ecs/ecs_base.hpp"
#include "pch.h"

class TestComponent
{
  int test = 69;
};

int main()
{
  LOG_INFO_IV("|-----------< Thesis >-----------|");
  // Ths::SDLApp* app = new Ths::SDLApp();
  // app->name = "Test App";
  // app->version = VK_MAKE_API_VERSION(1,0,0,0);
  // app->run();

  Ths::ecs::Coordinator crd {};
  crd.init();

  auto testSystem = crd.registerSystem<Ths::ecs::TestSystem>();

  Ths::ecs::Signature sig;
  sig.set(crd.getComponentType<TestComponent>());

  std::vector<Ths::ecs::Entity> entities(Ths::ecs::MAX_ENTITIES);

  entities[0] = crd.createEntity();
  crd.addComponent(entities[0], TestComponent{});



  LOG_INFO_IV("|--------------------------------|");
  return 0;
}