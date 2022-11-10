#include "sdl_base.h"

namespace Ths::SDL
{
  bool createVkWindowSurfaceSDL(VkInstance instance, SDL_Window* pWindow, VkSurfaceKHR* pSurface)
  {
    if (SDL_Vulkan_CreateSurface(pWindow, instance, pSurface) != SDL_TRUE)
    {
      LOG_ERROR("An error occured, whilst creating a VkSurfaceKHR via SDL");
      return false;
    }
    return true;
  }

  bool initSDL(Uint32 flags)
  {
    LOG_INIT("SDL");
    if (SDL_Init(flags) != 0)
    {
      LOG_ERROR("Error initializing SDL: ", SDL_GetError());
      LOG_INIT_AB("SDL");
      return false;
    }
    sdl_initialized = true;
    LOG_INIT_OK("SDL");
    return true;
  }

  bool initSDLVid(Uint32 flags)
  {
    return initSDL(SDL_INIT_VIDEO | flags);
  }

  SDL_Window* createSDLWindow(const char* title, uint32_t dx, uint32_t dy, uint32_t w, uint32_t h, Uint32 flags)
  {
    LOG_INIT("SDL Window");
    if (!sdl_initialized)
    {
      LOG_ERROR("SDL not initialized! aborting...");
      LOG_INIT_AB("SDL Window");
      return 0;
    }

    SDL_Window* wnd = SDL_CreateWindow(title, dx, dy, w, h, flags);
    if (!wnd)
    {
      LOG_ERROR("Error createing SDL Window: ", SDL_GetError);
      LOG_INIT_AB("SDL Window");
      return 0;
    }

    LOG_INIT_OK("SDL Window");
    return wnd;
  }

  SDL_Window* createSDLWindowVk(const char* title, uint32_t dx, uint32_t dy, uint32_t w, uint32_t h, Uint32 flags)
  {
    return createSDLWindow(title, dx, dy, w, h, SDL_WINDOW_VULKAN | flags);
  }

  bool destroySDLWindow(SDL_Window* wnd)
  {
    LOG_DEST("SDL Window");
    if (!wnd)
    {
      LOG_ERROR("Cannot delete nullptr!");
      LOG_DEST_AB("SDL Window");
      return false;
    }
    SDL_DestroyWindow(wnd);
    LOG_DEST_OK("SDL Window");
    return true;
  }

  bool quitSDL()
  {
    LOG_DEIN("SDL");
    if (!sdl_initialized)
    {
      LOG_ERROR("Cannot Quit, if not initialized!");
      LOG_DEIN_AB("SDL");
      return false;
    }
    SDL_Quit();
    LOG_DEIN_OK("SDL");
    return true;
  }

  bool maintainSDLWindow(SDL_Window* window, std::function<void(int, int)> resizeCallback)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
      case SDL_QUIT:
        return false;
        break;
      case SDL_WINDOWEVENT:
        switch (event.window.event)
        {
        case SDL_WINDOWEVENT_RESIZED:
          if (resizeCallback)
          {
            int w, h;
            SDL_Vulkan_GetDrawableSize(window, &w, &h);
            resizeCallback(w, h);
            LOG_DEBUG("SDL_Window resized!");
          }
          else
          {
            LOG_DEBUG("resizeCallback not defined!");
          }
          break;
        default:
          break;
        }
        break;
      default:
        break;
      }
    }
    return true;
  }
  
  bool querySDLVkInstanceExtensions(SDL_Window* window, std::vector<const char*>* extensions)
  {
    LOG_QUER("SDL for VkInstance Extensions");

    uint32_t count = 0;
    if (!SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr))
    {
      LOG_ERROR("An Error occured whilst querying SDL for VkInstance Extension count: ", SDL_GetError);
      LOG_QUER_AB("SDL for VkInstance Extensions");
      return false;
    }

    extensions->resize(extensions->size()+count);

    if (!SDL_Vulkan_GetInstanceExtensions(window, &count, extensions->data()))
    {
      LOG_ERROR("An Error occured whilst querying SDL for VkInstance Extensions: ", SDL_GetError);
      LOG_QUER_AB("SDL for VkInstance Extensions");
      return false;
    }

    LOG_QUER_OK("SDL for VkInstance Extensions");
    return true;
  }
}