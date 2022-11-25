#include "../pch.h"
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
    window = Ths::SDL::createSDLWindowVk(title, dx, dy, w, h, SDL_WINDOW_RESIZABLE);
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
    gpuReqirements.tessellationShader = true;
    std::vector<const char*> deviceExtensions;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    Ths::Vk::selectPhysicalDevice(vContext, &gpuReqirements, &deviceExtensions);
    Ths::Vk::createLogicalDevice(vContext, &gpuReqirements, &deviceExtensions);
    int width, height;
    SDL_Vulkan_GetDrawableSize(window, &width, &height);
    Ths::Vk::createSwapChain(vContext, width, height, 2);
    Ths::Vk::createImageViews(vContext);
    Ths::Vk::createRenderPass(vContext, 0);
    Ths::Vk::createDescriptorSetLayout(vContext);
    Ths::Vk::createGraphicsPipeline(vContext);
    Ths::Vk::createCommandPools(vContext);
    Ths::Vk::createCommandBuffers(vContext);
    //Ths::Vk::createTextureImage(vContext);
    Ths::Vk::createVertexBuffer(vContext);
    Ths::Vk::createIndexBuffer(vContext);
    Ths::Vk::createFramebuffers(vContext);
    Ths::Vk::createUniformBuffers(vContext);
    Ths::Vk::createDescriptorPool(vContext);
    Ths::Vk::createDescriptorSets(vContext);
    Ths::Vk::createSyncObjects(vContext);
  }
  
  void SDLApp::safeRecreateSwapChain(uint32_t imgs,
    VkPresentModeKHR preferredPresentMode,
    VkFormat preferredFormat, VkColorSpaceKHR preferredColorSpace)
  {
    int win_width = 0, win_height = 0;
    SDL_Vulkan_GetDrawableSize(window, &win_width, &win_height);
    if ((SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) == SDL_WINDOW_MINIMIZED || win_width == 0 || win_height == 0)
    {
      SDL_Vulkan_GetDrawableSize(window, &win_width, &win_height);
      SDL_WaitEvent(nullptr);
    }

    Ths::Vk::recreateSwapChain(vContext, win_width, win_height, imgs, preferredPresentMode, preferredFormat, preferredColorSpace);
  }
  
  void SDLApp::updateUniformBuffer()
  {
    // TODO: Use push constants instead of uniform buffers

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    Ths::Vk::UniformBufferObject ubo {};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    float fov = 45.0f;
    ubo.proj = glm::perspective(glm::radians(fov), vContext->swapchainExtent.width / static_cast<float>(vContext->swapchainExtent.height),
      0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    memcpy(vContext->uniformBuffersMapped[vContext->currentFrame], &ubo, sizeof(ubo));
  }

  void SDLApp::drawFrame()
  {
    vkWaitForFences(vContext->device, 1, &vContext->inFlightFences[vContext->currentFrame], VK_TRUE, UINT64_MAX);
    uint32_t imageIndex;
    if (VkResult result =
      vkAcquireNextImageKHR(vContext->device, vContext->swapchain, UINT64_MAX,
      vContext->imageAvailableSemaphores[vContext->currentFrame], VK_NULL_HANDLE, &imageIndex); result == VK_ERROR_OUT_OF_DATE_KHR)
    {
      LOG_DEBUG("Rebuilding swapchain; reason: ", result);
      int width, height;
      safeRecreateSwapChain(2);
      SDL_Vulkan_GetDrawableSize(window, &width, &height);
      //Ths::Vk::recreateSwapChain(vContext, width, height, 2);
      return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
      LOG_ERROR("Error acquireing VkImage: ", result);
      return;
    }
    vkResetFences(vContext->device, 1, &vContext->inFlightFences[vContext->currentFrame]);
    
    updateUniformBuffer();

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

    if (VkResult result = vkQueuePresentKHR(vContext->presentQueue, &presentInfo); result == VK_ERROR_OUT_OF_DATE_KHR ||
      result == VK_SUBOPTIMAL_KHR || vContext->framebufferResized)
    {
      LOG_DEBUG("Rebuilding swapchain; reason: ", result);
      int width, height;
      //Ths::Vk::recreateSwapChain(vContext, width, height, 2);
      safeRecreateSwapChain(2);
      SDL_Vulkan_GetDrawableSize(window, &width, &height);
      vContext->framebufferResized = false;
    }
    else if (result != VK_SUCCESS)
    {
      LOG_ERROR("An error occured whilst presenting swap chain image: ", result, " continuing");
    }

    vContext->currentFrame = (vContext->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
  }

  void SDLApp::resizeCallback(int w, int h)
  {
    vContext->framebufferResized = true;
  }

  void SDLApp::mainLoop(bool (*func)())
  {
    while(Ths::SDL::maintainSDLWindow(window, [this](int w, int h) { this->resizeCallback(w, h); }) && func())
    {
      this->drawFrame();
    }

    vkDeviceWaitIdle(vContext->device);
  }
  
  void SDLApp::cleanup(bool dein_sdl)
  {
    // Delete Vk stuff
    Ths::Vk::cleanupSwapChain(vContext);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      vkDestroyBuffer(vContext->device, vContext->uniformBuffers[i], nullptr);
      vkFreeMemory(vContext->device, vContext->uniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(vContext->device, vContext->descriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(vContext->device, vContext->descriptorSetLayout, nullptr);

    vkDestroyBuffer(vContext->device, vContext->indexBuffer, nullptr);
    vkFreeMemory(vContext->device, vContext->indexBufferMemory, nullptr);

    vkDestroyBuffer(vContext->device, vContext->vertexBuffer, nullptr);
    vkFreeMemory(vContext->device, vContext->vertexBufferMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      vkDestroySemaphore(vContext->device, vContext->imageAvailableSemaphores[i], nullptr);
      vkDestroySemaphore(vContext->device, vContext->renderFinishedSemaphores[i], nullptr);
      vkDestroyFence(vContext->device, vContext->inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(vContext->device, vContext->transferPool, nullptr);
    vkDestroyCommandPool(vContext->device, vContext->commandPool, nullptr);
    vkDestroyPipeline(vContext->device, vContext->graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(vContext->device, vContext->pipelineLayout, nullptr);
    for (auto renderPass : vContext->renderPasses)
    {
      vkDestroyRenderPass(vContext->device, renderPass, nullptr);
    }
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