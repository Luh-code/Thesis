#ifndef __VULKAN_BASE_H__
#define __VULKAN_BASE_H__

#include "vulkan/vulkan.h"
#include "../thesis.h"
#include "logger.h"
#include <vector>
//#include "../sdl/sdl-base.h"

#define VK_FAIL(val) if(VkResult res = (val); res != VK_SUCCESS)
#define VKF(val) VK_FAIL(val)
#define ASSERT_VULKAN(val) VKF(val) { LOG_CRIT("An error occured in an attempted vulkan call: ", ret); assert(false); }
#define VK(f) f
#define VKA(f) ASSERT_VULKAN(VK(f))

#define ARRAY_SIZE(v) (sizeof(v)/sizeof(v[0]))

namespace Ths::Vk
{
    // Vulkan Creation
    // Data

    typedef struct VulkanContext
    {
        VkInstance instance;
    } VContext;

    // Functions

    bool queryAvailableLayers(const char** ppLayers, uint32_t* pLayerCount);
    bool checkLayerAvailability(std::vector<const char*>* pLayers, uint32_t* pLayerCount);
    bool createVulkanInstance(VContext* pContext, uint32_t extensionCount, const char** ppExtensions, uint32_t layerCount, const char** ppLayers, const char* pAppName = "Test App", uint32_t appVersion = VK_MAKE_API_VERSION(1,0,0,0));
}

#endif // __VULKAN_BASE_H__