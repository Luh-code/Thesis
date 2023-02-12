#include "../../pch.h"
#include "vulkan_base.hpp"

namespace Ths::Vk
{
  VkSampleCountFlagBits getMaxUsableSampleCount(VulkanHandle& vh)
  {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(vh.systemContext.physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    return counts & VK_SAMPLE_COUNT_64_BIT ? VK_SAMPLE_COUNT_64_BIT :
      counts & VK_SAMPLE_COUNT_32_BIT ? VK_SAMPLE_COUNT_32_BIT :
      counts & VK_SAMPLE_COUNT_16_BIT ? VK_SAMPLE_COUNT_16_BIT :
      counts & VK_SAMPLE_COUNT_8_BIT ? VK_SAMPLE_COUNT_8_BIT :
      counts & VK_SAMPLE_COUNT_4_BIT ? VK_SAMPLE_COUNT_4_BIT :
      counts & VK_SAMPLE_COUNT_2_BIT ? VK_SAMPLE_COUNT_2_BIT :
      VK_SAMPLE_COUNT_1_BIT
    ;
  }
  
  uint32_t rateDeviceSuitability(VkPhysicalDeviceProperties* pProps, VkPhysicalDeviceFeatures* pFeatures)
  {
    uint32_t score = 0;
    score += static_cast<uint32_t>(pProps->deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)*1000;
    score += static_cast<uint32_t>(pProps->deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)*500;

    score += pProps->limits.maxImageDimension2D;

    for (uint32_t i = 0; i < (sizeof(VkPhysicalDeviceFeatures)/sizeof(int)); i++) score += reinterpret_cast<uint32_t*>(pFeatures)[i];
    return score;
  }

  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
  {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    // for(const auto& queueFamily : queueFamilies)
    // {
    //   VkBool32 presentSupport = false;
    //   vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
    //   if (!indices.graphicsFamily.has_value() && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
    //   {
    //     indices.graphicsFamily = i;
    //   }
    //   else if (!indices.transferFamily.has_value() && (queueFamily.queueFlags & (VK_QUEUE_TRANSFER_BIT)) == (VK_QUEUE_TRANSFER_BIT))
    //   // else if (!indices.transferFamily.has_value() && (queueFamily.queueFlags & (VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT)) == (VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT))
    // //  if ((indices.graphicsFamily.has_value() && indices.graphicsFamily.value() != i) || queueFamilyCount > 2 &&
    // //     !indices.transferFamily.has_value() && (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
    //   {
    //     indices.transferFamily = i;
    //   }
    //   else if (!indices.presentFamily.has_value() && presentSupport == VK_TRUE)
    //   {
    //     indices.presentFamily = i;
    //   }
    //   if (indices.isComplete()) break;
    //   i++;
    // }
    for(const auto& queueFamily : queueFamilies)
    {
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
      if (!indices.graphicsFamily.has_value() && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
      {
        indices.graphicsFamily = i;
      }
      if ((indices.graphicsFamily.has_value() && indices.graphicsFamily.value() != i) || queueFamilyCount > 2 &&
        !indices.transferFamily.has_value() && (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
      {
        indices.transferFamily = i;
      }
      if (!indices.presentFamily.has_value() && presentSupport == VK_TRUE)
      {
        indices.presentFamily = i;
      }
      if (indices.isComplete()) break;
      i++;
    }
    return indices;
  }

  bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*>* pDeviceExtensions)
  {
    uint32_t extensionCount = 0;
    VK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    VK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()));

    std::set<std::string> requiredExtensions(pDeviceExtensions->begin(), pDeviceExtensions->end());

    for (const auto& extension : availableExtensions)
    {
      if (requiredExtensions.erase(extension.extensionName) > 0)
      {
        LOG_INFO("    \"", extension.extensionName, "\" - available");
      }
    }
    for (const auto& extension : requiredExtensions)
    {
      LOG_WARN("    \"", extension, "\" - unavailable");
    }

    return requiredExtensions.empty();
  }

  SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
  {
    SwapchainSupportDetails details;

    VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities));

