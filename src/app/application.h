#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "../sdl/sdl-base.h"
#include "../vk/vulkan-base.h"

namespace Ths
{
    class App
    {
    public:
        virtual void mainLoop(bool (*func)());
        virtual void cleanup();
    public:
        virtual void run()
        {
            mainLoop([]() -> bool {return true;});
            cleanup();
        }
    };

    class VkApp: public App
    {
    public:
        virtual void initVulkan();
        virtual void mainLoop(bool (*func)());
        virtual void cleanup();
    public:
        Vk::VData* vData;

        virtual void run()
        {
            initVulkan();
            mainLoop([]() -> bool {return true;});
            cleanup();
        }
    };

    class SDLApp: public VkApp
    {
    public:
        virtual void initWindow(const char* title, uint32_t w, uint32_t h, uint32_t dx = SDL_WINDOWPOS_CENTERED, uint32_t dy = SDL_WINDOWPOS_CENTERED);
        virtual void initVulkan();
        virtual void mainLoop(bool (*func)());
        virtual void cleanup(bool dein_sdl);
        virtual void cleanup();
    public:
        SDL_Window* window;

        virtual void run()
        {
            initWindow("SDL App", 1240, 720);
            initVulkan();
            mainLoop([]() -> bool {return true;});
            cleanup();
        }
    };
}

#endif // __APPLICATION_H__