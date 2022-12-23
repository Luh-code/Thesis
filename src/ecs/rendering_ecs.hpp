#ifndef __RENDERING_ECS_H__
#define __RENDERING_ECS_H__

#include "../pch.h"
#include "../vk/vulkan_base.h"
#include "ecs_base.hpp"

namespace Ths::ecs
{
  class RenderSystem : public System
  {
  public:
    Ths::Vk::VContext* pContext;
    Coordinator* crd;

    inline RenderSystem(Ths::Vk::VContext* pContext, Coordinator* crd)
     : pContext(pContext), crd(crd)
    { }

    inline void initEntities()
    {
      for(const auto& e : mEntities)
      {
        auto& object = crd->getComponent<Ths::Vk::OContext>(e);
        auto& mesh = crd->getComponent<Ths::Vk::Mesh>(e);
        auto& material = crd->getComponent<Ths::Vk::Material>(e);

        Ths::Vk::createDescriptorSetLayout(pContext, object);
        Ths::Vk::createGraphicsPipeline(pContext, object);
        Ths::Vk::createTextureImage(pContext, object, material.path);
        
        Ths::Vk::createTextureImageView(pContext, object);
        Ths::Vk::createTextureSampler(pContext, object);

        Ths::Vk::loadModel(pContext, object, mesh.path, mesh.basepath);
        Ths::Vk::createVertexBuffer(pContext, object);
        Ths::Vk::createIndexBuffer(pContext, object);

        Ths::Vk::createDescriptorPool(pContext, object);
        Ths::Vk::createDescriptorSets(pContext, object);

        Ths::Vk::QueueFamilyIndices queueFamilyIndices = Ths::Vk::findQueueFamilies(pContext->physicalDevice, pContext->surface);
    
        VkCommandPoolCreateInfo poolInfo {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        Ths::Vk::createCommandPool(pContext, poolInfo, &object.commandPool);
        Ths::Vk::createCommandBuffers(pContext, object);
      }
    }

    inline void recordBuffers(uint32_t imageIndex)
    {
      for(const auto& e : mEntities)
      {
        auto& object = crd->getComponent<Ths::Vk::OContext>(e);
        auto& model = crd->getComponent<Ths::Vk::Mesh>(e);

        vkResetCommandBuffer(object.commandBuffers[imageIndex], 0);
        Ths::Vk::recordCommandBuffer(pContext, object, object.commandBuffers[imageIndex], imageIndex);
      }
    }

    inline std::vector<VkCommandBuffer> getCommandBuffers(uint32_t imageIndex)
    {
      std::vector<VkCommandBuffer> commandBuffers{mEntities.size()};
      size_t i = 0;
      for(auto const& e : mEntities)
      {
        auto& object = crd->getComponent<Ths::Vk::OContext>(e);
        commandBuffers[i] = object.commandBuffers[imageIndex];
        ++i;
      }
      return commandBuffers;
    }

    inline void destroyAllVkObjects()
    {
      for(auto const& e : mEntities)
      {
        auto& object = crd->getComponent<Ths::Vk::OContext>(e);
        auto& material = crd->getComponent<Ths::Vk::Material>(e);
        object.destroy(pContext);
        material.destroy(pContext);
      }
    }
  };
}

#endif // __RENDERING_ECS_H__