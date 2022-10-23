#include "vulkan-base.h"

namespace Ths::Vk
{
    bool createVulkanInstance(VContext& context, const char* appName, uint32_t appVersion)
    {
        LOG_INIT("Vulkan Instance");
        VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        appInfo.pApplicationName = appName;
        appInfo.applicationVersion = appVersion;
        appInfo.pEngineName = ENGINE_NAME;
        appInfo.engineVersion = ENGINE_VERSION;
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        createInfo.pApplicationInfo = &appInfo;
        LOG_INIT_OK("Vulkan Instance");
        return true;
    }
}