#ifndef __SDL_BASE_H__
#define __SDL_BASE_H__

#include "SDL.h"
#include "SDL_vulkan.h"
#include "logger.h"
#include <string>
#include <vector>
#include <functional>
#include "../vk/vulkan_base.h"

namespace Ths::SDL
{
  static bool sdl_initialized = false;

  bool createVkWindowSurfaceSDL(VkInstance instance, SDL_Window* pWindow, VkSurfaceKHR* pSurface);
  bool initSDL(Uint32 flags = 0);
  bool initSDLVid(Uint32 flags = 0);
  SDL_Window* createSDLWindow(const char* title, uint32_t dx, uint32_t dy, uint32_t w, uint32_t h, Uint32 flags = 0);
  SDL_Window* createSDLWindowVk(const char* title, uint32_t dx, uint32_t dy, uint32_t w, uint32_t h, Uint32 flags = 0);
  bool destroySDLWindow(SDL_Window* wnd);
  bool quitSDL();
  bool maintainSDLWindow(SDL_Window* window, std::function<void(int, int)> resizeCallback);
  bool querySDLVkInstanceExtensions(SDL_Window* window, std::vector<const char*>* extensions);
}

#endif // __SDL_BASE_H__