#ifndef __VULKAN_BASE_H__
#define __VULKAN_BASE_H__

#include "vulkan_types.hpp"

#define VK_FAIL(val) if(VkResult res = (val); res != VK_SUCCESS)
#define VKF(val) VK_FAIL(val)
#define ASSERT_VULKAN(val) VKF(val) { LOG_CRIT("An error occured in an attempted vulkan call: ", ret); assert(false); }
#define VK(f) f
#define VKA(f) ASSERT_VULKAN(VK(f))

#define ARRAY_SIZE(v) (sizeof(v)/sizeof(v[0]))

#ifndef MAX_FRAMES_IN_FLIGHT
#define MAX_FRAMES_IN_FLIGHT 2
#endif

namespace Ths::Vk
{
  // Vulkan Creation

  // TODO: Change all VContext*/VulkanContext* to VContext&

  // Functions
  inline void imGuiCheckVkRes(VkResult err){
    if (err == VK_SUCCESS) return;
    LOG_ERROR("[imgui] Error: VkResult = ", static_cast<uint32_t>(err));
  }

  void createColorResources(VContext* pContext);
  VkSampleCountFlagBits getMaxUsableSampleCount(VContext* pContext);

  bool loadModel(VContext* pContext, MeshResource*& pMesh, const char* modelpath, const char* basedir);
  bool loadModel(VContext* pContext, MeshResource& mesh, const char* modelpath, const char* basedir);

  bool hasStencilComponent(VkFormat format);
  VkFormat findDepthFormat(VContext* pContext);
  VkFormat findSupportedFormat(VContext* pContext, const std::vector<VkFormat>& candidates, VkImageTiling tiling,
    VkFormatFeatureFlags features);
  bool createDepthResources(VContext* pContext);

