#define SDL_MAIN_HANDLED
#include "app/application.h"
#include "memory_mger.h"
int main()
{
    LOG_INFO("|------< Thesis >------|");
    Ths::SDLApp* app = new Ths::SDLApp();
    app->version = VK_MAKE_API_VERSION(1,0,0,0);
    app->run();
    return 0;
}