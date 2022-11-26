#include "../pch.h"
#include "vulkan_base.h"

namespace Ths::Vk
{
  void copyBufferToImage(VContext* pContext, VkBuffer buffer, VkImage image, uint32_t w, uint32_t h)
  {
    VkCommandBuffer commandBuffer = beginSingleTimeTransferCommands(pContext);

    VkBufferImageCopy region {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {w, h, 1};

    vkCmdCopyBufferToImage(
      commandBuffer,
      buffer,
      image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1,
      &region
    );

    endSingleTimeTransferCommands(pContext, commandBuffer);
  }

  void transitionImageLayout(VContext* pContext, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
  {
    VkCommandBuffer commandBuffer = beginSingleTimeGraphicsCommands(pContext);

    VkImageMemoryBarrier barrier {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

      sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
      LOG_ERROR("An unsupported Image Layout Transition was called: old=\"", oldLayout, "\", new=\"", newLayout, "\"");
      return;
    }

    vkCmdPipelineBarrier(
      commandBuffer,
      sourceStage, destinationStage,
      0,
      0, nullptr,
      0, nullptr,
      1, &barrier
    );

    endSingleTimeGraphicsCommands(pContext, commandBuffer);
  }

  bool createImage(VContext* pContext, uint32_t w, uint32_t h, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
  {
    VkImageCreateInfo imageInfo {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(w);
    imageInfo.extent.height = static_cast<uint32_t>(h);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    VKF(vkCreateImage(pContext->device, &imageInfo, nullptr, &image))
    {
      LOG_ERROR("An error occured whilst creating Vk Image: ", res);
      return false;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(pContext->device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(pContext, memRequirements.memoryTypeBits, properties);

    VKF(vkAllocateMemory(pContext->device, &allocInfo, nullptr, &imageMemory))
    {
      LOG_ERROR("An error occured whilst allocating memory for Vk Image: ", res);
      return false;
    }

    vkBindImageMemory(pContext->device, image, imageMemory, 0);

    return true;
  }

  bool createTextureImage(VContext* pContext)
  {
    int w, h, channels;
    stbi_uc* pixels = stbi_load("D:/Projects/Thesis/src/vk/textures/texture.jpg", &w, &h, &channels, STBI_rgb_alpha);
    VkDeviceSize size = w * h * 4; // 4 for amt of channels (rgba)

    if(size == 0)
    {
      LOG_ERROR("Couldn't load image!");
      return false;
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    
    if (!createImage(pContext, w, h, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
      VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pContext->textureImage, pContext->textureImageMemory))
      return false;
    

    createBuffer(pContext, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
      stagingBufferMemory);
    
    void* data;
    vkMapMemory(pContext->device, stagingBufferMemory, 0,
      size, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(size));
    vkUnmapMemory(pContext->device, stagingBufferMemory);

    stbi_image_free(pixels);

    transitionImageLayout(pContext, pContext->textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(
      pContext,
      stagingBuffer, pContext->textureImage,
      static_cast<uint32_t>(w), static_cast<uint32_t>(h)
    );
    
    transitionImageLayout(pContext, pContext->textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(pContext->device, stagingBuffer, nullptr);
    vkFreeMemory(pContext->device, stagingBufferMemory, nullptr);

    return true;
  }
}