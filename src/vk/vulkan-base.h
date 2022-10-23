#ifndef __VULKAN_BASE_H__
#define __VULKAN_BASE_H__

#include "sheriff-base.h"
#include "vulkan/vulkan.h"
#include "../sdl/sdl-base.h"

// Sheriff Setup & Config

namespace Ths::Vk
{
    enum VkShfErr : int32_t
    {
        ERR_VK_UNDEFINED = 1,
        ERR_VK_NONE = 0,
        CERR_VK_UNDEFINED = -1,
    };

    static shf::Territory vkTerr("Vulkan", ERR_VK_NONE);

    // Vulkan Creation
    // Data

    typedef struct VulkanData
    {
        VkInstance* instance;
    } VData;

    // Functions

    shf::RetV createVulkanInstance();
}

#endif // __VULKAN_BASE_H__