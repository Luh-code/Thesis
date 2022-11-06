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
    Ths::Vk::createFramebuffers(vContext);
    Ths::Vk::createCommandPool(vContext);
    Ths::Vk::createCommandBuffers(vContext);
    Ths::Vk::createSyncObjects(vContext);
  }
  
  void SDLApp::drawFrame()
  {
    vkWaitForFences(vContext->device, 1, &vContext->inFlightFences[vContext->currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(vContext->device, 1, &vContext->inFlightFences[vContext->currentFrame]);
    uint32_t imageIndex;
    vkAcquireNextImageKHR(vContext->device, vContext->swapchain, UINT64_MAX, vContext->imageAvailableSemaphores[vContext->currentFrame], VK_NULL_HANDLE, &imageIndex);
    vkResetCommandBuffer(vContext->commandBuffers[vContext->currentFrame], 0);
    Ths::Vk::recordCommandBuffer(vContext, vContext->commandBuffers[vContext->currentFrame], imageIndex);

    VkSubmitInfo submitInfo {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    
    VkSemaphore waitSemaphores[] = {vContext->imageAvailableSemaphores[vContext->currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vContext->commandBuffers[vContext->currentFrame];

    VkSemaphore signalSemaphores[] = {vContext->renderFinishedSemaphores[vContext->currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VKF(vkQueueSubmit(vContext->graphicsQueue, 1, &submitInfo, vContext->inFlightFences[vContext->currentFrame]))
    {
      LOG_ERROR("An error occured whilst submitting draw command buffer: ", res);
    }

    VkPresentInfoKHR presentInfo {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {vContext->swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(vContext->presentQueue, &presentInfo);
    vContext->currentFrame = (vContext->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
  }

  void SDLApp::mainLoop(bool (*func)())
  {
    while(Ths::SDL::maintainSDLWindow() && func())
    {
      this->drawFrame();
    }

    vkDeviceWaitIdle(vContext->device);
  }
  
  void SDLApp::cleanup(bool dein_sdl)
  {
    // Delete Vk stuff
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      vkDestroySemaphore(vContext->device, vContext->imageAvailableSemaphores[i], nullptr);
      vkDestroySemaphore(vContext->device, vContext->renderFinishedSemaphores[i], nullptr);
      vkDestroyFence(vContext->device, vContext->inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(vContext->device, vContext->commandPool, nullptr);
    for (auto framebuffer : vContext->swapchainFramebuffers) {
        vkDestroyFramebuffer(vContext->device, framebuffer, nullptr);
    }
    vkDestroyPipeline(vContext->device, vContext->graphicsPipeline, nullptr);
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