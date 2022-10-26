#include "application.h"

namespace Ths
{

// App

    void App::mainLoop(bool (*func)())
    {
        while(func());
    }
    
    void App::cleanup()
    {
        
    }

// VkApp

    void VkApp::initVulkan()
    {
        Vk::createVulkanInstance(vContext, 0, 0, 0, 0, name, version);
    }
    
    void VkApp::cleanup()
    {
        delVContext();
    }
    
    void VkApp::delVContext()
    {
        if (this->vContext) delete this->vContext;
    }
    
// SDLApp

    void SDLApp::initWindow(const char* title, uint32_t w, uint32_t h, uint32_t dx, uint32_t dy)
    {
        if (!Ths::SDL::sdl_initialized) Ths::SDL::initSDLVid();
        window = Ths::SDL::createSDLWindowVk(title, dx, dy, w, h);
    }

    void SDLApp::initVulkan()
    {
        uint32_t eCount;
        std::vector<const char*> extensions;
        Ths::SDL::querySDLVkInstanceExtensions(window, &eCount, &extensions);
        uint32_t lCount = 1;
        std::vector<const char*> layers;
        Ths::Vk::addDebugging(&layers, &extensions, false);
        //layers.push_back("VK_LAYER_KHRONOS_validation");
        //printMemoryUsage();
        Ths::Vk::checkLayerAvailability(&layers, &lCount);
        //printMemoryUsage();
        Vk::createVulkanInstance(vContext, extensions.size(), extensions.data(), layers.size(), layers.data(), name, version);
    }
    
    void SDLApp::mainLoop(bool (*func)())
    {
        while(Ths::SDL::maintainSDLWindow() && func()) {}
    }
    
    void SDLApp::cleanup(bool dein_sdl)
    {
        Ths::SDL::destroySDLWindow(window);
        if(dein_sdl && Ths::SDL::sdl_initialized) Ths::SDL::quitSDL();
        delVContext();
    }

    void SDLApp::cleanup()
    {
        cleanup(true);
    }
}