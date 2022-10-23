#ifndef __SDL_BASE_H__
#define __SDL_BASE_H__

#include "SDL2/SDL.h"
#include "logger.h"
#include <string>

namespace Ths::SDL
{
    static bool sdl_initialized = false;

    bool initSDL(Uint32 flags = 0)
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

    bool initSDLVid(Uint32 flags = 0)
    {
        return initSDL(SDL_INIT_VIDEO | flags);
    }

    SDL_Window* createSDLWindow(const char* title, uint32_t dx, uint32_t dy, uint32_t w, uint32_t h, Uint32 flags = 0)
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

    SDL_Window* createSDLWindowVk(const char* title, uint32_t dx, uint32_t dy, uint32_t w, uint32_t h, Uint32 flags = 0)
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

    bool maintainSDLWindow()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                return false;
            }
        }
        return true;
    }
}


#endif // __SDL_BASE_H__