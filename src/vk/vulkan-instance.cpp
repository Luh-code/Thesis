#include "vulkan-base.h"

namespace Ths::Vk
{
    bool addDebugging(std::vector<const char*>* pLayers, std::vector<const char*>* pExtensions, bool shrink)
    {
        const char* validationLayer = "VK_LAYER_KHRONOS_validation";
        if (pLayers->size() > 0) {
            for (std::vector<const char*>::iterator i = pLayers->begin(); strcmp(*i, validationLayer) != 0; i++)
                if (i == pLayers->end()) {
            pLayers->push_back(validationLayer);
            break;
        }} else pLayers->push_back(validationLayer);

        const char* debugExtension = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        if (pExtensions->size() > 0) {
            for (std::vector<const char*>::iterator i = pExtensions->begin(); strcmp(*i, debugExtension) != 0; i++)
                if (i == pExtensions->end()) {
            pExtensions->push_back(debugExtension);
            break;
        }} else pExtensions->push_back(debugExtension);

        if (shrink)
        {
            pLayers->shrink_to_fit();
            pExtensions->shrink_to_fit();
        }
        return true;
    }

    bool queryAvailableLayers(std::vector<char*>* pLayers)
    {
        LOG_QUER("for available Layers");
        uint32_t pLayerPropertyCount = 0;// = pLayers->size();
        VKF(vkEnumerateInstanceLayerProperties(&pLayerPropertyCount, 0))
        {
            LOG_ERROR("An Error occured, whilst querying for available Layers: ", res);
            LOG_QUER_AB("for available Layers");
            return false;
        }
        VkLayerProperties* pLayerProperties = new VkLayerProperties[pLayerPropertyCount];
        VKF(vkEnumerateInstanceLayerProperties(&pLayerPropertyCount, pLayerProperties))
        {
            LOG_ERROR("An Error occured, whilst querying for available Layers: ", res);
            LOG_QUER_AB("for available Layers");
            return false;
        }
        LOG_INFO("AVAILABLE LAYERS:");
        for (uint32_t i = 0; i < pLayerPropertyCount; i++)
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
            if (pLayers)
            {
                size_t len = strlen(pLayerProperties[i].layerName)+1;
                pLayers->push_back(new char[len]);
                memcpy(pLayers->at(pLayers->size()-1), static_cast<const char*>(pLayerProperties[i].layerName), len);
            }
        }
        delete[] pLayerProperties;
        LOG_QUER_OK("for available Layers");
        return true;
    }

    bool checkLayerAvailability(std::vector<const char*>* pLayers)
    {
        LOG_ING("check", "layer availability");
        std::vector<char*> availableLayers;
        if(!queryAvailableLayers(&availableLayers))
        {
            LOG_ING_AB("check", "layer availability");
            return false;
        }

        //std::vector<const char*> layers;
        //for (uint32_t i = 0; i < *pLayerCount; i++)
        for (std::vector<const char*>::iterator it = pLayers->begin(); it != pLayers->end(); it++)
        {
            for (std::vector<char*>::iterator jt = availableLayers.begin(); jt != availableLayers.end(); jt++)
            {
                //LOG_DEBUG("  |", *it, " - ", *jt, "|");
                if (strcmp(*it, *jt) == 0)
                {
                    LOG_INFO("  ", *it, " - available");
                    //layers.push_back(ppLayers[i]);
                    break;
                }
                else if (jt+1 == availableLayers.end())
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
        //delete[] ppAvailableLayers;
        //LOG_DEBUG(pLayers->at(0));

        pLayers->shrink_to_fit();
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
        //LOG_DEBUG(ppLayers[0]);
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