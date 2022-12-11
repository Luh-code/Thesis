#include "../pch.h"
#include "vulkan_base.h"

namespace Ths::Vk
{
  bool hasStencilComponent(VkFormat format)
  {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
  }

  VkFormat findDepthFormat(VContext* pContext)
  {
    return findSupportedFormat(
      pContext,
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
  }

  VkFormat findSupportedFormat(VContext* pContext, const std::vector<VkFormat>& candidates, VkImageTiling tiling,
    VkFormatFeatureFlags features)
  {
    for (VkFormat format : candidates)
    {
      VkFormatProperties props;
      vkGetPhysicalDeviceFormatProperties(pContext->physicalDevice, format, &props);
      if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
      {
          return format;
      } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
      {
          return format;
      }
    }

    LOG_ERROR("Couldn't find suitable format!");
    return VK_FORMAT_UNDEFINED;
  }

  bool createDepthResources(VContext* pContext)
  {
    VkFormat depthFormat = findDepthFormat(pContext);
    if (!createImage(pContext, pContext->swapchainExtent.width, pContext->swapchainExtent.height, 1, depthFormat,
      VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      pContext->depthImage, pContext->depthImageMemory))
    {
      LOG_ERROR("An error occurred whilst creating depth componens!");
      return false;
    }
    
    pContext->depthImageView = createImageView(pContext, pContext->depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    
    transitionImageLayout(pContext, pContext->depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);

    return true;
  }
}