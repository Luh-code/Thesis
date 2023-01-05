#ifndef __RESOURCES_VULKAN_H__
#define __RESOURCES_VULKAN_H__

#include "../../pch.h"

namespace Ths::Vk
{
  typedef struct ImageResource : public Ths::Resource
  {
    VkImage image;
    VkDeviceMemory imageMemory;
  } ImageResource;

  typedef struct TextureResource : public ImageResource
  {
    VkSampler sampler;
    VkImageView view;
  } TextureResource;
}

#endif // __RESOURCES_VULKAN_H__