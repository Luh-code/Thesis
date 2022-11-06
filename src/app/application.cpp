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
    std::vector<const char*> extensions;
    Ths::SDL::querySDLVkInstanceExtensions(window, &extensions);
    std::vector<const char*> layers;
    Ths::Vk::addDebugging(&layers, &extensions, false);
    Ths::Vk::checkLayerAvailability(&layers);
    Ths::Vk::createVulkanInstance(vContext, extensions.size(), extensions.data(), layers.size(), layers.data(), name, version, debug);
    Ths::Vk::setupDebugMessenger(vContext);
    Ths::SDL::createVkWindowSurfaceSDL(vContext->instance, window, &vContext->surface);
    VkPhysicalDeviceFeatures gpuReqirements {};
    gpuReqirements.geometryShader = true;
    std::vector<const char*> deviceExtensions;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    Ths::Vk::selectPhysicalDevice(vContext, &gpuReqirements, &deviceExtensions);
    Ths::Vk::createLogicalDevice(vContext, &gpuReqirements, &deviceExtensions);
    int width, height;
    SDL_Vulkan_GetDrawableSize(window, &width, &height);
    Ths::Vk::createSwapChain(vContext, width, height, 2);
    Ths::Vk::createImageViews(vContext);
    Ths::Vk::createRenderPass(vContext, 0);
    Ths::Vk::createGraphicsPipeline(vContext);
  }
  
  void SDLApp::mainLoop(bool (*func)())
  {
    while(Ths::SDL::maintainSDLWindow() && func()) {}
  }
  
  void SDLApp::cleanup(bool dein_sdl)
  {
    // Delete Vk stuff
    vkDestroyPipelineLayout(vContext->device, vContext->pipelineLayout, nullptr);
    for (auto renderPass : vContext->renderPasses)
    {
      vkDestroyRenderPass(vContext->device, renderPass, nullptr);
    }
    for (auto imageView : vContext->swapchainImageViews)
    {
      vkDestroyImageView(vContext->device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(vContext->device, vContext->swapchain, nullptr);
    vkDestroyDevice(vContext->device, nullptr);
    Ths::Vk::DestroyDebugUtilsMessengerEXT(vContext->instance, vContext->debugMessenger, nullptr);
    vkDestroySurfaceKHR(vContext->instance, vContext->surface, nullptr);
    vkDestroyInstance(vContext->instance, nullptr);
    
    delVContext(); // * Do AFTER deleting all Vk related objs

    // Delete SDL stuff
    Ths::SDL::destroySDLWindow(window);
    if(dein_sdl && Ths::SDL::sdl_initialized) Ths::SDL::quitSDL();
  }

  void SDLApp::cleanup()
  {
    cleanup(true);
  }
}