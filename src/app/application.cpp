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
        
    }
    
    void VkApp::mainLoop(bool (*func)())
    {
        while (func());
    }
    
    void VkApp::cleanup()
    {
        if (this->vData) delete this->vData;
    }
    
// SDLApp

    void SDLApp::initWindow(const char* title, uint32_t w, uint32_t h, uint32_t dx, uint32_t dy)
    {
        if (!Ths::SDL::sdl_initialized) Ths::SDL::initSDLVid();
        window = Ths::SDL::createSDLWindowVk(title, dx, dy, w, h);
    }
    
    void SDLApp::initVulkan()
    {
        
    }
    
    void SDLApp::mainLoop(bool (*func)())
    {
        while(Ths::SDL::maintainSDLWindow() && func()) {}
    }
    
    void SDLApp::cleanup(bool dein_sdl)
    {
        Ths::SDL::destroySDLWindow(window);
        if(dein_sdl && Ths::SDL::sdl_initialized) Ths::SDL::quitSDL();
    }

    void SDLApp::cleanup()
    {
        cleanup(true);
    }
}