#include "vulkan_base.h"
#include "../pch.h"

namespace Ths::Vk
{
  bool createDescriptorSets(VContext* pContext, GPUContext& context,
    std::vector<DescriptorSetInfo> descriptorSetInfos)
  {
    LOG_ING("allocat", "Descriptor Sets");
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, context.descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = context.descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    context.descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    VKF(vkAllocateDescriptorSets(pContext->device, &allocInfo, context.descriptorSets.data()))
    {
      LOG_ERROR("An error occured whilst allocating Descriptor Sets: ", res);
      LOG_ING_AB("allocat", "Descriptor Sets");
      return false;
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      // VkDescriptorBufferInfo bufferInfo {};

      // VkDescriptorImageInfo imageInfo {};
      // imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      // imageInfo.imageView = context.material->pTexture->view;
      // imageInfo.sampler = context.material->pTexture->sampler;

      std::vector<VkWriteDescriptorSet> descriptorWrites {descriptorSetInfos.size()};
      std::vector<VkDescriptorBufferInfo> bufferInfos {descriptorSetInfos.size()};
      std::vector<VkDescriptorImageInfo> imageInfos {descriptorSetInfos.size()};
      for (size_t j = 0; j < descriptorSetInfos.size(); j++)
      {
        if (descriptorSetInfos[j].pBuffer)
        {
          bufferInfos[j] = {
            .buffer = *descriptorSetInfos[j].pBuffer,
            .offset = 0,
            .range = static_cast<uint32_t>(descriptorSetInfos[j].bufferSize),
          };
        }
        if (Material*& mat = descriptorSetInfos[j].pMaterial)
        {
          imageInfos[j] = {
            .sampler = mat->pTexture->sampler,
            .imageView = mat->pTexture->view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
          };
        }
        descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[j].dstSet = context.descriptorSets[i];
        descriptorWrites[j].dstBinding = static_cast<uint32_t>(j);
        descriptorWrites[j].dstArrayElement = 0;
        descriptorWrites[j].descriptorType = descriptorSetInfos[j].poolSize.type;
        descriptorWrites[j].descriptorCount = descriptorSetInfos[j].poolSize.descriptorCount;
        descriptorWrites[j].pBufferInfo = (descriptorSetInfos[j].pBuffer ? &bufferInfos[j] : nullptr);
        descriptorWrites[j].pImageInfo = (descriptorSetInfos[j].pMaterial ? &imageInfos[j] : nullptr);
        descriptorWrites[j].pTexelBufferView = nullptr;
      }
      // descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      // descriptorWrites[0].dstSet = context.descriptorSets[i];
      // descriptorWrites[0].dstBinding = 0;
      // descriptorWrites[0].dstArrayElement = 0;
      // descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      // descriptorWrites[0].descriptorCount = 1;
      // descriptorWrites[0].pBufferInfo = &bufferInfo;
      // descriptorWrites[0].pImageInfo = nullptr;
      // descriptorWrites[0].pTexelBufferView = nullptr;

      // descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      // descriptorWrites[1].dstSet = context.descriptorSets[i];
      // descriptorWrites[1].dstBinding = 1;
      // descriptorWrites[1].dstArrayElement = 0;
      // descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      // descriptorWrites[1].descriptorCount = 1;
      // descriptorWrites[1].pBufferInfo = nullptr;
      // descriptorWrites[1].pImageInfo = &imageInfo;
      // descriptorWrites[1].pTexelBufferView = nullptr;

      vkUpdateDescriptorSets(pContext->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    LOG_ING_OK("allocat", "Descriptor Sets");
    return true;
  }

  bool createDescriptorPool(VContext* pContext, GPUContext& context,
    std::vector<DescriptorSetInfo> descriptorSetInfos)
  {
    LOG_INIT("Descriptor Pool");
    // VkDescriptorPoolSize poolSize {};
    // poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    // * pool size MUST always be correct. Some drivers may solve size errors internally, others will throw!
    // std::vector<VkDescriptorPoolSize> poolSizes = {
    //   { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1*static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) },
    //   { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1*static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) },
    // };

    std::vector<VkDescriptorPoolSize> poolSizes {descriptorSetInfos.size()};
    for(size_t i = 0; i < descriptorSetInfos.size(); i++)
    {
      poolSizes[i] = descriptorSetInfos[i].poolSize;
    }
    VkDescriptorPoolCreateInfo poolInfo {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT*descriptorSetInfos.size()); // ! <--
    poolInfo.flags = 0; // Change to VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT when sets are going to be changed after creation
    
    VKF(vkCreateDescriptorPool(pContext->device, &poolInfo, nullptr, &context.descriptorPool))
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

  bool createDescriptorSetLayout(VContext* pContext, GPUContext& context,
   std::vector<DescriptorSetInfo> descriptorSetInfos)
  {
    LOG_INIT("Descriptor Set Layout");
    std::vector<VkDescriptorSetLayoutBinding> layouts {descriptorSetInfos.size()};
    for (size_t i = 0; i < descriptorSetInfos.size(); i++)
    {
      layouts[i] = {
        .binding = static_cast<uint32_t>(i),
        .descriptorType = descriptorSetInfos[i].poolSize.type,
        .descriptorCount = descriptorSetInfos[i].poolSize.descriptorCount,
        .stageFlags = descriptorSetInfos[i].stage,
        .pImmutableSamplers = nullptr,
      };
    }

    // VkDescriptorSetLayoutBinding uboLayoutBinding {};
    // uboLayoutBinding.binding = 0;
    // uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // uboLayoutBinding.descriptorCount = 1;
    // uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    // uboLayoutBinding.pImmutableSamplers = nullptr; // Only really used for textures

    // VkDescriptorSetLayoutBinding worldEnvLayoutBinding {};
    // worldEnvLayoutBinding.binding = 0;
    // worldEnvLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // worldEnvLayoutBinding.descriptorCount = 1;
    // worldEnvLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    // worldEnvLayoutBinding.pImmutableSamplers = nullptr;

    // VkDescriptorSetLayoutBinding materialBinding {};
    // materialBinding.binding = 0;
    // materialBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // materialBinding.descriptorCount = 2;
    // materialBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    // materialBinding.pImmutableSamplers = nullptr;
    
    // VkDescriptorSetLayoutBinding samplerLayoutBinding {};
    // samplerLayoutBinding.binding = 3;
    // samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    // samplerLayoutBinding.descriptorCount = 1;
    // samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    // samplerLayoutBinding.pImmutableSamplers = nullptr;

    // std::array<VkDescriptorSetLayoutBinding, 4> layouts = {
    //   uboLayoutBinding,
    //   worldEnvLayoutBinding,
    //   materialBinding,
    //   samplerLayoutBinding
    // };
    VkDescriptorSetLayoutCreateInfo layoutInfo {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutInfo.bindingCount = static_cast<uint32_t>(layouts.size());
    layoutInfo.pBindings = layouts.data();

    VKF(vkCreateDescriptorSetLayout(pContext->device, &layoutInfo, nullptr, &context.descriptorSetLayout))
    {
      LOG_ERROR("An error occured whilst creating DescriptorSetLayout: ", res);
      LOG_INIT_AB("Descriptor Set Layout");
      return false;
    }
    LOG_INIT_OK("Descriptor Set Layout");
    return true;
  }
}