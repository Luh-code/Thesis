#ifndef __VULKAN_BASE_H__
#define __VULKAN_BASE_H__

#include "../pch.h"
// #include "vulkan/vulkan.h"
// #include "../thesis.h"
// #include "logger.h"
// #include <vector>
// #include <algorithm>
// #include <cstring>
// #include <map>
// #include <optional>
// #include <set>
// #include <cstdint>
// #include <limits>
// #include <algorithm>
// #include <fstream>
// #include <glm/glm.hpp>
// #include <array>
//#include "../sdl/sdl-base.h"

#define VK_FAIL(val) if(VkResult res = (val); res != VK_SUCCESS)
#define VKF(val) VK_FAIL(val)
#define ASSERT_VULKAN(val) VKF(val) { LOG_CRIT("An error occured in an attempted vulkan call: ", ret); assert(false); }
#define VK(f) f
#define VKA(f) ASSERT_VULKAN(VK(f))

#define ARRAY_SIZE(v) (sizeof(v)/sizeof(v[0]))

#ifndef MAX_FRAMES_IN_FLIGHT
#define MAX_FRAMES_IN_FLIGHT 2
#endif

namespace Ths::Vk
{
  // Vulkan Creation
  // Data

  typedef struct Vertex
  {
    glm::vec2 pos;
    glm::vec3 color;

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
    {
      std::array<VkVertexInputAttributeDescription, 2> ad {};
      ad[0].binding = 0;
      ad[0].location = 0;
      ad[0].format = VK_FORMAT_R32G32_SFLOAT; //aka vec2
      ad[0].offset = offsetof(Vertex, pos);

      ad[1].binding = 0;
      ad[1].location = 1;
      ad[1].format = VK_FORMAT_R32G32B32_SFLOAT; //aka vec3
      ad[1].offset = offsetof(Vertex, color);

      return ad;
    }

    static VkVertexInputBindingDescription getBindingDescription()
    {
      VkVertexInputBindingDescription bd {};
      bd.binding = 0;
      bd.stride = sizeof(Vertex);
      bd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Change to VK_VERTEX_INPUT_RATE_INSTANCE for instanced rendering


      return bd;
    }
  } Vertex;

  typedef struct VulkanContext
  {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    VkPhysicalDevice physicalDevice;
    VkDevice device = VK_NULL_HANDLE;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;

    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
    std::vector<VkImageView> swapchainImageViews;

    std::vector<VkRenderPass> renderPasses;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapchainFramebuffers;

    VkCommandPool commandPool;
    VkCommandPool transferPool;
    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;
    bool framebufferResized = false;

    const std::vector<Vertex> verticies = {
      {{-0.1f, -0.6f}, {0.0f, 0.0f, 1.0f}},
      {{0.4f, 0.4f}, {1.0f, 0.0f, 0.0f}},
      {{-0.6f, 0.4f}, {0.0f, 1.0f, 0.0f}},
      {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
      {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
      {{0.1f, -0.4f}, {0.0f, 1.0f, 0.0f}},
      {{0.6f, 0.6f}, {0.0f, 0.0f, 1.0f}},
      {{-0.4f, 0.6f}, {1.0f, 0.0f, 0.0f}}
    };
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
  } VContext;

  typedef struct QueueFamilyIndices
  {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> transferFamily;

    inline bool isComplete()
    {
      uint32_t arrSize = sizeof(QueueFamilyIndices)/sizeof(std::optional<uint32_t>);
      std::optional<uint32_t>* pThisArr = reinterpret_cast<std::optional<uint32_t>*>(this);
      for (int i = 0; i < arrSize; i++)
      {
        if (!pThisArr->has_value()) return false;
        pThisArr++;
      }
      return true;
    }
  } QueueFamilyIndices;

  typedef struct SwapChainSupportDetails
  {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  } SwapChainSupportDetails;


  // Functions
  bool createSyncObjects(VContext* pContext);
  bool recordCommandBuffer(VContext* pContext, VkCommandBuffer commandBuffer, uint32_t imageIndex);
  bool createCommandBuffers(VContext* pContext);
  bool createCommandPools(VContext* pContext);

  int findMemoryType(VContext* pContext, uint32_t typeFilter, VkMemoryPropertyFlags properties);
  bool createVertexBuffer(VContext* pContext);
  bool createFramebuffers(VContext* pContext);
  bool createRenderPass(VContext* pContext, uint32_t idx);
  VkShaderModule createShaderModule(VulkanContext* pContext, const std::vector<char>& code);
  static std::vector<char> readFile(const std::string& filename);
  bool createGraphicsPipeline(VulkanContext* pContext);

  void cleanupSwapChain(VulkanContext* pContext);
  bool recreateSwapChain(VulkanContext* pContext, uint32_t win_width, uint32_t win_height, uint32_t imgs,
    VkPresentModeKHR preferredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR,
    VkFormat preferredFormat = VK_FORMAT_B8G8R8A8_SRGB , VkColorSpaceKHR preferredColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
  bool createImageViews(VulkanContext* pContext);
  bool createSwapChain(VulkanContext* pContext, uint32_t win_width, uint32_t win_height, uint32_t imgs,
    VkPresentModeKHR preferredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR,
    VkFormat preferredFormat = VK_FORMAT_B8G8R8A8_SRGB , VkColorSpaceKHR preferredColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
  VkExtent2D chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t win_width, uint32_t win_height);
  VkPresentModeKHR chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes,
    VkPresentModeKHR preferredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR);
  VkSurfaceFormatKHR chooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats,
    VkFormat preferredFormat = VK_FORMAT_B8G8R8A8_SRGB, VkColorSpaceKHR preferredColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
  bool createLogicalDevice(VulkanContext* pContext, VkPhysicalDeviceFeatures* pFeatures, std::vector<const char*>* pDeviceExtensions);
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
  uint32_t rateDeviceSuitability(VkPhysicalDeviceProperties* pProps, VkPhysicalDeviceFeatures* pFeatures);
  bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*>* pDeviceExtensions);
  bool isPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, VkPhysicalDeviceProperties* pProps,
    VkPhysicalDeviceFeatures* pFeatures, VkPhysicalDeviceFeatures* pRequiredFeatures, std::vector<const char*>* pDeviceExtensions);
  bool selectPhysicalDevice(VContext* context, VkPhysicalDeviceFeatures* pRequiredFeatures, std::vector<const char*>* pDeviceExtensions);

  void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
  VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
  VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);
  bool setupDebugMessenger(VContext* context);
  bool addDebugging(std::vector<const char*>* pLayers, std::vector<const char*>* pExtensions, bool shrink = true);
  bool queryAvailableLayers(std::vector<char*>* pLayers);
  bool checkLayerAvailability(std::vector<const char*>* pLayers);
  bool createVulkanInstance(VContext* pContext, uint32_t extensionCount, const char** ppExtensions,
    uint32_t layerCount, const char** ppLayers, const char* pAppName = "Test App", uint32_t appVersion = VK_MAKE_API_VERSION(1,0,0,0), bool debug = true);
}

#endif // __VULKAN_BASE_H__