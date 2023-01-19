#define SDL_MAIN_HANDLED
#include "app/application.h"
#include "memory_mger.h"
#include "pch.h"

int main()
{
  LOG_INFO_IV("|-----------< Thesis >-----------|");
  Ths::SDLApp* app = new Ths::SDLApp();
  app->name = "Test App";
  app->version = VK_MAKE_API_VERSION(1,0,0,0);

  app->initEcs();

  auto& pContext = app->vContext;
  auto& entities = app->entities;
  auto& crd = app->crd;
  entities[0] = crd.createEntity();
  entities[1] = crd.createEntity();
  entities[2] = crd.createEntity();

  auto& camera = app->renderSystem->camera;

  camera.fov = 90.0f;
  camera.translation.x = -3.9f;

  app->init();

  crd.registerResourceType<Ths::Vk::ShaderResource>();
  crd.registerResourceType<Ths::Vk::TextureResource>();
  crd.registerResourceType<Ths::Vk::MeshResource>();

  crd.setResource("basicVShader", new Ths::Vk::ShaderResource(Ths::Vk::readFile("D:/dev/Thesis/src/vk/shaders/vert.spv")));
  crd.setResource("basicFShader", new Ths::Vk::ShaderResource(Ths::Vk::readFile("D:/dev/Thesis/src/vk/shaders/frag.spv")));

  crd.setResource("lightTexture", Ths::Vk::createTextureResource(pContext, "D:/dev/Thesis/assets/textures/Light/texture_06.png"));
  crd.setResource("darkTexture", Ths::Vk::createTextureResource(pContext, "D:/dev/Thesis/assets/textures/Dark/texture_01.png"));
  crd.setResource("redTexture", Ths::Vk::createTextureResource(pContext, "D:/dev/Thesis/assets/textures/Red/texture_08.png"));

  crd.addComponent(entities[0], Ths::Vk::Mesh{
    .basepath = "D:/dev/Thesis/assets/models/",
    .path = "TestThingy.obj",
    // .pMesh =
  });
  crd.addComponent(entities[0], Ths::Vk::Material{
    // .path = "D:/Projects/Thesis/assets/textures/viking.png",
    .pTexture = crd.getResource<Ths::Vk::TextureResource>("lightTexture"),
    .pVertexShader = crd.getResource<Ths::Vk::ShaderResource>("basicVShader"),
    .pFragmentShader = crd.getResource<Ths::Vk::ShaderResource>("basicFShader"),
  });
  crd.addComponent(entities[0], Ths::Vk::Transform{
    .translation = glm::vec3{0.0f, 0.0f, -0.25f},
    .rotation = glm::vec3{0.0f, 0.0f, 0.0f},
    .scale = glm::vec3{1.0f, 1.0f, 1.0f},
  });
  crd.addComponent(entities[0], Ths::Vk::OContext{});
  
  crd.addComponent(entities[1], Ths::Vk::Mesh{
    .basepath = "D:/dev/Thesis/assets/models/",
    .path = "Cube.obj",
  });
  crd.addComponent(entities[1], Ths::Vk::Material{
    // .path = "D:/Projects/Thesis/assets/textures/walker_color.jpg",
    .pTexture = crd.getResource<Ths::Vk::TextureResource>("darkTexture"),
    .pVertexShader = crd.getResource<Ths::Vk::ShaderResource>("basicVShader"),
    .pFragmentShader = crd.getResource<Ths::Vk::ShaderResource>("basicFShader"),
  });
  crd.addComponent(entities[1], Ths::Vk::Transform{
    .translation = glm::vec3{-0.67f, 0.47f, 0.4f},
    .rotation = glm::vec3{0.0f, 0.0f, glm::radians(35.0f)},
    .scale = glm::vec3{0.4f, 0.4f, 0.4f},
  });
  crd.addComponent(entities[1], Ths::Vk::OContext{});

  crd.addComponent(entities[2], Ths::Vk::Mesh{
    .basepath = "D:/dev/Thesis/assets/models/",
    .path = "Cube.obj",
  });
  crd.addComponent(entities[2], Ths::Vk::Material{
    // .path = "D:/Projects/Thesis/assets/textures/Obama.png",
    .pTexture = crd.getResource<Ths::Vk::TextureResource>("redTexture"),
    .pVertexShader = crd.getResource<Ths::Vk::ShaderResource>("basicVShader"),
    .pFragmentShader = crd.getResource<Ths::Vk::ShaderResource>("basicFShader"),
  });
  crd.addComponent(entities[2], Ths::Vk::Transform{
    .translation = glm::vec3{1.03f, -1.25f, 0.6f},
    .rotation = glm::vec3{0.0f, 0.0f, 0.148},
    .scale = glm::vec3{.6f, .6f, .6f},
  });
  crd.addComponent(entities[2], Ths::Vk::OContext{});

  app->run();

  Ths::Vk::destroyTextureResource(pContext, crd.getResource<Ths::Vk::TextureResource>("lightTexture"));
  Ths::Vk::destroyTextureResource(pContext, crd.getResource<Ths::Vk::TextureResource>("darkTexture"));
  Ths::Vk::destroyTextureResource(pContext, crd.getResource<Ths::Vk::TextureResource>("redTexture"));
  crd.deleteAllResources<
    Ths::Vk::TextureResource,
    Ths::Vk::ShaderResource
  >();

  app->cleanup();

  LOG_INFO_IV("|--------------------------------|");
  return 0;
}