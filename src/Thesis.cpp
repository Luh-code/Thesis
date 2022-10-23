#include "pch.h"
#include "vk/vulkan-base.h"

int main()
{
    LOG_INFO("\\\\\\\\\\\\\\\\ Thesis ////////");
    Ths::SDL::initSDLVid();
    SDL_Window* window = Ths::SDL::createSDLWindowVk("Test App", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1240, 720);

    while(Ths::SDL::maintainSDLWindow())
    {

    }

    Ths::SDL::destroySDLWindow(window);
    Ths::SDL::quitSDL();
    return 0;
}