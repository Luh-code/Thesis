#ifndef __VULKAN_BASE_H__
#define __VULKAN_BASE_H__

#include "vulkan/vulkan.h"
#include "../thesis.h"
#include "logger.h"
#include <vector>
#include <algorithm>
#include <cstring>
#include <map>
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
        VkDebugUtilsMessengerEXT debugMessenger;

        VkPhysicalDevice physicalDevice;
        VkDevice device = VK_NULL_HANDLE;
    } VContext;

    // Functions
    uint32_t rateDeviceSuitability(VkPhysicalDeviceProperties* pProps, VkPhysicalDeviceFeatures* pFeatures);
    bool isPhysicalDeviceSuitable(VkPhysicalDevice device, VkPhysicalDeviceProperties* pProps,
        VkPhysicalDeviceFeatures* pFeatures, VkPhysicalDeviceFeatures* pRequiredFeatures);
    bool selectPhysicalDevice(VContext* context, VkPhysicalDeviceFeatures* pRequiredFeatures);

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
    bool setupDebugMessenger(VContext* context);
    bool addDebugging(std::vector<const char*>* pLayers, std::vector<const char*>* pExtensions, bool shrink = true);
    bool queryAvailableLayers(std::vector<char*>* pLayers);
    bool checkLayerAvailability(std::vector<const char*>* pLayers);
    bool createVulkanInstance(VContext* pContext, uint32_t extensionCount, const char** ppExtensions,
        uint32_t layerCount, const char** ppLayers, const char* pAppName = "Test App", uint32_t appVersion = VK_MAKE_API_VERSION(1,0,0,0), bool debug = true);
}

#endif // __VULKAN_BASE_H__