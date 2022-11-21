#include "vulkan_base.h"
#include "../pch.h"

namespace Ths::Vk
{
  bool createDescriptorSets(VContext* pContext)
  {
    LOG_ING("allocat", "Descriptor Sets");
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, pContext->descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = pContext->descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    pContext->descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    VKF(vkAllocateDescriptorSets(pContext->device, &allocInfo, pContext->descriptorSets.data()))
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

      VkWriteDescriptorSet descriptorWrite {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
      descriptorWrite.dstSet = pContext->descriptorSets[i];
      descriptorWrite.dstBinding = 0;
      descriptorWrite.dstArrayElement = 0;
      descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrite.descriptorCount = 1;
      descriptorWrite.pBufferInfo = &bufferInfo;
      descriptorWrite.pImageInfo = nullptr;
      descriptorWrite.pTexelBufferView = nullptr;

      vkUpdateDescriptorSets(pContext->device, 1, &descriptorWrite, 0, nullptr);
    }

    LOG_ING_OK("allocat", "Descriptor Sets");
    return true;
  }

  bool createDescriptorPool(VContext* pContext)
  {
    LOG_INIT("Descriptor Pool");
    VkDescriptorPoolSize poolSize {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolInfo.flags = 0; // Change to VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT when sets are going to be changed after creation
    
    VKF(vkCreateDescriptorPool(pContext->device, &poolInfo, nullptr, &pContext->descriptorPool))
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

  bool createDescriptorSetLayout(VContext* pContext)
  {
    LOG_INIT("Descriptor Set Layout");
    VkDescriptorSetLayoutBinding uboLayoutBinding {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Only really used for textures

    VkDescriptorSetLayoutCreateInfo layoutInfo {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    VKF(vkCreateDescriptorSetLayout(pContext->device, &layoutInfo, nullptr, &pContext->descriptorSetLayout))
    {
      LOG_ERROR("An error occured whilst creating DescriptorSetLayout: ", res);
      LOG_INIT_AB("Descriptor Set Layout");
      return false;
    }
    LOG_INIT_OK("Descriptor Set Layout");
    return true;
  }
}