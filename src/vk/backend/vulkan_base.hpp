#ifndef __VULKAN_BASE_H__
#define __VULKAN_BASE_H__

#include "vulkan_types.hpp"

namespace Ths::Vk
{

//----- SystemContext -----

bool selectPhysicalDevice(SystemContext& sys, VkPhysicalDeviceFeatures& pRequiredFeatures, std::vector<const char*>& pDeviceExtensions);

//----- ApplicationContext -----

//----- RenderContext -----

//----- SceneContext -----

}

#endif // __VULKAN_BASE_H__