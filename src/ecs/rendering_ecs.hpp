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
      }
    }

    inline void recordBuffer(uint32_t imageIndex, VkCommandBuffer& commandBuffer)
    {
      for (auto const& e : mEntities)
      {
        auto& object = crd->getComponent<Ths::Vk::OContext>(e);
        auto& mesh = crd->getComponent<Ths::Vk::Mesh>(e);
        auto& material = crd->getComponent<Ths::Vk::Material>(e);
        auto& transform = crd->getComponent<Ths::Vk::Transform>(e);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object.graphicsPipeline);
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(pContext->swapchainExtent.width);
        viewport.height = static_cast<float>(pContext->swapchainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = pContext->swapchainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        VkBuffer vertexBuffers[] = {object.vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, object.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object.pipelineLayout, 0, 1, &object.descriptorSets[pContext->currentFrame], 0, nullptr);
        
        // push constants

        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        
        glm::mat4 model = glm::translate(glm::mat4(1.0f), transform.translation);
        model = glm::rotate(model, transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, transform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, transform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model *= glm::vec4(transform.scale, 1.0f);
        model = glm::rotate(model, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        
        glm::vec3 cameraTranslation = {2.8f, 2.8f, 2.0f};
        glm::vec3 lookAtTranslation = {0.0f, 0.0f, 0.0f};
        glm::vec3 upDirection {0.0f, 0.0f, 1.0f};
        glm::mat4 view = glm::lookAt(cameraTranslation, lookAtTranslation, upDirection);

        float fov = 60;
        float nearClippingPlane = 0.1f;
        float farClippingPlane = 100.0f;
        glm::mat4 projection = glm::perspective(
          glm::radians(fov),
          pContext->swapchainExtent.width / static_cast<float>(pContext->swapchainExtent.height),
          nearClippingPlane, farClippingPlane
        );
        projection[1][1] *= -1;

        glm::mat4 mesh_matrix = projection*view*model;

        Ths::Vk::MeshPushConstants modelViewProjection;
        modelViewProjection.render_matrix = mesh_matrix;

        vkCmdPushConstants(
          commandBuffer,
          object.pipelineLayout,
          VK_SHADER_STAGE_VERTEX_BIT,
          0,
          sizeof(Ths::Vk::MeshPushConstants), &modelViewProjection
        );

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(object.mesh->indices.size()), 1, 0, 0, 0);
      }
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