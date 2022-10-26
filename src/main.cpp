#define SDL_MAIN_HANDLED
//#include "pch.h"
//#include "vk/vulkan-base.h"
//#include "sdl/sdl-base.h"
#include "app/application.h"
#include "memory_mger.h"

/*inline void printMemoryUsage() override
{
    LOG_DEBUG("Current Heap Memory Usage: ", memoryMetrics.currentUsage()," Bytes");
}*/

int main()
{
    LOG_INFO("|------< Thesis >------|");
    Ths::SDLApp* app = new Ths::SDLApp();
    //app->name = static_cast<const char*>("TestApp");
    app->version = VK_MAKE_API_VERSION(1,0,0,0);
    app->run();
    //app->initWindow("Test App", 1240, 720);
    //app->mainLoop([]()->bool{return true;});

    /*Ths::SDL::initSDLVid();
    SDL_Window* window = Ths::SDL::createSDLWindowVk("Test App", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1240, 720);

    while(Ths::SDL::maintainSDLWindow())
    {

    }

    Ths::SDL::destroySDLWindow(window);
    Ths::SDL::quitSDL();*/
    return 0;
}