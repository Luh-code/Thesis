#include "vulkan_base.h"
#include "../pch.h"

namespace Ths::Vk
{
  bool createTextureImage(VContext* pContext)
  {
    uint32_t w, h, channels;
    stbi_uc* pixels = stbi_load("textures/texture.jpg", &w, &h, &channels, STBI_rgb_alpha);
    VkDeviceSize size = w * h * 4; // 4 for amt of channels (rgba)

    if(!size)
    {
      LOG_ERROR("Couldn't load image!");
      return false;
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
      stagingBufferMemory);
    
    void* data;
    vkMapMemory(pContext->device, stagingBufferMemory, 0,
      size, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(size));
    vkUnmapMemory(pContext->device, stagingBufferMemory);

    stbi_image_free(pixels);

    VkImageCreateInfo imageInfo {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(w);
    imageInfo.extemt.height = static_cast<uint32_t>(h);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.intialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
      VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    VKF(vkCreateImage(pContext->device, &imageInfo, nullptr, pContext->textureImage))
    {
      LOG_ERROR("An error occured whilst creating Vk Image: ", res);
      return false;
    }

    return true;
  }
}