    uint32_t formatCount;
    VK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr));

    if (formatCount != 0)
    {
      details.formats.resize(formatCount);
      VK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data()));
    }

    uint32_t presentModeCount;
    VK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr));

    if (presentModeCount != 0)
    {
      details.presentModes.resize(presentModeCount);
      VK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data()));
    }

    return details;
  }

  bool isPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, VkPhysicalDeviceProperties* pProps,
    VkPhysicalDeviceFeatures* pFeatures, VkPhysicalDeviceFeatures* pRequiredFeatures, std::vector<const char*>* pDeviceExtensions)
  {
    VK(vkGetPhysicalDeviceProperties(device, pProps));
    VK(vkGetPhysicalDeviceFeatures(device, pFeatures));

    VkBool32* pBoolRequiredFeatures = reinterpret_cast<VkBool32*>(pRequiredFeatures);
    VkBool32* pBoolFeatures = reinterpret_cast<VkBool32*>(pFeatures);
    
    for (uint32_t i = 0; i < (sizeof(VkPhysicalDeviceFeatures)/sizeof(VkBool32)); i++)
      if (pBoolRequiredFeatures[i] && !pBoolFeatures[i])
    {
      return false;
    }

    QueueFamilyIndices indices = findQueueFamilies(device, surface);

    bool extensionsSupported = checkDeviceExtensionSupport(device, pDeviceExtensions);
    bool swapchainAdequate = false;
    if (extensionsSupported)
    {
      SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
      swapchainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && swapchainAdequate; // TODO: Make more customizable (selectable which queueFamilies are required)
  }

  bool isPhysicalDeviceSuitable(VulkanHandle& vh, VkPhysicalDevice device, VkPhysicalDeviceProperties* pProps,
    VkPhysicalDeviceFeatures* pFeatures)
  {
    return isPhysicalDeviceSuitable(device, vh.renderContext.surface, pProps, pFeatures,
      vh.options.requiredFeatures, vh.options.deviceExtensions
    );
  }

  bool selectPhysicalDevice(VulkanHandle& vh)
  {
    LOG_ING("select", "Physical Device");
    uint32_t deviceCount = 0;
    VKF(vkEnumeratePhysicalDevices(vh.applicationContext.instance, &deviceCount, nullptr))
    {
      LOG_ERROR("An Error occured whilst querying for the amount of physical devices: ", res);
      LOG_ING_AB("select", "Physical Device");
      return false;
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    VKF(vkEnumeratePhysicalDevices(vh.applicationContext.instance, &deviceCount, devices.data()))
    {
      LOG_ERROR("An Error occured whilst querying for physical devices: ", res);
      LOG_ING_AB("select", "Physical Device");
      return false;
    }
    std::multimap<int, std::tuple<VkPhysicalDevice, const char*>> candidates;
    // for (std::vector<VkPhysicalDevice>::iterator i = devices.begin(); i != devices.end(); i++)
    for (const auto& i : devices)
    {
      VkPhysicalDeviceProperties props;
      VkPhysicalDeviceFeatures features;
      if (isPhysicalDeviceSuitable(vh, i, &props, &features))
      {
        uint32_t score = rateDeviceSuitability(&props, &features);
        candidates.insert(std::make_pair(score, std::make_tuple(i, props.deviceName)));
        LOG_INFO("  \"", props.deviceName, "\" - suitable (", score, ")");
      }
      else LOG_INFO("  \"", props.deviceName, "\" - not suitable");
    }
    if (candidates.rbegin()->first > 0)
    {
      vh.systemContext.physicalDevice = std::get<0>(candidates.rbegin()->second);
      VkSampleCountFlagBits maxSamples = getMaxUsableSampleCount(vh);
      if (vh.options.msaaSamples > maxSamples)
      {
        LOG_WARN(vh.options.msaaSamples, "x MSAA samples are not supported! Lowering to ", maxSamples, "x");
        vh.options.msaaSamples = maxSamples;
      }
      LOG_INFO_IV("Selected GPU: \"", std::get<1>(candidates.rbegin()->second), "\"");
    }
    else
    {
      LOG_CRIT("No suitable GPU found!");
      LOG_ING_AB("select", "Physical Device");
      return false;
    }
    LOG_ING_OK("select", "Physical Device");
    return true;
  }
} // namespace Ths::Vk
