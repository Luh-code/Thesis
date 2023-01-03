#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "../sdl/sdl_base.h"
#include "../vk/vulkan_base.h"
#include "../memory_mger.h"
#include "../ecs/ecs_base.hpp"
#include "../ecs/rendering_ecs.hpp"
#include "debug_window.hpp"

namespace Ths
{
  class BaseApp
  {
  public:
    const char*  name;
    uint32_t version;

    inline BaseApp()
     : name("Application"), version(VK_MAKE_API_VERSION(1,0,0,0)) {}
    inline BaseApp(std::string name, uint32_t version)
     : name(name.c_str()), version(version) {}
    inline BaseApp(const char* name, uint32_t version)
     : name(name), version(version) {}
  };

  class App: public BaseApp
  {
  public:
    Ths::ecs::Coordinator crd{};
    std::vector<Ths::ecs::Entity> entities;

    /*inline App(std::string name, uint32_t version)
     : BaseApp::name(name.c_str()), version(version) {}
    App(const char* name, uint32_t version)
     : BaseApp::name(name), version(version) {}*/
    virtual void initEcs() = 0;
    virtual inline void run()
    {
      mainLoop([]() -> bool {return true;});
      cleanup();
    }
    virtual void mainLoop(bool (*func)());
    virtual void cleanup();
  };

  class VkApp: public App
  {
  public:
    Vk::VContext* vContext = new Vk::VContext();
    bool debug = true;

    virtual inline void run()
    {
      LOG_INIT("Vk App \"", name, "\"");
      initVulkan();
      LOG_INIT_OK("Vk App \"", name, "\"");
      mainLoop([]() -> bool {return true;});
      LOG_DEST("Vk App \"", name, "\"");
      cleanup();
      LOG_DEST_OK("Vk App \"", name, "\"");
    }
    virtual void initVulkan();
    //virtual void mainLoop(bool (*func)());
    virtual void cleanup() override;
    void delVContext();
  };


  class SDLApp: public VkApp
  {
  public:
    SDL_Window* window;

    Ths::ecs::RenderSystem* renderSystem;
    Ths::Vk::ImGuiData* imGuiData;
    Ths::DebugGui debugGui {};

    inline SDLApp() {};
    virtual inline void run()
    {
      LOG_INIT("SDL App \"", name, "\"");
      // initWindow(name, 800, 600);
      initWindow(name, 1600, 900);
      initVulkan();
      renderSystem->initEntities();
      initImGui();
      LOG_INIT_OK("SDL App \"", name, "\"");
      mainLoop([]() -> bool {return true;});
      LOG_DEST("SDL App \"", name, "\"");
      cleanup();
      LOG_DEST_OK("SDL App \"", name, "\"");
    }
    virtual void initImGui();
    virtual void initEcs() override;
    virtual void initWindow(const char* title, uint32_t w, uint32_t h, uint32_t dx = SDL_WINDOWPOS_CENTERED, uint32_t dy = SDL_WINDOWPOS_CENTERED);
    virtual void initVulkan() override;
    virtual void safeRecreateSwapChain(uint32_t imgs,
      VkPresentModeKHR preferredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR,
      VkFormat preferredFormat = VK_FORMAT_B8G8R8A8_SRGB , VkColorSpaceKHR preferredColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    virtual void updateUniformBuffer();
    virtual void drawFrame();
    virtual void resizeCallback(int w, int h);
    virtual void mainLoop(bool (*func)()) override;
    virtual void cleanup(bool dein_sdl);
    virtual void cleanup() override;
  };
}

#endif // __APPLICATION_H__