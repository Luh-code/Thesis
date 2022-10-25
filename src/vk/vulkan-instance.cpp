#include "vulkan-base.h"

namespace Ths::Vk
{
    bool queryAvailableLayers(const char** ppLayers, uint32_t* pLayerCount)
    {
        LOG_QUER("for available Layers");
        uint32_t* pLayerPropertyCount;
        if (pLayerCount = 0) pLayerPropertyCount = pLayerCount;
        else pLayerPropertyCount = new uint32_t();
        VKF(vkEnumerateInstanceLayerProperties(pLayerPropertyCount, 0))
        {
            LOG_ERROR("An Error occured, whilst querying for available Layers: ", res);
            LOG_QUER_AB("for available Layers");
            return false;
        }
        VkLayerProperties* pLayerProperties = new VkLayerProperties[*pLayerPropertyCount];
        //if (ppLayers) delete[] ppLayers;
        VKF(vkEnumerateInstanceLayerProperties(pLayerPropertyCount, pLayerProperties))
        {
            LOG_ERROR("An Error occured, whilst querying for available Layers: ", res);
            LOG_QUER_AB("for available Layers");
            return false;
        }
        //ppLayers = new const char*[*pLayerPropertyCount];
        std::vector<const char*> layers;
        LOG_INFO("-----AVAILABLE LAYERS-----");
        for (uint32_t i = 0; i < *pLayerPropertyCount; i++)
        {
            LOG_INFO("  ", pLayerProperties[i].layerName, ": ", pLayerProperties[i].description);
            layers.push_back(pLayerProperties[i].layerName);
            //ppLayers[i] = pLayerProperties[i].layerName;//std::move(pLayerProperties[i].layerName);
            //ppLayers[i] = pLayerProperties[i].layerName;
        }
        LOG_INFO("--------------------------");
        delete[] pLayerProperties;
        if (pLayerCount = 0) delete pLayerPropertyCount;
        ppLayers = layers.data();
        LOG_QUER_OK("for available Layers");
        return true;
    }

    bool checkLayerAvailability(const char** ppLayers, uint32_t* pLayerCount)
    {
        LOG_ING("check", "layer availability");
        const char** ppAvailableLayers;
        uint32_t availableLayerCount = 0;
        if(!queryAvailableLayers(ppAvailableLayers, &availableLayerCount))
        {
            LOG_ING_AB("check", "layer availability");
            return false;
        }

        std::vector<const char*> layers;

        for (uint32_t i = 0; i < availableLayerCount; i++)
        {
            for (uint32_t j = 0; j < *pLayerCount; j++)
            {
                if (ppLayers[j] == ppAvailableLayers[i])
                {
                    LOG_INFO("  ", ppLayers[j], " - available");
                    layers.push_back(ppLayers[j]);
                    break;
                }
                else if (j == *pLayerCount-1)
                {
                    LOG_WARN("\t", ppLayers[j], " - unavailable, continuing");
                }
            }
        }

        delete[] ppAvailableLayers;
        delete[] ppLayers;

        layers.shrink_to_fit();
        ppLayers = layers.data();
        *pLayerCount = layers.size();
        LOG_ING_OK("check", "layer availability");
        return true;
    }

    bool createVulkanInstance(VContext* pContext, uint32_t extensionCount, const char** ppExtensions, uint32_t layerCount, const char** ppLayers, const char* pAppName, uint32_t appVersion)
    {
        LOG_INIT("Vulkan Instance");
        VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        appInfo.pApplicationName = pAppName;
        appInfo.applicationVersion = appVersion;
        appInfo.pEngineName = ENGINE_NAME;
        appInfo.engineVersion = ENGINE_VERSION;
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = extensionCount;
        createInfo.ppEnabledExtensionNames = ppExtensions;
        createInfo.enabledLayerCount = layerCount;
        createInfo.ppEnabledLayerNames = ppLayers;

        VKF(vkCreateInstance(&createInfo, nullptr, &pContext->instance))
        {
            LOG_ERROR("An error occured whilst creating Vulkan Instance: ", res);
            LOG_INIT_AB("Vulkan Instance");
            return false;
        }

        LOG_INIT_OK("Vulkan Instance");
        return true;
    }
}