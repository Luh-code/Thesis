#ifndef __VULKAN_BASE_H__
#define __VULKAN_BASE_H__

#include "vulkan/vulkan.h"
#include "../thesis.h"
#include "logger.h"
//#include "../sdl/sdl-base.h"
// Sheriff Setup & Config

#define VK_FAIL(val) if((val) != VK_SUCCESS)
#define VKF(val) VK_FAIL(val)
#define ASSERT_VULKAN(val) VKF(val) { LOG_CRIT("An error occured in an attempted vulkan call"); assert(false); }
#define VK(f) f
#define VKA(f) ASSERT_VULKAN(VK(f))

namespace Ths::Vk
{
    // Vulkan Creation
    // Data

    typedef struct VulkanContext
    {
        VkInstance* instance;
    } VContext;

    // Functions

    bool createVulkanInstance(VContext* context, uint32_t extensionCount, const char** ppExtensions, const char* appName = "Test App", uint32_t appVersion = VK_MAKE_API_VERSION(1,0,0,0));
}

#endif // __VULKAN_BASE_H__