  bool createTextureSampler(VContext* pContext, TextureResource& pTexture);
  VkImageView createImageView(VContext* pContext, VkImage image, VkFormat format,
    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevels = 1);
  bool createTextureImageView(VContext* pContext, TextureResource& pTexture);
  void copyBufferToImage(VContext* pContext, VkBuffer buffer, VkImage image, uint32_t w, uint32_t h);
  void transitionImageLayout(VContext* pContext, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
  bool createImage(VContext* pContext, uint32_t w, uint32_t h, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
    VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
  bool generateMipmaps(VContext* pContext, VkImage image, VkFormat format, int32_t w, int32_t h, uint32_t mipLevels);
  bool createTextureImage(VContext* pContext, ImageResource*& pImage, const char* filename);
  bool createTextureImage(VContext* pContext, TextureResource*& pTexture, const char* filename);
  bool createTextureImage(VContext* pContext, ImageResource& image, const char* filename);

  bool createDescriptorSets(VContext* pContext, GPUContext& context,
    std::vector<DescriptorSetInfo> descriptorSetInfos);
  bool createDescriptorPool(VContext* pContext, GPUContext& context,
    std::vector<DescriptorSetInfo> descriptorSetInfos);
  bool createUniformBuffers(VContext* pContext);
  bool createDescriptorSetLayout(VContext* pContext, GPUContext& context,
   std::vector<DescriptorSetInfo> descriptorSetInfos);

  VkCommandBuffer beginSingleTimeGraphicsCommands(VContext* pContext);
  void endSingleTimeGraphicsCommands(VContext* pContext, VkCommandBuffer& commandBuffer);
  VkCommandBuffer beginSingleTimeTransferCommands(VContext* pContext);
  void endSingleTimeTransferCommands(VContext* pContext, VkCommandBuffer& commandBuffer);
  VkCommandBuffer beginSingleTimeCommands(VContext* pContext, VkCommandPool pool);
  void endSingleTimeCommands(VContext* pContext, VkCommandBuffer& commandBuffer, VkCommandPool pool, VkQueue queue);
  bool createSyncObjects(VContext* pContext);
  bool beginCommandBuffer(VContext* pContext, VkCommandBuffer commandBuffer, uint32_t imageIndex);
  bool endCommandBuffer(VContext* pContext, VkCommandBuffer commandBuffer, uint32_t imageIndex);
  bool createCommandBuffers(VContext* pContext);
  VkResult createCommandPool(VContext* pContext, VkCommandPoolCreateInfo& poolInfo, VkCommandPool* commandPool);
  bool createCommandPools(VContext* pContext);

  int findMemoryType(VContext* pContext, uint32_t typeFilter, VkMemoryPropertyFlags properties);
  void copyBuffer(VContext* pContext, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  bool createBuffer(VContext* pContext, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
  bool createIndexBuffer(VContext* pContext, OContext& object);
  bool createVertexBuffer(VContext* pContext, OContext& object);
  bool createFramebuffers(VContext* pContext);
  bool createRenderPass(VContext* pContext, uint32_t idx);
  VkShaderModule createShaderModule(VulkanContext* pContext, const std::vector<char> code);
  VkShaderModule createShaderModule(VulkanContext* pContext, const ShaderResource& shader);
  std::vector<char> readFile(const std::string& filename);
  bool createGraphicsPipeline(VulkanContext* pContext, OContext& object, PipelineResource*& pPipeline, ShaderResource& vShader, ShaderResource& fShader);
  bool createGraphicsPipeline(VulkanContext* pContext, OContext& object, PipelineResource& pipeline, ShaderResource& vShader, ShaderResource& fShader);

  void cleanupSwapChain(VulkanContext* pContext);
  bool recreateSwapChain(VulkanContext* pContext, uint32_t win_width, uint32_t win_height, uint32_t imgs,
    VkPresentModeKHR preferredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR,
    VkFormat preferredFormat = VK_FORMAT_B8G8R8A8_SRGB , VkColorSpaceKHR preferredColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
  bool createImageViews(VulkanContext* pContext);
  bool createSwapChain(VulkanContext* pContext, uint32_t win_width, uint32_t win_height, uint32_t imgs,
    VkPresentModeKHR preferredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR,
    VkFormat preferredFormat = VK_FORMAT_B8G8R8A8_SRGB , VkColorSpaceKHR preferredColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
  VkExtent2D chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t win_width, uint32_t win_height);
  VkPresentModeKHR chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes,
    VkPresentModeKHR preferredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR);
  VkSurfaceFormatKHR chooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats,
    VkFormat preferredFormat = VK_FORMAT_B8G8R8A8_SRGB, VkColorSpaceKHR preferredColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
  bool createLogicalDevice(VulkanContext* pContext, VkPhysicalDeviceFeatures* pFeatures, std::vector<const char*>* pDeviceExtensions);
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
  uint32_t rateDeviceSuitability(VkPhysicalDeviceProperties* pProps, VkPhysicalDeviceFeatures* pFeatures);
  bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*>* pDeviceExtensions);
  bool isPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, VkPhysicalDeviceProperties* pProps,
    VkPhysicalDeviceFeatures* pFeatures, VkPhysicalDeviceFeatures* pRequiredFeatures, std::vector<const char*>* pDeviceExtensions);
  bool selectPhysicalDevice(VContext* context, VkPhysicalDeviceFeatures* pRequiredFeatures, std::vector<const char*>* pDeviceExtensions);

  void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
  VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
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

  // Resource Creation
  
  inline void createImageResource(VContext* pContext, ImageResource*& pImage, const char* path)
  {
    createTextureImage(pContext, pImage, path);
  }
  inline ImageResource* createImageResource(VContext* pContext, const char* path)
  {
    ImageResource* temp = {};
    createImageResource(pContext, temp, path);
    return temp;
  }
  inline void destroyImageResource(VContext* pContext, ImageResource*& pImage)
  {
    vkDestroyImage(pContext->device, pImage->image, nullptr);
    vkFreeMemory(pContext->device, pImage->imageMemory, nullptr);
  }

  inline void createTextureResource(VContext* pContext, TextureResource*& pTexture, const char* path)
  {
    createImageResource(pContext, reinterpret_cast<ImageResource*&>(pTexture), path);
    createTextureImageView(pContext, *pTexture);
    createTextureSampler(pContext, *pTexture);
  }
  inline TextureResource* createTextureResource(VContext* pContext, const char* path)
  {
    TextureResource* temp = {};
    createTextureResource(pContext, temp, path);
    return temp;
  }
  inline void destroyTextureResource(VContext* pContext, TextureResource*& pTexture)
  {
    vkDestroySampler(pContext->device, pTexture->sampler, nullptr);
    vkDestroyImageView(pContext->device, pTexture->view, nullptr);
    destroyImageResource(pContext, reinterpret_cast<ImageResource*&>(pTexture));
  }
}

#endif // __VULKAN_BASE_H__