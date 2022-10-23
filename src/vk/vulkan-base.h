#ifndef __VULKAN_BASE_H__
#define __VULKAN_BASE_H__

#include "vulkan/vulkan.h"
#include "../thesis.h"
//#include "../sdl/sdl-base.h"
// Sheriff Setup & Config

namespace Ths::Vk
{
    // Vulkan Creation
    // Data

    typedef struct VulkanContext
    {
        VkInstance* instance;
    } VContext;

    // Functions

    bool createVulkanInstance(VContext& context, const char* appName = "Test App", uint32_t appVersion = VK_MAKE_API_VERSION(1,0,0,0));
}

#endif // __VULKAN_BASE_H__