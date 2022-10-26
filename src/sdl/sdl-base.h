#ifndef __SDL_BASE_H__
#define __SDL_BASE_H__

#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"
#include "logger.h"
#include <string>
#include <vector>

namespace Ths::SDL
{
    static bool sdl_initialized = false;

    bool initSDL(Uint32 flags = 0);
    bool initSDLVid(Uint32 flags = 0);
    SDL_Window* createSDLWindow(const char* title, uint32_t dx, uint32_t dy, uint32_t w, uint32_t h, Uint32 flags = 0);
    SDL_Window* createSDLWindowVk(const char* title, uint32_t dx, uint32_t dy, uint32_t w, uint32_t h, Uint32 flags = 0);
    bool destroySDLWindow(SDL_Window* wnd);
    bool quitSDL();
    bool maintainSDLWindow();
    bool querySDLVkInstanceExtensions(SDL_Window* window, std::vector<const char*>* extensions);
}

#endif // __SDL_BASE_H__