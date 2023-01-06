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
  entities[3] = crd.createEntity();

  auto& camera = app->renderSystem->camera;

  camera.fov = 60.0f;

  app->init();

  crd.registerResource<Ths::Vk::ShaderResource>();
  crd.registerResource<Ths::Vk::TextureResource>();

  crd.setResource("basicVShader", new Ths::Vk::ShaderResource(Ths::Vk::readFile("D:/Projects/Thesis/src/vk/shaders/vert.spv")));
  crd.setResource("basicFShader", new Ths::Vk::ShaderResource(Ths::Vk::readFile("D:/Projects/Thesis/src/vk/shaders/frag.spv")));

  crd.setResource("vikingTexture", Ths::Vk::createTextureResource(pContext, "D:/Projects/Thesis/assets/textures/viking.png"));
  crd.setResource("mechTexture", Ths::Vk::createTextureResource(pContext, "D:/Projects/Thesis/assets/textures/walker_color.jpg"));
  crd.setResource("obamaTexture", Ths::Vk::createTextureResource(pContext, "D:/Projects/Thesis/assets/textures/Obama.png"));

  crd.addComponent(entities[0], Ths::Vk::Mesh{
    .basepath = "D:/Projects/Thesis/assets/models/",
    .path = "viking.obj",
  });
  crd.addComponent(entities[0], Ths::Vk::Material{
    .path = "D:/Projects/Thesis/assets/textures/viking.png",
    .pTexture = crd.getResource<Ths::Vk::TextureResource>("vikingTexture"),
    .pVertexShader = crd.getResource<Ths::Vk::ShaderResource>("basicVShader"),
    .pFragmentShader = crd.getResource<Ths::Vk::ShaderResource>("basicFShader"),
  });
  crd.addComponent(entities[0], Ths::Vk::Transform{
    .translation = glm::vec3{0.0f, -1.0f, 0.0f},
    .rotation = glm::vec3{0.0f, 0.0f, glm::radians(270.0f)},
    .scale = glm::vec3{1.0f, 1.0f, 1.0f},
  });
  crd.addComponent(entities[0], Ths::Vk::OContext{});

  crd.addComponent(entities[3], Ths::Vk::Mesh{
    .basepath = "D:/Projects/Thesis/assets/models/",
    .path = "viking.obj",
  });
  crd.addComponent(entities[3], Ths::Vk::Material{
    .path = "D:/Projects/Thesis/assets/textures/viking.png",
    .pTexture = crd.getResource<Ths::Vk::TextureResource>("vikingTexture"),
    .pVertexShader = crd.getResource<Ths::Vk::ShaderResource>("basicVShader"),
    .pFragmentShader = crd.getResource<Ths::Vk::ShaderResource>("basicFShader"),
  });
  crd.addComponent(entities[3], Ths::Vk::Transform{
    .translation = glm::vec3{0.0f, -1.0f, 1.0f},
    .rotation = glm::vec3{0.0f, 0.0f, glm::radians(270.0f)},
    .scale = glm::vec3{1.0f, 1.0f, 1.0f},
  });
  crd.addComponent(entities[3], Ths::Vk::OContext{});
  
  crd.addComponent(entities[1], Ths::Vk::Mesh{
    .basepath = "D:/Projects/Thesis/assets/models/",
    .path = "Neck_Mech_Walker_by_3DHaupt-(Wavefront OBJ).obj",
  });
  crd.addComponent(entities[1], Ths::Vk::Material{
    .path = "D:/Projects/Thesis/assets/textures/walker_color.jpg",
    .pTexture = crd.getResource<Ths::Vk::TextureResource>("mechTexture"),
    .pVertexShader = crd.getResource<Ths::Vk::ShaderResource>("basicVShader"),
    .pFragmentShader = crd.getResource<Ths::Vk::ShaderResource>("basicFShader"),
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
    .pTexture = crd.getResource<Ths::Vk::TextureResource>("obamaTexture"),
    .pVertexShader = crd.getResource<Ths::Vk::ShaderResource>("basicVShader"),
    .pFragmentShader = crd.getResource<Ths::Vk::ShaderResource>("basicFShader"),
  });
  crd.addComponent(entities[2], Ths::Vk::Transform{
    .translation = glm::vec3{0.0f, 0.0f, 0.0f},
    .rotation = glm::vec3{0.0f, 0.0f, 0.0f},
    .scale = glm::vec3{50.0f, 50.0f, 50.0f},
  });
  crd.addComponent(entities[2], Ths::Vk::OContext{});

  app->run();

  Ths::Vk::destroyTextureResource(pContext, crd.getResource<Ths::Vk::TextureResource>("vikingTexture"));
  Ths::Vk::destroyTextureResource(pContext, crd.getResource<Ths::Vk::TextureResource>("mechTexture"));
  Ths::Vk::destroyTextureResource(pContext, crd.getResource<Ths::Vk::TextureResource>("obamaTexture"));
  crd.deleteResource<Ths::Vk::TextureResource>("vikingTexture");
  crd.deleteResource<Ths::Vk::TextureResource>("mechTexture");
  crd.deleteResource<Ths::Vk::TextureResource>("obamaTexture");

  app->cleanup();

  LOG_INFO_IV("|--------------------------------|");
  return 0;
}