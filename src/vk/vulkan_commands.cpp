#include "vulkan_base.h"

namespace Ths::Vk
{
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

  bool recordCommandBuffer(VContext* pContext, VkCommandBuffer commandBuffer, uint32_t imageIndex)
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
    renderPassInfo.framebuffer = pContext->swapchainFramebuffers[imageIndex];
    
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = pContext->swapchainExtent;
    
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pContext->graphicsPipeline);
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
    VkBuffer vertexBuffers[] = {pContext->vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdDraw(commandBuffer, static_cast<uint32_t>(pContext->verticies.size()), 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);

    VKF(vkEndCommandBuffer(commandBuffer))
    {
      LOG_ERROR("An error occured whilst ending command buffer: ", res);
      return false;
    }

    return true;
  }

  bool createCommandBuffers(VContext* pContext)
  {
    LOG_INIT("Command Buffer");
    pContext->commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocInfo.commandPool = pContext->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(pContext->commandBuffers.size());

    VKF(vkAllocateCommandBuffers(pContext->device, &allocInfo, pContext->commandBuffers.data()))
    {
      LOG_ERROR("An error occured whilst creating command buffers: ", res);
      LOG_INIT_AB("Command Buffer");
      return false;
    }
    LOG_INIT_OK("Command Buffer");
    return false;
  }

  bool createCommandPools(VContext* pContext)
  {
    LOG_INIT("Command Pool");
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(pContext->physicalDevice, pContext->surface);

    VkCommandPoolCreateInfo poolInfo {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    VKF(vkCreateCommandPool(pContext->device, &poolInfo, nullptr, &pContext->commandPool))
    {
      LOG_ERROR("An error occured whilst creating a VkCommandPool: ", res);
      LOG_INIT_AB("Command Pool");
      return false;
    }

    VkCommandPoolCreateInfo poolInfo2 {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolInfo2.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo2.queueFamilyIndex = queueFamilyIndices.transferFamily.value();

    VKF(vkCreateCommandPool(pContext->device, &poolInfo2, nullptr, &pContext->transferPool))
    {
      LOG_ERROR("An error occured whilst creating a VkCommandPool: ", res);
      LOG_INIT_AB("Command Pool");
      return false;
    }

    LOG_INIT_OK("Command Pool");
    return true;
  }
} // namespace Ths::Vk