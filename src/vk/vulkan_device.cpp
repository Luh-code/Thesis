#include "vulkan_base.h"

namespace Ths::Vk
{
    uint32_t rateDeviceSuitability(VkPhysicalDeviceProperties* pProps, VkPhysicalDeviceFeatures* pFeatures)
    {
        uint32_t score = 0;
        score += static_cast<uint32_t>(pProps->deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)*1000;
        score += static_cast<uint32_t>(pProps->deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)*500;

        score += pProps->limits.maxImageDimension2D;

        for (uint32_t i = 0; i < (sizeof(VkPhysicalDeviceFeatures)/sizeof(int)); i++) score += reinterpret_cast<uint32_t*>(pFeatures)[i];
        return score;
    }
    
    bool isPhysicalDeviceSuitable(VkPhysicalDevice device, VkPhysicalDeviceProperties* pProps,
        VkPhysicalDeviceFeatures* pFeatures, VkPhysicalDeviceFeatures* pRequiredFeatures)
    {
        VK(vkGetPhysicalDeviceProperties(device, pProps));
        VK(vkGetPhysicalDeviceFeatures(device, pFeatures));

        VkBool32* pBoolRequiredFeatures = reinterpret_cast<VkBool32*>(pRequiredFeatures);
        VkBool32* pBoolFeatures = reinterpret_cast<VkBool32*>(pFeatures);
        
        for (uint32_t i = 0; i < (sizeof(VkPhysicalDeviceFeatures)/sizeof(VkBool32)); i++)
            if (pBoolRequiredFeatures[i] && !pBoolFeatures[i])
        {
            return false;
        }
        return true;//pProps->deviceType == type;
    }

    bool selectPhysicalDevice(VContext* context, VkPhysicalDeviceFeatures* pRequiredFeatures)
    {
        LOG_ING("select", "Physical Device");
        uint32_t deviceCount = 0;
        VKF(vkEnumeratePhysicalDevices(context->instance, &deviceCount, nullptr))
        {
            LOG_ERROR("An Error occured whilst querying for the amount of physical devices: ", res);
            LOG_ING_AB("select", "Physical Device");
            return false;
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        VKF(vkEnumeratePhysicalDevices(context->instance, &deviceCount, devices.data()))
        {
            LOG_ERROR("An Error occured whilst querying for physical devices: ", res);
            LOG_ING_AB("select", "Physical Device");
            return false;
        }
        std::multimap<int, std::tuple<VkPhysicalDevice, const char*>> candidates;
        for (std::vector<VkPhysicalDevice>::iterator i = devices.begin(); i != devices.end(); i++)
        {
            VkPhysicalDeviceProperties props;
            VkPhysicalDeviceFeatures features;
            if (isPhysicalDeviceSuitable(*i, &props, &features, pRequiredFeatures))
            {
                uint32_t score = rateDeviceSuitability(&props, &features);
                candidates.insert(std::make_pair(score, std::make_tuple(*i, props.deviceName)));
                LOG_INFO("  \"", props.deviceName, "\" - suitable (", score, ")");
            }
            else LOG_INFO("  \"", props.deviceName, "\" - not suitable");
        }
        if (candidates.rbegin()->first > 0)
        {
            context->physicalDevice = std::get<0>(candidates.rbegin()->second);
            LOG_INFO_IV("Selected GPU: \"", std::get<1>(candidates.rbegin()->second), "\"");
        }
        else
        {
            LOG_CRIT("No suitable GPU found!");
            LOG_ING_AB("select", "Physical Device");
            return false;
        }
        LOG_ING_OK("select", "Physical Device");
        return true;
    }
}