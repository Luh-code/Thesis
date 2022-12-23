#include "../pch.h"
#include "vulkan_base.h"

namespace Ths::Vk
{
  VkCommandBuffer beginSingleTimeGraphicsCommands(VContext* pContext)
  {
    return beginSingleTimeCommands(pContext, pContext->commandPool);
  }

  void endSingleTimeGraphicsCommands(VContext* pContext, VkCommandBuffer& commandBuffer)
  {
    endSingleTimeCommands(pContext, commandBuffer, pContext->commandPool, pContext->graphicsQueue);
  }

  VkCommandBuffer beginSingleTimeTransferCommands(VContext* pContext)
  {
    return beginSingleTimeCommands(pContext, pContext->transferPool);
  }

  void endSingleTimeTransferCommands(VContext* pContext, VkCommandBuffer& commandBuffer)
  {
    endSingleTimeCommands(pContext, commandBuffer, pContext->transferPool, pContext->transferQueue);
  }

  VkCommandBuffer beginSingleTimeCommands(VContext* pContext, VkCommandPool pool)
  {
    VkCommandBufferAllocateInfo allocInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    vkAllocateCommandBuffers(pContext->device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
  }

  void endSingleTimeCommands(VContext* pContext, VkCommandBuffer& commandBuffer, VkCommandPool pool, VkQueue queue)
  {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    // ? consider using fence here instead of waiting for idle
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(pContext->device, pool, 1, &commandBuffer);
  }

  bool createSyncObjects(VContext* pContext)
  {
    LOG_INIT("syncronization objects");
    pContext->imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    pContext->renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    pContext->inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fenceInfo {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      if (vkCreateSemaphore(pContext->device, &semaphoreInfo, nullptr, &pContext->imageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(pContext->device, &semaphoreInfo, nullptr, &pContext->renderFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(pContext->device, &fenceInfo, nullptr, &pContext->inFlightFences[i]) != VK_SUCCESS)
      {
        LOG_ERROR("An error occured whilst creating syncronization objects (iteration ", i, ")!");
        LOG_INIT_AB("syncronization objects");
        return false;
      }
    }
    LOG_INIT_OK("syncronization objects");
    return true;
  }

  bool recordCommandBuffer(VContext* pContext, OContext& object, VkCommandBuffer commandBuffer, uint32_t imageIndex)
  {
    VkCommandBufferBeginInfo beginInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    VKF(vkBeginCommandBuffer(commandBuffer, &beginInfo))
    {
      LOG_ERROR("An error occured whilst beginning command buffer: ", res);
      return false;
    }

    VkRenderPassBeginInfo renderPassInfo {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    renderPassInfo.renderPass = pContext->renderPasses[0];
    renderPassInfo.framebuffer = pContext->swapchainFramebuffers[imageIndex]; // ????
    
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = pContext->swapchainExtent;
    
    std::array<VkClearValue, 2> clearColors;
    clearColors[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearColors[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
    renderPassInfo.pClearValues = clearColors.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
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
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(object.mesh->indices.size()), 1, 0, 0, 0);
    //vkCmdDraw(commandBuffer, static_cast<uint32_t>(pContext->verticies.size()), 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);

    VKF(vkEndCommandBuffer(commandBuffer))
    {
      LOG_ERROR("An error occured whilst ending command buffer: ", res);
      return false;
    }

    return true;
  }

  // bool recordCommandBuffer(VContext* pContext, VkCommandBuffer commandBuffer, uint32_t imageIndex)
  // {
  //   VkCommandBufferBeginInfo beginInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  //   beginInfo.flags = 0;
  //   beginInfo.pInheritanceInfo = nullptr;

  //   VKF(vkBeginCommandBuffer(commandBuffer, &beginInfo))
  //   {
  //     LOG_ERROR("An error occured whilst beginning command buffer: ", res);
  //     return false;
  //   }

  //   VkRenderPassBeginInfo renderPassInfo {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  //   renderPassInfo.renderPass = pContext->renderPasses[0];
  //   renderPassInfo.framebuffer = pContext->swapchainFramebuffers[imageIndex];
    
  //   renderPassInfo.renderArea.offset = {0, 0};
  //   renderPassInfo.renderArea.extent = pContext->swapchainExtent;
    
  //   std::array<VkClearValue, 2> clearColors;
  //   clearColors[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
  //   clearColors[1].depthStencil = {1.0f, 0};
  //   renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
  //   renderPassInfo.pClearValues = clearColors.data();

  //   vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  //   vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pContext->graphicsPipeline);
  //   VkViewport viewport{};
  //   viewport.x = 0.0f;
  //   viewport.y = 0.0f;
  //   viewport.width = static_cast<float>(pContext->swapchainExtent.width);
  //   viewport.height = static_cast<float>(pContext->swapchainExtent.height);
  //   viewport.minDepth = 0.0f;
  //   viewport.maxDepth = 1.0f;
  //   vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  //   VkRect2D scissor{};
  //   scissor.offset = {0, 0};
  //   scissor.extent = pContext->swapchainExtent;
  //   vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
  //   VkBuffer vertexBuffers[] = {pContext->vertexBuffer};
  //   VkDeviceSize offsets[] = {0};
  //   vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
  //   vkCmdBindIndexBuffer(commandBuffer, pContext->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
  //   vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pContext->pipelineLayout, 0, 1, &pContext->descriptorSets[pContext->currentFrame], 0, nullptr);
  //   vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(pContext->indices.size()), 1, 0, 0, 0);
  //   //vkCmdDraw(commandBuffer, static_cast<uint32_t>(pContext->verticies.size()), 1, 0, 0);
  //   vkCmdEndRenderPass(commandBuffer);

  //   VKF(vkEndCommandBuffer(commandBuffer))
  //   {
  //     LOG_ERROR("An error occured whilst ending command buffer: ", res);
  //     return false;
  //   }

  //   return true;
  // }

  bool createCommandBuffers(VContext* pContext, OContext& object)
  {
    LOG_INIT("Command Buffer");
    object.commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocInfo.commandPool = object.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(object.commandBuffers.size());

    VKF(vkAllocateCommandBuffers(pContext->device, &allocInfo, object.commandBuffers.data()))
    {
      LOG_ERROR("An error occured whilst creating command buffers: ", res);
      LOG_INIT_AB("Command Buffer");
      return false;
    }
    LOG_INIT_OK("Command Buffer");
    return false;
  }

  VkResult createCommandPool(VContext* pContext, VkCommandPoolCreateInfo& poolInfo, VkCommandPool* commandPool)
  {
    VKF(vkCreateCommandPool(pContext->device, &poolInfo, nullptr, commandPool))
    {
      LOG_ERROR("An error occured whilst creating a VkCommandPool: ", res);
      //LOG_INIT_AB("Command Pools");
      return res;
    }
    return VK_SUCCESS;
  }

  bool createCommandPools(VContext* pContext)
  {
    LOG_INIT("Command Pools");

    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(pContext->physicalDevice, pContext->surface);
    
    VkCommandPoolCreateInfo poolInfo {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    VKF(createCommandPool(pContext, poolInfo, &pContext->commandPool))
    {
      // LOG_ERROR("An error occured whilst creating a VkCommandPool!");
      LOG_INIT_AB("Command Pools");
      return false;
    }

    VkCommandPoolCreateInfo poolInfo2 {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolInfo2.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo2.queueFamilyIndex = queueFamilyIndices.transferFamily.value();

    VKF(createCommandPool(pContext, poolInfo2, &pContext->transferPool))
    {
      // LOG_ERROR("An error occured whilst creating a VkCommandPool: ", res);
      LOG_INIT_AB("Command Pools");
      return false;
    }

    LOG_INIT_OK("Command Pools");
    return true;
  }
} // namespace Ths::Vk