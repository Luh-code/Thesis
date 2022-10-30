#include "vulkan_base.h"

namespace Ths::Vk
{
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
  {
    SwapChainSupportDetails details;

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

  bool createLogicalDevice(VulkanContext* pContext, VkPhysicalDeviceFeatures* pFeatures, std::vector<const char*>* pDeviceExtensions) // TODO: Make more customizable
  {
    LOG_INIT("Logical Device");
    QueueFamilyIndices indices = findQueueFamilies(pContext->physicalDevice, pContext->surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
      VkDeviceQueueCreateInfo queueCreateInfo {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
      queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo createInfo {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = pFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(pDeviceExtensions->size());
    createInfo.ppEnabledExtensionNames = pDeviceExtensions->data();

    // TODO: For compatibility, implement device-specific layers here

    VKF(vkCreateDevice(pContext->physicalDevice, &createInfo, nullptr, &pContext->device))
    {
      LOG_CRIT("An error occured, whilst creating Logical Device: ", res);
      LOG_INIT_AB("Logical Device");
      return false;
    }

    vkGetDeviceQueue(pContext->device, indices.graphicsFamily.value(), 0, &pContext->graphicsQueue);
    vkGetDeviceQueue(pContext->device, indices.presentFamily.value(), 0, &pContext->presentQueue);

    LOG_INIT_OK("Logical Device");
    return true;
  }

  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
  {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    for(const auto& queueFamily : queueFamilies)
    {
      VkBool32 presentSupport = false;
      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
      {
        indices.graphicsFamily = i;
      }
      if (vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport) == VK_SUCCESS)
      {
        indices.presentFamily = i;
      }
      if (indices.isComplete()) break;
      i++;
    }
    return indices;
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

  bool selectPhysicalDevice(VContext* context, VkPhysicalDeviceFeatures* pRequiredFeatures, std::vector<const char*>* pDeviceExtensions)
  {
    LOG_ING("select", "Physical Device");
    uint32_t deviceCount = 0;
    VKF(vkEnumeratePhysicalDevices(context->instance, &deviceCount, nullptr))
    {
      LOG_ERROR("An Error occured whilst querying for the amount of physical devices: ", res);
      LOG_ING_AB("select", "Physical Device");
      return false;
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    VKF(vkEnumeratePhysicalDevices(context->instance, &deviceCount, devices.data()))
    {
      LOG_ERROR("An Error occured whilst querying for physical devices: ", res);
      LOG_ING_AB("select", "Physical Device");
      return false;
    }
    std::multimap<int, std::tuple<VkPhysicalDevice, const char*>> candidates;
    for (std::vector<VkPhysicalDevice>::iterator i = devices.begin(); i != devices.end(); i++)
    {
      VkPhysicalDeviceProperties props;
      VkPhysicalDeviceFeatures features;
      if (isPhysicalDeviceSuitable(*i, context->surface, &props, &features, pRequiredFeatures, pDeviceExtensions))
      {
        uint32_t score = rateDeviceSuitability(&props, &features);
        candidates.insert(std::make_pair(score, std::make_tuple(*i, props.deviceName)));
        LOG_INFO("  \"", props.deviceName, "\" - suitable (", score, ")");
      }
      else LOG_INFO("  \"", props.deviceName, "\" - not suitable");
    }
    if (candidates.rbegin()->first > 0)
    {
      context->physicalDevice = std::get<0>(candidates.rbegin()->second);
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
}