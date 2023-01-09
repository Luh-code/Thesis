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

  void SDLApp::initImGui()
  {
    LOG_INIT("Dear ImGui");
    // init ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    imGuiData = new Ths::Vk::ImGuiData{
      .io = ImGui::GetIO(),
    };
    
    // setup ImGui IO
    auto& imio = imGuiData->io;
    imio.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    imio.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // set ImGui theme/style
    ImGui::StyleColorsDark(); // Dark
    // ImGui::StyleColorsLight(); // Light
    // ImGui::StyleColorsClassic(); // Classic

    // setup ImGui Platform/Renderer backends
    ImGui_ImplSDL2_InitForVulkan(window);
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = vContext->instance;
    initInfo.PhysicalDevice = vContext->physicalDevice;
    initInfo.Device = vContext->device;

    Ths::Vk::QueueFamilyIndices indices = Ths::Vk::findQueueFamilies(vContext->physicalDevice, vContext->surface);
    assert(indices.isComplete());

    initInfo.QueueFamily = indices.graphicsFamily.value();
    initInfo.Queue = vContext->graphicsQueue;
    initInfo.PipelineCache = imGuiData->pipelineCache;

    VkDescriptorPoolSize poolSizes[] = {
      { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
      { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
      { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
      { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
      { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
      { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
      { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
      { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
      { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
      { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkDescriptorPoolCreateInfo poolInfo {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = static_cast<uint32_t>(sizeof(poolSizes)/sizeof(poolSizes[0])) * 1000;
    poolInfo.poolSizeCount = static_cast<uint32_t>(sizeof(poolSizes)/sizeof(poolSizes[0]));
    poolInfo.pPoolSizes = poolSizes;
    VKF(vkCreateDescriptorPool(vContext->device, &poolInfo, nullptr, &imGuiData->descriptorPool))
    {
      LOG_ERROR("An error occured whilst creating Descriptor Pool: ", res);
      return;
    }

    initInfo.DescriptorPool = imGuiData->descriptorPool; // ! <--
    initInfo.Subpass = 0;
    initInfo.MinImageCount = MAX_FRAMES_IN_FLIGHT; // ! <--
    initInfo.ImageCount = MAX_FRAMES_IN_FLIGHT;
    initInfo.MSAASamples = vContext->msaaSamples;
    initInfo.Allocator = nullptr;
    initInfo.CheckVkResultFn = Ths::Vk::imGuiCheckVkRes;
    ImGui_ImplVulkan_Init(&initInfo, vContext->renderPasses[0]);

    imio.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);

    VkCommandBuffer cmd = Ths::Vk::beginSingleTimeGraphicsCommands(vContext);
    ImGui_ImplVulkan_CreateFontsTexture(cmd);
    Ths::Vk::endSingleTimeGraphicsCommands(vContext, cmd);

    debugGui.crd = &crd;

    using dType = Ths::DebugGui::ManagedVariableDataType;
    debugGui.addManagedVariable("Lighting UBO", {
      {&ubo.ambientLight, "Light color", dType::COL3},
      {&glm::value_ptr(ubo.ambientLight)[3], "Light strength", dType::FLOAT},
      {&glm::value_ptr(ubo.specular)[0], "Spec size", dType::FLOAT},
      {&glm::value_ptr(ubo.specular)[1], "Specular strength", dType::FLOAT},
      {&ubo.lightPos, "Light pos", dType::VEC3},
    });

    LOG_INIT_OK("Dear ImGui");
  }

  void SDLApp::initEcs()
  {
    entities = std::vector<Ths::ecs::Entity>(Ths::ecs::MAX_ENTITIES);
    crd.init();

    crd.registerComponent<Ths::Vk::OContext>();
    crd.registerComponent<Ths::Vk::Mesh>();
    crd.registerComponent<Ths::Vk::Material>();
    crd.registerComponent<Ths::Vk::Transform>();
    
    renderSystem = crd.registerSystem<Ths::ecs::RenderSystem>(vContext, &crd, &debugGui);

    Ths::ecs::Signature renderSystemSignature;
    renderSystemSignature.set(crd.getComponentType<Ths::Vk::OContext>());
    renderSystemSignature.set(crd.getComponentType<Ths::Vk::Mesh>());
    renderSystemSignature.set(crd.getComponentType<Ths::Vk::Material>());
    renderSystemSignature.set(crd.getComponentType<Ths::Vk::Transform>());
    crd.setSystemSignature<Ths::ecs::RenderSystem>(renderSystemSignature);
  }

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
    Ths::Vk::createVulkanInstance(
      vContext, static_cast<uint32_t>(extensions.size()), extensions.data(),
      static_cast<uint32_t>(layers.size()), layers.data(),
      name, static_cast<uint32_t>(version),
      debug
    );
    Ths::Vk::setupDebugMessenger(vContext);
    Ths::SDL::createVkWindowSurfaceSDL(vContext->instance, window, &vContext->surface);
    VkPhysicalDeviceFeatures gpuReqirements {};
    gpuReqirements.geometryShader = true;
    gpuReqirements.tessellationShader = true;
    gpuReqirements.samplerAnisotropy = true;
    gpuReqirements.sampleRateShading = true;
    std::vector<const char*> deviceExtensions;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    Ths::Vk::selectPhysicalDevice(vContext, &gpuReqirements, &deviceExtensions);
    Ths::Vk::createLogicalDevice(vContext, &gpuReqirements, &deviceExtensions);
    int width, height;
    SDL_Vulkan_GetDrawableSize(window, &width, &height);
    Ths::Vk::createSwapChain(vContext, width, height, 2);
    Ths::Vk::createRenderPass(vContext, 0);
    Ths::Vk::createCommandPools(vContext);
    Ths::Vk::createCommandBuffers(vContext);
    Ths::Vk::createImageViews(vContext);
    Ths::Vk::createColorResources(vContext);
    Ths::Vk::createDepthResources(vContext);
    Ths::Vk::createFramebuffers(vContext);
    Ths::Vk::createUniformBuffers(vContext);
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
    // static auto startTime = std::chrono::high_resolution_clock::now();

    // auto currentTime = std::chrono::high_resolution_clock::now();
    // float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    ubo.cameraPos = glm::vec4(renderSystem->camera.translation, 1.0f);

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

    // renderSystem->recordBuffers(imageIndex);

    vkResetCommandBuffer(vContext->commandBuffers[vContext->currentFrame], 0);
    Ths::Vk::beginCommandBuffer(vContext, vContext->commandBuffers[vContext->currentFrame], imageIndex);
    renderSystem->recordBuffer(imageIndex, vContext->commandBuffers[vContext->currentFrame]);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vContext->commandBuffers[vContext->currentFrame]);
    Ths::Vk::endCommandBuffer(vContext, vContext->commandBuffers[vContext->currentFrame], imageIndex);

    VkSubmitInfo submitInfo {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    VkSemaphore waitSemaphores[] = {vContext->imageAvailableSemaphores[vContext->currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    std::vector<VkCommandBuffer> commandBuffers(1);// = renderSystem->getCommandBuffers(imageIndex);
    commandBuffers[0] = vContext->commandBuffers[vContext->currentFrame];
    submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
    submitInfo.pCommandBuffers = commandBuffers.data();
    // submitInfo.pCommandBuffers = &vContext->commandBuffers[vContext->currentFrame];

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
    // ImGui_ImplVulkan_CreateOrResizeWindow(
    //   vContext->instance,
    //   vContext->physicalDevice,
    //   vContext->device,
    // );
  }

  void SDLApp::mainLoop(bool (*func)())
  {
    while(Ths::SDL::maintainSDLWindow(window, [this](int w, int h) { this->resizeCallback(w, h); }) && func())
    {
      ImGui_ImplVulkan_NewFrame();
      ImGui_ImplSDL2_NewFrame();
      ImGui::NewFrame();
      bool showDemoWindow = false;
      bool showDebugWindow = true;
      // ImGui::ShowDemoWindow(&showDemoWindow);
      debugGui.showDebugGui(&showDebugWindow);
      ImGui::EndFrame();
      ImGui::Render();
      this->drawFrame();
    }

    vkDeviceWaitIdle(vContext->device);
  }
  
  void SDLApp::cleanup(bool dein_sdl)
  {
    // Delete Vk stuff
    // Ths::Vk::cleanupSwapChain(vContext);
    LOG_DEST_OK("SDL App \"", name, "\"");

    if (imGuiData)
    {
      ImGui_ImplVulkan_Shutdown();
      ImGui_ImplSDL2_Shutdown();
      ImGui::DestroyContext();
      vkDestroyDescriptorPool(vContext->device, imGuiData->descriptorPool, nullptr);
    }

    renderSystem->destroyAllVkObjects();

    // vkDestroySampler(vContext->device, vContext->textureSampler, nullptr);
    // vkDestroyImageView(vContext->device, vContext->textureImageView, nullptr);

    // vkDestroyImage(vContext->device, vContext->textureImage, nullptr);
    // vkFreeMemory(vContext->device, vContext->textureImageMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      vkDestroyBuffer(vContext->device, vContext->uniformBuffers[i], nullptr);
      vkFreeMemory(vContext->device, vContext->uniformBuffersMemory[i], nullptr);
    }

    // vkDestroyDescriptorPool(vContext->device, vContext->descriptorPool, nullptr);

    // vkDestroyDescriptorSetLayout(vContext->device, vContext->descriptorSetLayout, nullptr);

    // vkDestroyBuffer(vContext->device, vContext->indexBuffer, nullptr);
    // vkFreeMemory(vContext->device, vContext->indexBufferMemory, nullptr);

    // vkDestroyBuffer(vContext->device, vContext->vertexBuffer, nullptr);
    // vkFreeMemory(vContext->device, vContext->vertexBufferMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      vkDestroySemaphore(vContext->device, vContext->imageAvailableSemaphores[i], nullptr);
      vkDestroySemaphore(vContext->device, vContext->renderFinishedSemaphores[i], nullptr);
      vkDestroyFence(vContext->device, vContext->inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(vContext->device, vContext->transferPool, nullptr);
    vkDestroyCommandPool(vContext->device, vContext->commandPool, nullptr);
    // vkDestroyPipeline(vContext->device, vContext->graphicsPipeline, nullptr);
    // vkDestroyPipelineLayout(vContext->device, vContext->pipelineLayout, nullptr);
    for (auto renderPass : vContext->renderPasses)
    {
      vkDestroyRenderPass(vContext->device, renderPass, nullptr);
    }
    vkDestroyDevice(vContext->device, nullptr);
    Ths::Vk::destroyDebugUtilsMessengerEXT(vContext->instance, vContext->debugMessenger, nullptr);
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