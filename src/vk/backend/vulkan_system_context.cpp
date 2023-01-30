#include "pch.h"
#include "vulkan_base.hpp"

namespace Ths::Vk
{
  bool selectPhysicalDevice(SystemContext& sys, VkPhysicalDeviceFeatures& pRequiredFeatures, std::vector<const char*>& pDeviceExtensions)
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
    // for (std::vector<VkPhysicalDevice>::iterator i = devices.begin(); i != devices.end(); i++)
    for (const auto& i : devices)
    {
      VkPhysicalDeviceProperties props;
      VkPhysicalDeviceFeatures features;
      if (isPhysicalDeviceSuitable(i, context->surface, &props, &features, pRequiredFeatures, pDeviceExtensions))
      {
        uint32_t score = rateDeviceSuitability(&props, &features);
        candidates.insert(std::make_pair(score, std::make_tuple(i, props.deviceName)));
        LOG_INFO("  \"", props.deviceName, "\" - suitable (", score, ")");
      }
      else LOG_INFO("  \"", props.deviceName, "\" - not suitable");
    }
    if (candidates.rbegin()->first > 0)
    {
      context->physicalDevice = std::get<0>(candidates.rbegin()->second);
      VkSampleCountFlagBits maxSamples = getMaxUsableSampleCount(context);
      if (context->msaaSamples > maxSamples)
      {
        LOG_WARN(context->msaaSamples, "x MSAA samples are not supported! Lowering to ", maxSamples, "x");
        context->msaaSamples = maxSamples;
      }
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
} // namespace Ths::Vk
