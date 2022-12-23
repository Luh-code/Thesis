#include "../pch.h"
#include "vulkan_base.h"

namespace Ths::Vk
{
  VkSampleCountFlagBits getMaxUsableSampleCount(VContext* pContext)
  {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(pContext->physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    return counts & VK_SAMPLE_COUNT_64_BIT ? VK_SAMPLE_COUNT_64_BIT :
      counts & VK_SAMPLE_COUNT_32_BIT ? VK_SAMPLE_COUNT_32_BIT :
      counts & VK_SAMPLE_COUNT_16_BIT ? VK_SAMPLE_COUNT_16_BIT :
      counts & VK_SAMPLE_COUNT_8_BIT ? VK_SAMPLE_COUNT_8_BIT :
      counts & VK_SAMPLE_COUNT_4_BIT ? VK_SAMPLE_COUNT_4_BIT :
      counts & VK_SAMPLE_COUNT_2_BIT ? VK_SAMPLE_COUNT_2_BIT :
      VK_SAMPLE_COUNT_1_BIT;
  }

  void cleanupSwapChain(VulkanContext* pContext)
  {
    vkDestroyImageView(pContext->device, pContext->colorImageView, nullptr);
    vkDestroyImage(pContext->device, pContext->colorImage, nullptr);
    vkFreeMemory(pContext->device, pContext->colorImageMemory, nullptr);

    vkDestroyImageView(pContext->device, pContext->depthImageView, nullptr);
    vkDestroyImage(pContext->device, pContext->depthImage, nullptr);
    vkFreeMemory(pContext->device, pContext->depthImageMemory, nullptr);

    for (size_t i = 0; i < pContext->swapchainFramebuffers.size(); i++) {
        vkDestroyFramebuffer(pContext->device, pContext->swapchainFramebuffers[i], nullptr);
    }

    for (size_t i = 0; i < pContext->swapchainImageViews.size(); i++) {
        vkDestroyImageView(pContext->device, pContext->swapchainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(pContext->device, pContext->swapchain, nullptr);
  }

  bool recreateSwapChain(VulkanContext* pContext, uint32_t win_width, uint32_t win_height, uint32_t imgs,
    VkPresentModeKHR preferredPresentMode,
    VkFormat preferredFormat, VkColorSpaceKHR preferredColorSpace)
  {
    vkDeviceWaitIdle(pContext->device);

    cleanupSwapChain(pContext);

    createSwapChain(pContext, win_width, win_height, imgs, preferredPresentMode, preferredFormat, preferredColorSpace);
    createImageViews(pContext);
    createColorResources(pContext);
    createDepthResources(pContext);
    createFramebuffers(pContext);
    return true;
  }

  bool createImageViews(VulkanContext* pContext)
  {
    LOG_INIT("Image Views");
    pContext->swapchainImageViews.resize(pContext->swapchainImages.size());

    for (uint32_t i = 0; i < pContext->swapchainImages.size(); i++)
    {
      pContext->swapchainImageViews[i] = createImageView(pContext, pContext->swapchainImages[i],
        pContext->swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
      
      if (pContext->swapchainImageViews[i] == VK_NULL_HANDLE)
      {
        LOG_ERROR("An error occured whilst creating an image view");
        LOG_INIT_AB("Image Views");
        return false;
      }
    }
    LOG_INIT_OK("Image Views");
    return true;
  }

  bool createSwapChain(VulkanContext* pContext, uint32_t win_width, uint32_t win_height, uint32_t imgs,
    VkPresentModeKHR preferredPresentMode,
    VkFormat preferredFormat, VkColorSpaceKHR preferredColorSpace)
  {
    LOG_INIT("Swapchain");
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(pContext->physicalDevice, pContext->surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapChainSurfaceFormat(swapChainSupport.formats, preferredFormat, preferredColorSpace);
    VkPresentModeKHR presentMode = chooseSwapChainPresentMode(swapChainSupport.presentModes, preferredPresentMode);
    VkExtent2D extent = chooseSwapChainExtent(swapChainSupport.capabilities, win_width, win_height);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount == 0 || imgs <= swapChainSupport.capabilities.maxImageCount)
    {
      imageCount = imgs;
    }
    else LOG_WARN("  Couldn't set preferred image count! - continuing");

    VkSwapchainCreateInfoKHR createInfo {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    createInfo.surface = pContext->surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; // only increase to 2 for stereoscopic rendering
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // if post-processing is used, you might want to change this

    QueueFamilyIndices indices = findQueueFamilies(pContext->physicalDevice, pContext->surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value(), indices.transferFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily)
    {
      LOG_DEBUG("Sharing Mode: Concurrent");
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 3;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else
    {
      LOG_DEBUG("Sharing Mode: Exclusive");
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = 0;
      createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE; // used if swapchain becomes invalid (fe. when resizing window)

    VKF(vkCreateSwapchainKHR(pContext->device, &createInfo, nullptr, &pContext->swapchain))
    {
      LOG_ERROR("An error occured whilst creating a swapchain: ", res);
      LOG_INIT_AB("Swapchain");
      return false;
    }

    VK(vkGetSwapchainImagesKHR(pContext->device, pContext->swapchain, &imageCount, nullptr));
    pContext->swapchainImages.resize(imageCount);
    VK(vkGetSwapchainImagesKHR(pContext->device, pContext->swapchain, &imageCount, pContext->swapchainImages.data()));

    pContext->swapchainImageFormat = surfaceFormat.format;
    pContext->swapchainExtent = extent;

    LOG_INIT_OK("Swapchain");
    return true;
  }

  VkExtent2D chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t win_width, uint32_t win_height)
  {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
      return capabilities.currentExtent;
    } else
    {
      VkExtent2D actualExtent =
      {
        static_cast<uint32_t>(win_width),
        static_cast<uint32_t>(win_height)
      };

      actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
      actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

      return actualExtent;
    }
  }

  VkPresentModeKHR chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes,
    VkPresentModeKHR preferredPresentMode)
  {
    for (const auto& presentMode : availablePresentModes)
    {
      if (presentMode == preferredPresentMode)
      {
        LOG_INFO("  selected preferred present mode: ", presentMode);
        return presentMode;
      }
    }
    LOG_WARN("    preferred present mode not found! - continuing");

    LOG_INFO("  selected alternative present mode: ", VK_PRESENT_MODE_FIFO_KHR);
    return VK_PRESENT_MODE_FIFO_KHR;
  }

  VkSurfaceFormatKHR chooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats,
    VkFormat preferredFormat, VkColorSpaceKHR preferredColorSpace)
  {
    for (const auto& format : availableFormats)
    {
      if (format.format == preferredFormat && format.colorSpace == preferredColorSpace)
      {
        LOG_INFO("  selected preferred surface format: ", format.format, "/", format.colorSpace);
        return format;
      }
    }
    LOG_WARN("    preferred surface format not found! - continuing");

    LOG_INFO("  selected alternative surface format: ", availableFormats[0].format, "/", availableFormats[0].colorSpace);
    return availableFormats[0];
  }

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
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value(), indices.transferFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
      VkDeviceQueueCreateInfo queueCreateInfo {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
      queueCreateInfo.queueFamilyIndex = queueFamily;
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
    vkGetDeviceQueue(pContext->device, indices.transferFamily.value(), 0, &pContext->transferQueue);

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
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
      if (!indices.graphicsFamily.has_value() && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
      {
        indices.graphicsFamily = i;
      }
      else if (!indices.transferFamily.has_value() && (queueFamily.queueFlags & (VK_QUEUE_TRANSFER_BIT)) == (VK_QUEUE_TRANSFER_BIT))
      // else if (!indices.transferFamily.has_value() && (queueFamily.queueFlags & (VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT)) == (VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT))
    //  if ((indices.graphicsFamily.has_value() && indices.graphicsFamily.value() != i) || queueFamilyCount > 2 &&
    //     !indices.transferFamily.has_value() && (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
      {
        indices.transferFamily = i;
      }
      else if (!indices.presentFamily.has_value() && presentSupport == VK_TRUE)
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
    // for (std::vector<VkPhysicalDevice>::iterator i = devices.begin(); i != devices.end(); i++)
    for (const auto& i : devices)
    {
      VkPhysicalDeviceProperties props;
      VkPhysicalDeviceFeatures features;
      if (isPhysicalDeviceSuitable(i, context->surface, &props, &features, pRequiredFeatures, pDeviceExtensions))
      {
        uint32_t score = rateDeviceSuitability(&props, &features);
        candidates.insert(std::make_pair(score, std::make_tuple(i, props.deviceName)));
        LOG_INFO("  \"", props.deviceName, "\" - suitable (", score, ")");
      }
      else LOG_INFO("  \"", props.deviceName, "\" - not suitable");
    }
    if (candidates.rbegin()->first > 0)
    {
      context->physicalDevice = std::get<0>(candidates.rbegin()->second);
      VkSampleCountFlagBits maxSamples = getMaxUsableSampleCount(context);
      if (context->msaaSamples > maxSamples)
      {
        LOG_WARN(context->msaaSamples, "x MSAA samples are not supported! Lowering to ", maxSamples, "x");
        context->msaaSamples = maxSamples;
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
}