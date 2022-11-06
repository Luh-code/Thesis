#include "vulkan_base.h"

namespace Ths::Vk
{
  bool createCommandPool(VContext* pContext)
  {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(pContext->physicalDevice, pContext->surface);

    VkCommandPoolCreateInfo poolInfo {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    VKF(vkCreateCommandPool(pContext->device, &poolInfo, nullptr, &pContext->commandPool))
    {
      LOG_ERROR("An error occured whilst creating a VkCommandPool: ", res);
      return false;
    }
    return true;
  }
} // namespace Ths::Vk