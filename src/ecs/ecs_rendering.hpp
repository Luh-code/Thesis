#ifndef __RENDERING_ECS_H__
#define __RENDERING_ECS_H__

#include "../pch.h"
#include "../vk/vulkan_base.h"
// #include "ecs_base.hpp"
#include "../app/debug_window.hpp"
#include "../engine/camera.hpp"

namespace Ths::ecs
{
  class RenderSystem : public System
  {
  public:
    Ths::Vk::VContext* pContext;
    Coordinator* crd;
    Ths::DebugGui* debugGui;

    Ths::engine::Camera camera;

    inline RenderSystem(Ths::Vk::VContext* pContext, Coordinator* crd, Ths::DebugGui* debugGui)
     : pContext(pContext), crd(crd), debugGui(debugGui)
    {
      using dType = Ths::DebugGui::ManagedVariableDataType;
      debugGui->addManagedVariable("Camera", {
        {&camera.translation, "Translation", dType::VEC3},
        {&camera.focalPoint, "Focal point", dType::VEC3},
        {&camera.upDirection, "Up direction", dType::VEC3},
        {&camera.fov, "Field of view", dType::FLOAT},
        {&camera.nearClippingPlane, "Near clipping plane", dType::FLOAT},
        {&camera.farClippingPlane, "Far clipping plane", dType::FLOAT},
      });
    }

    inline void entityRegistered(Entity entity) override
    {
      auto& object = crd->getComponent<Ths::Vk::OContext>(entity);
      auto& mesh = crd->getComponent<Ths::Vk::Mesh>(entity);
      auto& material = crd->getComponent<Ths::Vk::Material>(entity);
      auto& transform = crd->getComponent<Ths::Vk::Transform>(entity);

      object.material = &material;
      object.mesh = &mesh;
      object.transform = &transform;
    }

    inline void initEntity(Entity e)
    {
      auto& object = crd->getComponent<Ths::Vk::OContext>(e);
      auto& mesh = crd->getComponent<Ths::Vk::Mesh>(e);
      auto& material = crd->getComponent<Ths::Vk::Material>(e);

      std::vector<Ths::Vk::DescriptorSetInfo> descriptorSetInfos;
      descriptorSetInfos = {
        {
          .poolSize = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(1) },
          .pMaterial = &material,
          .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
          .bufferSize = 0,
          .pBuffer = nullptr,
        },
        // { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(1*MAX_FRAMES_IN_FLIGHT) },
      };
      Ths::Vk::createDescriptorSetLayout(pContext, object, descriptorSetInfos);
      Ths::Vk::createGraphicsPipeline(pContext, object,
        object.pPipeline,
        *material.pVertexShader,
        *material.pFragmentShader
      );
      // Ths::Vk::createTextureImage(pContext, material.pTexture, material.path);
      
      // Ths::Vk::createTextureImageView(pContext, *material.pTexture);
      // Ths::Vk::createTextureSampler(pContext, *material.pTexture);

      Ths::Vk::loadModel(pContext, mesh.pMesh, mesh.path, mesh.basepath);
      Ths::Vk::createVertexBuffer(pContext, object);
      Ths::Vk::createIndexBuffer(pContext, object);

      Ths::Vk::createDescriptorPool(pContext, object, descriptorSetInfos);
      Ths::Vk::createDescriptorSets(pContext, object, descriptorSetInfos);

      // Ths::Vk::QueueFamilyIndices queueFamilyIndices = Ths::Vk::findQueueFamilies(pContext->physicalDevice, pContext->surface);
    }

    inline void initEntities()
    {
      for(const auto& e : mEntities)
      {
        initEntity(e);
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

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object.pPipeline->pipeline);
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
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object.pPipeline->layout, 0, 1, &object.descriptorSets[pContext->currentFrame], 0, nullptr);
        
        // push constants

        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        
        glm::mat4 model = glm::mat4(1.0f);

        model = glm::scale(model, transform.scale);
        model = glm::translate(model, transform.translation/transform.scale);
        // model = glm::rotate(model, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // debugGui->addMonitoredVariable("Rotation Magnitude", glm::degrees(time * glm::radians(90.0f)));
        model = glm::rotate(model, transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, transform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, transform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        
        glm::mat4 view = glm::lookAt(camera.translation, camera.focalPoint, camera.upDirection);
        
        glm::mat4 projection = glm::perspective(
          glm::radians(camera.fov),
          pContext->swapchainExtent.width / static_cast<float>(pContext->swapchainExtent.height),
          camera.nearClippingPlane, camera.farClippingPlane
        );
        projection[1][1] *= -1;

        // glm::mat4 mesh_matrix = projection*view*model;

        Ths::Vk::MeshPushConstants modelViewProjection;
        // modelViewProjection.render_matrix = mesh_matrix;
        modelViewProjection.model = model;
        modelViewProjection.view = view;
        modelViewProjection.projection = projection;

        vkCmdPushConstants(
          commandBuffer,
          object.pPipeline->layout,
          VK_SHADER_STAGE_VERTEX_BIT,
          0,
          sizeof(Ths::Vk::MeshPushConstants), &modelViewProjection
        );

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(object.mesh->pMesh->indices.size()), 1, 0, 0, 0);
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