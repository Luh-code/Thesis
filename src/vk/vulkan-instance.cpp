#include "vulkan-base.h"

namespace Ths::Vk
{
    bool queryAvailableLayers(const char** ppLayers, uint32_t* pLayerCount)
    {
        LOG_QUER("for available Layers");
        uint32_t* pLayerPropertyCount;
        if (pLayerCount != 0) pLayerPropertyCount = pLayerCount;
        else pLayerPropertyCount = new uint32_t();
        VKF(vkEnumerateInstanceLayerProperties(pLayerPropertyCount, 0))
        {
            LOG_ERROR("An Error occured, whilst querying for available Layers: ", res);
            LOG_QUER_AB("for available Layers");
            return false;
        }
        VkLayerProperties* pLayerProperties = new VkLayerProperties[*pLayerPropertyCount];
        VKF(vkEnumerateInstanceLayerProperties(pLayerPropertyCount, pLayerProperties))
        {
            LOG_ERROR("An Error occured, whilst querying for available Layers: ", res);
            LOG_QUER_AB("for available Layers");
            return false;
        }
        std::vector<const char*> layers;
        LOG_INFO("AVAILABLE LAYERS:");
        for (uint32_t i = 0; i < *pLayerPropertyCount; i++)
        {
            /*
            TODO: Fix formatting
            is:
              alayer: description
              anotherlayer: description
            should be:
              alayer:       description
              anotherlayer: description
            */
            LOG_INFO("  ", pLayerProperties[i].layerName, ": ", pLayerProperties[i].description);
            layers.push_back(pLayerProperties[i].layerName);
        }
        delete[] pLayerProperties;
        if (pLayerCount = 0) delete pLayerPropertyCount;
        ppLayers = std::move(layers.data());
        LOG_QUER_OK("for available Layers");
        return true;
    }

    bool checkLayerAvailability(std::vector<const char*>* pLayers, uint32_t* pLayerCount)
    {
        LOG_ING("check", "layer availability");
        const char** ppAvailableLayers;
        uint32_t availableLayerCount = 0;
        if(!queryAvailableLayers(ppAvailableLayers, &availableLayerCount))
        {
            LOG_ING_AB("check", "layer availability");
            return false;
        }

        //std::vector<const char*> layers;
        //for (uint32_t i = 0; i < *pLayerCount; i++)
        for (std::vector<const char*>::iterator it = pLayers->begin(); it != pLayers->end(); it++)
        {
            for (uint32_t j = 0; j < availableLayerCount; j++)
            {
                if (*it == ppAvailableLayers[j])
                {
                    LOG_INFO("  ", *it, " - available");
                    //layers.push_back(ppLayers[i]);
                    break;
                }
                else if (!(j < availableLayerCount))
                {
                    LOG_WARN("  ", *it, " - unavailable, continuing");
                    it = pLayers->erase(it);
                    it--;
                }
            }
        }

        //ppLayers = std::copy(layers.data());
        //std::copy(layers.begin(), layers.end(), ppLayers);
        //LOG_DEBUG(pLayers->at(0));
        //delete[] *ppAvailableLayers;
        //LOG_DEBUG(pLayers->at(0));

        pLayers->shrink_to_fit();
        *pLayerCount = pLayers->size();
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
        LOG_DEBUG(ppLayers[0]);
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