#include "vulkan-base.h"

namespace Ths::Vk
{
    bool createVulkanInstance(VContext* context, uint32_t extensionCount, const char** ppExtensions, const char* appName, uint32_t appVersion)
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
        createInfo.enabledExtensionCount = extensionCount;
        createInfo.ppEnabledExtensionNames = ppExtensions;
        createInfo.enabledLayerCount = 0;

        context->instance = nullptr;

        VKF(vkCreateInstance(&createInfo, nullptr, context->instance))
        {
            LOG_ERROR("An error occured whilst creating Vulkan Instance: "/*, res*/);
            LOG_INIT_AB("Vulkan Instance");
            return false;
        }

        LOG_INIT_OK("Vulkan Instance");
        return true;
    }
}