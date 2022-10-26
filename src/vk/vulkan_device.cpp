#include "vulkan_base.h"

namespace Ths::Vk
{
    bool isPhysicalDeviceSuitable(VkPhysicalDevice device, VkPhysicalDeviceProperties* pProps,
        VkPhysicalDeviceFeatures* pRequiredFeatures, VkPhysicalDeviceType type)
    {
        VK(vkGetPhysicalDeviceProperties(device, pProps));
        VkPhysicalDeviceFeatures features;
        VK(vkGetPhysicalDeviceFeatures(device, &features));

        VkBool32* pBoolRequiredFeatures = reinterpret_cast<VkBool32*>(pRequiredFeatures);
        VkBool32* pBoolFeatures = reinterpret_cast<VkBool32*>(&features);
        
        for (uint32_t i = 0; i < (sizeof(VkPhysicalDeviceFeatures)/sizeof(VkBool32)); i++)
            if (pBoolRequiredFeatures[i] && !pBoolFeatures[i])
        {
            return false;
        }
        return pProps->deviceType == type;
    }

    bool selectPhysicalDevice(VContext* context, VkPhysicalDeviceFeatures* pRequiredFeatures)
    {
        LOG_ING("pick", "Physical Device");
        uint32_t deviceCount = 0;
        VKF(vkEnumeratePhysicalDevices(context->instance, &deviceCount, nullptr))
        {
            LOG_ERROR("An Error occured whilst querying for the amount of physical devices: ", res);
            LOG_ING_AB("pick", "Physical Device");
            return false;
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        VKF(vkEnumeratePhysicalDevices(context->instance, &deviceCount, devices.data()))
        {
            LOG_ERROR("An Error occured whilst querying for physical devices: ", res);
            LOG_ING_AB("pick", "Physical Device");
            return false;
        }
        for (std::vector<VkPhysicalDevice>::iterator i = devices.begin(); i != devices.end(); i++)
        {
            VkPhysicalDeviceProperties props;
            if (isPhysicalDeviceSuitable(*i, &props, pRequiredFeatures, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU))
            {
                LOG_INFO("  \"", props.deviceName, "\" - suitable");
                if (context->physicalDevice == VK_NULL_HANDLE)
                {
                    context->physicalDevice = *i;
                    LOG_INFO_IV("Selected GPU: \"", props.deviceName, "\"");
                }
            }
            else LOG_INFO("  \"", props.deviceName, "\" - not suitable");
        }
        if (context->physicalDevice == VK_NULL_HANDLE)
        {
            LOG_CRIT("No suitable GPU found!");
            LOG_ING_AB("pick", "Physical Device");
            return false;
        }
        LOG_ING_OK("pick", "Physical Device");
        return true;
    }
}