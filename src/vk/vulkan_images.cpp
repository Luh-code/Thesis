#include "../pch.h"
#include "vulkan_base.h"

namespace Ths::Vk
{
  void createColorResources(VContext* pContext)
  {
    LOG_INIT("Color Resources");
    VkFormat colorFormat = pContext->swapchainImageFormat;
    createImage(pContext, pContext->swapchainExtent.width, pContext->swapchainExtent.height, 1, pContext->msaaSamples,
      colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pContext->colorImage, pContext->colorImageMemory);
    pContext->colorImageView = createImageView(pContext, pContext->colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    LOG_INIT_OK("Color Resources");
  }

  bool createTextureSampler(VContext* pContext)
  {
    VkSamplerCreateInfo samplerInfo {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    // VK_FILTER_LINEAR is with filtering
    // VK_FILTER_NEAREST is without filtering
    samplerInfo.magFilter = VK_FILTER_LINEAR; // Oversampling
    samplerInfo.minFilter = VK_FILTER_LINEAR; // Undersampling
    // What to do when image is too small
    // VK_SAMPLER_ADDRESS_MODE_REPEAT - repeats image
    // VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT - repeats image mirrored
    // VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE - repeats color of closest texel
    // VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE - repeats color of farthest texel
    // VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER - repeats solid color when beyond the dimensions of the image
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT; // x
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT; // y
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT; // z
    samplerInfo.anisotropyEnable = true;
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(pContext->physicalDevice, &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; // Enable maximal supported anistropy (can be lowered)
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // Border color if adress mode is clamp to border
    samplerInfo.unnormalizedCoordinates = VK_FALSE; // Convert texture space to UV space?
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    // Mipmapping
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(pContext->mipLevels);

    VKF(vkCreateSampler(pContext->device, &samplerInfo, nullptr, &pContext->textureSampler))
    {
      LOG_ERROR("An error occured whilst creating a VkSampler: ", res);
      return false;
    }
    return true;
  }

  VkImageView createImageView(VContext* pContext, VkImage image, VkFormat format,
    VkImageAspectFlags aspectFlags, uint32_t mipLevels)
  {
    VkImageViewCreateInfo createInfo {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = mipLevels;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    VkImageView imageView = VK_NULL_HANDLE;
    VKF(vkCreateImageView(pContext->device, &createInfo, nullptr, &imageView))
    {
      LOG_ERROR("An Error occured whilst creating a VkImageView: ", res);
    }
    return imageView;
  }

  bool createTextureImageView(VContext* pContext)
  {
    pContext->textureImageView = createImageView(pContext, pContext->textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, pContext->mipLevels);
    return pContext->textureImageView != VK_NULL_HANDLE;
  }

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

  void transitionImageLayout(VContext* pContext, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
  {
    VkCommandBuffer commandBuffer = beginSingleTimeGraphicsCommands(pContext);

    VkImageMemoryBarrier barrier {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
      barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

      if (hasStencilComponent(format))
      {
        barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
      }
    } else
    {
      barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
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
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

      sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
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

  bool createImage(VContext* pContext, uint32_t w, uint32_t h, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
    VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
  {
    VkImageCreateInfo imageInfo {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(w);
    imageInfo.extent.height = static_cast<uint32_t>(h);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = numSamples;
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

  bool generateMipmaps(VContext* pContext, VkImage image, VkFormat format, int32_t w, int32_t h, uint32_t mipLevels)
  {
    // Check support for blitting linearly
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(pContext->physicalDevice, format, &formatProperties);

    if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) != VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)
    {
      LOG_ERROR("Physical device does NOT support linear blitting!");
      return false;
    }

    VkCommandBuffer commandBuffer = beginSingleTimeGraphicsCommands(pContext);

    VkImageMemoryBarrier barrier {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipW = w;
    int32_t mipH = h;

    for (uint32_t i = 1; i < mipLevels; i++)
    {
      barrier.subresourceRange.baseMipLevel = i - 1;
      barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

      vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

      VkImageBlit blit{};
      blit.srcOffsets[0] = { 0, 0, 0 };
      blit.srcOffsets[1] = { mipW, mipH, 1 };
      blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      blit.srcSubresource.mipLevel = i - 1;
      blit.srcSubresource.baseArrayLayer = 0;
      blit.srcSubresource.layerCount = 1;
      blit.dstOffsets[0] = { 0, 0, 0 };
      blit.dstOffsets[1] = { mipW > 1 ? mipW / 2 : 1, mipH > 1 ? mipH / 2 : 1, 1};
      blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      blit.dstSubresource.mipLevel = i;
      blit.dstSubresource.baseArrayLayer = 0;
      blit.dstSubresource.layerCount = 1;

      vkCmdBlitImage(commandBuffer,
        image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &blit,
        VK_FILTER_LINEAR);

      barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

      if (mipW > 1) mipW /= 2;
      if (mipH > 1) mipH /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    endSingleTimeGraphicsCommands(pContext, commandBuffer);
    return true;
  }

  bool createTextureImage(VContext* pContext)
  {
    int w, h, channels;
    stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &w, &h, &channels, STBI_rgb_alpha);
    VkDeviceSize size = w * h * 4; // 4 for amt of channels (rgba)

    if(size == 0)
    {
      LOG_ERROR("Couldn't load image!");
      return false;
    }

    pContext->mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(w, h)))) + 1;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    
    if (!createImage(pContext, w, h, pContext->mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pContext->textureImage, pContext->textureImageMemory))
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

    transitionImageLayout(pContext, pContext->textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, pContext->mipLevels);
    copyBufferToImage(
      pContext,
      stagingBuffer, pContext->textureImage,
      static_cast<uint32_t>(w), static_cast<uint32_t>(h)
    );
    
    generateMipmaps(pContext, pContext->textureImage, VK_FORMAT_B8G8R8A8_SRGB, w, h, pContext->mipLevels);

    // transitionImageLayout(pContext, pContext->textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, pContext->mipLevels);

    vkDestroyBuffer(pContext->device, stagingBuffer, nullptr);
    vkFreeMemory(pContext->device, stagingBufferMemory, nullptr);

    return true;
  }
}