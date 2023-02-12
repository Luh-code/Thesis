#ifndef __VULKAN_BASE_H__
#define __VULKAN_BASE_H__

#include "vulkan_types.hpp"

#define VK_FAIL(val) if(VkResult res = (val); res != VK_SUCCESS)
#define VKF(val) VK_FAIL(val)
#define ASSERT_VULKAN(val) VKF(val) { LOG_CRIT("An error occured in an attempted vulkan call: ", ret); assert(false); }
#define VK(f) f
#define VKA(f) ASSERT_VULKAN(VK(f))

#define ARRAY_SIZE(v) (sizeof(v)/sizeof(v[0]))

namespace Ths::Vk
{

  //----- Devices -----
  VkSampleCountFlagBits getMaxUsableSampleCount(VulkanHandle& vh);
  uint32_t rateDeviceSuitability(VkPhysicalDeviceProperties* pProps, VkPhysicalDeviceFeatures* pFeatures);
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
  SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
  bool isPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, VkPhysicalDeviceProperties* pProps,
    VkPhysicalDeviceFeatures* pFeatures, VkPhysicalDeviceFeatures* pRequiredFeatures,
    std::vector<const char*>* pDeviceExtensions);
  bool isPhysicalDeviceSuitable(VulkanHandle& vh, VkPhysicalDevice device, VkPhysicalDeviceProperties* pProps,
    VkPhysicalDeviceFeatures* pFeatures);
  bool selectPhysicalDevice(VulkanHandle& vh);

  //----- ApplicationContext -----

  //----- RenderContext -----

  //----- SceneContext -----

}

#endif // __VULKAN_BASE_H__