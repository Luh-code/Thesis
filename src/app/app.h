#ifndef __APP_H__
#define __APP_H__

#include "../vk/vulkan-base.h"

namespace Ths
{
    virtual class App
    {
    public:
        void run() = delete;
    private:
        void mainLoop() = delete;
        void cleanup() = delete;
    } = delete;

    class VkApp: public App
    {
    public:
        Vk::VData* vData;

        virtual void run()
        {
            initVulkan();
            mainLoop();
            cleanup();
        }
    private:
        virtual void initVulkan();
        virtual void mainLoop();
        virtual void cleanup();
    };

    class SDLApp: public VkApp
    {
    public:
        SDL_Window* window;

        virtual void run()
        {
            initWindow();
            initVulkan();
            mainLoop();
            cleanup();
        }
    private:
        virtual void initWindow();
        virtual void initVulkan();
        virtual void mainLoop();
        virtual void cleanup();
    };
}

#endif // __APP_H__