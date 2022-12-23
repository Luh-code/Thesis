#include "vulkan_base.h"
#include "../pch.h"

namespace Ths::Vk
{
  bool createDescriptorSets(VContext* pContext, OContext& object)
  {
    LOG_ING("allocat", "Descriptor Sets");
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, object.descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = object.descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    object.descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    VKF(vkAllocateDescriptorSets(pContext->device, &allocInfo, object.descriptorSets.data()))
    {
      LOG_ERROR("An error occured whilst allocating Descriptor Sets: ", res);
      LOG_ING_AB("allocat", "Descriptor Sets");
      return false;
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      VkDescriptorBufferInfo bufferInfo {};
      bufferInfo.buffer = pContext->uniformBuffers[i];
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(UniformBufferObject);

      VkDescriptorImageInfo imageInfo {};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = object.textureImageView;
      imageInfo.sampler = object.textureSampler;

      std::array<VkWriteDescriptorSet, 2> descriptorWrites {};
      descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[0].dstSet = object.descriptorSets[i];
      descriptorWrites[0].dstBinding = 0;
      descriptorWrites[0].dstArrayElement = 0;
      descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrites[0].descriptorCount = 1;
      descriptorWrites[0].pBufferInfo = &bufferInfo;
      descriptorWrites[0].pImageInfo = nullptr;
      descriptorWrites[0].pTexelBufferView = nullptr;

      descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[1].dstSet = object.descriptorSets[i];
      descriptorWrites[1].dstBinding = 1;
      descriptorWrites[1].dstArrayElement = 0;
      descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptorWrites[1].descriptorCount = 1;
      descriptorWrites[1].pBufferInfo = nullptr;
      descriptorWrites[1].pImageInfo = &imageInfo;
      descriptorWrites[1].pTexelBufferView = nullptr;

      vkUpdateDescriptorSets(pContext->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    LOG_ING_OK("allocat", "Descriptor Sets");
    return true;
  }

  bool createDescriptorPool(VContext* pContext, OContext& object)
  {
    LOG_INIT("Descriptor Pool");
    VkDescriptorPoolSize poolSize {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    // * pool size MUST always be correct. Some drivers may solve size errors internally, others will throw!
    std::array<VkDescriptorPoolSize, 2> poolSizes = {};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolInfo.flags = 0; // Change to VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT when sets are going to be changed after creation
    
    VKF(vkCreateDescriptorPool(pContext->device, &poolInfo, nullptr, &object.descriptorPool))
    {
      LOG_ERROR("An error occured whilst creating Descriptor Pool: ", res);
      LOG_INIT_AB("Descriptor Pool");
      return false;
    }
    LOG_INIT_OK("Descriptor Pool");
    return true;
  }

  bool createUniformBuffers(VContext* pContext)
  {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    pContext->uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    pContext->uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    pContext->uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      createBuffer(pContext, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        pContext->uniformBuffers[i], pContext->uniformBuffersMemory[i]);

      vkMapMemory(pContext->device, pContext->uniformBuffersMemory[i], 0, bufferSize, 0, &pContext->uniformBuffersMapped[i]);
    }
    return true;
  }

  bool createDescriptorSetLayout(VContext* pContext, OContext& object)
  {
    LOG_INIT("Descriptor Set Layout");
    VkDescriptorSetLayoutBinding uboLayoutBinding {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Only really used for textures

    VkDescriptorSetLayoutBinding samplerLayoutBinding {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> layouts = {uboLayoutBinding, samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutInfo.bindingCount = static_cast<uint32_t>(layouts.size());
    layoutInfo.pBindings = layouts.data();

    VKF(vkCreateDescriptorSetLayout(pContext->device, &layoutInfo, nullptr, &object.descriptorSetLayout))
    {
      LOG_ERROR("An error occured whilst creating DescriptorSetLayout: ", res);
      LOG_INIT_AB("Descriptor Set Layout");
      return false;
    }
    LOG_INIT_OK("Descriptor Set Layout");
    return true;
  }
}