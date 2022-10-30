#ifndef __VULKAN_BASE_H__
#define __VULKAN_BASE_H__

#include "vulkan/vulkan.h"
#include "../thesis.h"
#include "logger.h"
#include <vector>
#include <algorithm>
#include <cstring>
#include <map>
#include <optional>
#include <set>
//#include "../sdl/sdl-base.h"

#define VK_FAIL(val) if(VkResult res = (val); res != VK_SUCCESS)
#define VKF(val) VK_FAIL(val)
#define ASSERT_VULKAN(val) VKF(val) { LOG_CRIT("An error occured in an attempted vulkan call: ", ret); assert(false); }
#define VK(f) f
#define VKA(f) ASSERT_VULKAN(VK(f))

#define ARRAY_SIZE(v) (sizeof(v)/sizeof(v[0]))

namespace Ths::Vk
{
  // Vulkan Creation
  // Data

  typedef struct VulkanContext
  {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    VkPhysicalDevice physicalDevice;
    VkDevice device = VK_NULL_HANDLE;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSurfaceKHR surface;
  } VContext;

  typedef struct QueueFamilyIndices
  {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

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

  // Functions
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