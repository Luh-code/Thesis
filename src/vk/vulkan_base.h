#ifndef __VULKAN_BASE_H__
#define __VULKAN_BASE_H__

#include "../pch.h"

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
  // Data

  // bonk
  // static const std::string MODEL_PATH = "assets/models/2bonk.obj";
  // static const std::string TEXTURE_PATH = "assets/textures/2butterfly.png";

  // static const std::string BASE_PATH = "D:/Projects/Thesis/";
  static const std::string BASE_PATH = "";
  static const std::string MODEL_PATH = BASE_PATH + "assets/models/Viking.obj";
  static const std::string TEXTURE_PATH = BASE_PATH + "assets/textures/Viking.png";

  typedef struct Vertex
  {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static inline std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
    {
      std::array<VkVertexInputAttributeDescription, 3> ad {};
      ad[0].binding = 0;
      ad[0].location = 0;
      ad[0].format = VK_FORMAT_R32G32B32_SFLOAT; //aka vec3
      ad[0].offset = offsetof(Vertex, pos);

      ad[1].binding = 0;
      ad[1].location = 1;
      ad[1].format = VK_FORMAT_R32G32B32_SFLOAT;
      ad[1].offset = offsetof(Vertex, color);

      ad[2].binding = 0;
      ad[2].location = 2;
      ad[2].format = VK_FORMAT_R32G32_SFLOAT; //aka vec2
      ad[2].offset = offsetof(Vertex, texCoord);

      return ad;
    }

    static inline VkVertexInputBindingDescription getBindingDescription()
    {
      VkVertexInputBindingDescription bd {};
      bd.binding = 0;
      bd.stride = sizeof(Vertex);
      bd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Change to VK_VERTEX_INPUT_RATE_INSTANCE for instanced rendering

      return bd;
    }
  } Vertex;

  typedef struct VulkanContext
  {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    VkPhysicalDevice physicalDevice;
    VkDevice device = VK_NULL_HANDLE;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;

    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
    std::vector<VkImageView> swapchainImageViews;

    std::vector<VkRenderPass> renderPasses;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapchainFramebuffers;

    VkCommandPool commandPool;
    VkCommandPool transferPool;
    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;
    bool framebufferResized = false;

    std::vector<Vertex> verticies;
    std::vector<uint32_t> indices;
    VkBuffer vertexBuffer;
    VkBuffer indexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkDeviceMemory indexBufferMemory;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    uint32_t mipLevels;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_64_BIT;
    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;
  } VContext;

  typedef struct QueueFamilyIndices
  {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> transferFamily;

    inline bool isComplete()
    {
      uint32_t arrSize = sizeof(QueueFamilyIndices)/sizeof(std::optional<uint32_t>);
      std::optional<uint32_t>* pThisArr = reinterpret_cast<std::optional<uint32_t>*>(this);
      for (uint32_t i = 0; i < arrSize; i++)
      {
        if (!pThisArr->has_value()) return false;
        pThisArr++;
      }
      return true;
    }
  } QueueFamilyIndices;

  typedef struct SwapChainSupportDetails
  {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  } SwapChainSupportDetails;

  typedef struct UniformBufferObject
  {
    /*
      If need be align to 16 - possible with alignas(16)
    */
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  } UniformBufferObject;

  typedef struct Mesh
  {
    const char* basepath;
    const char* path;
    std::vector<Ths::Vk::Vertex> verticies;
    std::vector<uint32_t> indices;
  } Mesh;

  typedef struct Shader
  {
    const size_t size;
    const uint32_t* code;

    inline Shader(const uint32_t* code, size_t size)
     : size(size), code(code)
    { }

    inline Shader(std::vector<uint32_t> code)
     : size(code.size()), code(code.data())
    { }
  } Shader;

  typedef struct Material
  {
    const char* path;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;

    Shader* vertexShader;
    Shader* fragmentShader;

    // inline Material(Shader vertexShader, Shader fragmentShader)
    //  : vertexShader(vertexShader), fragmentShader(fragmentShader)
    // {}

    inline void destroy(VContext* pContext)
    {
      vkDestroyImage(pContext->device, textureImage, nullptr);
      vkFreeMemory(pContext->device, textureImageMemory, nullptr);

      delete vertexShader;
      delete fragmentShader;
    }
  } Material;

  typedef struct ObjectContext
  {
    Material* material;
    Mesh* mesh;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    
    VkImageView textureImageView;
    VkSampler textureSampler;

    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    std::vector<VkCommandBuffer> commandBuffers;
    VkCommandPool commandPool;
    
    inline void destroy(VContext* pContext)
    {
      vkDestroyDescriptorSetLayout(pContext->device, descriptorSetLayout, nullptr);
      // vkFreeDescriptorSets(pContext->device, descriptorPool, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data());
      vkDestroyDescriptorPool(pContext->device, descriptorPool, nullptr);
      
      vkDestroyImageView(pContext->device, textureImageView, nullptr);
      vkDestroySampler(pContext->device, textureSampler, nullptr);

      vkDestroyPipelineLayout(pContext->device, pipelineLayout, nullptr);
      vkDestroyPipeline(pContext->device, graphicsPipeline, nullptr);

      vkDestroyBuffer(pContext->device, vertexBuffer, nullptr);
      vkFreeMemory(pContext->device, vertexBufferMemory, nullptr);
      vkDestroyBuffer(pContext->device, indexBuffer, nullptr);
      vkFreeMemory(pContext->device, indexBufferMemory, nullptr);

      vkFreeCommandBuffers(pContext->device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
      vkDestroyCommandPool(pContext->device, commandPool, nullptr);
    }
  } OContext;

  // TODO: Change all VContext*/VulkanContext* to VContext&

  // Functions
  void createColorResources(VContext* pContext);
  VkSampleCountFlagBits getMaxUsableSampleCount(VContext* pContext);

  bool loadModel(VContext* pContext, OContext& object, const char* modelpath, const char* basedir);

  bool hasStencilComponent(VkFormat format);
  VkFormat findDepthFormat(VContext* pContext);
  VkFormat findSupportedFormat(VContext* pContext, const std::vector<VkFormat>& candidates, VkImageTiling tiling,
    VkFormatFeatureFlags features);
  bool createDepthResources(VContext* pContext);

  bool createTextureSampler(VContext* pContext, OContext& object);
  VkImageView createImageView(VContext* pContext, VkImage image, VkFormat format,
    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevels = 1);
  bool createTextureImageView(VContext* pContext, OContext& object);
  void copyBufferToImage(VContext* pContext, VkBuffer buffer, VkImage image, uint32_t w, uint32_t h);
  void transitionImageLayout(VContext* pContext, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
  bool createImage(VContext* pContext, uint32_t w, uint32_t h, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
    VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
  bool generateMipmaps(VContext* pContext, VkImage image, VkFormat format, int32_t w, int32_t h, uint32_t mipLevels);
  bool createTextureImage(VContext* pContext, OContext& object, const char* filename);

  bool createDescriptorSets(VContext* pContext, OContext& object);
  bool createDescriptorPool(VContext* pContext, OContext& object);
  bool createUniformBuffers(VContext* pContext);
  bool createDescriptorSetLayout(VContext* pContext, OContext& object);

  VkCommandBuffer beginSingleTimeGraphicsCommands(VContext* pContext);
  void endSingleTimeGraphicsCommands(VContext* pContext, VkCommandBuffer& commandBuffer);
  VkCommandBuffer beginSingleTimeTransferCommands(VContext* pContext);
  void endSingleTimeTransferCommands(VContext* pContext, VkCommandBuffer& commandBuffer);
  VkCommandBuffer beginSingleTimeCommands(VContext* pContext, VkCommandPool pool);
  void endSingleTimeCommands(VContext* pContext, VkCommandBuffer& commandBuffer, VkCommandPool pool, VkQueue queue);
  bool createSyncObjects(VContext* pContext);
  bool recordCommandBuffer(VContext* pContext, OContext& object, VkCommandBuffer commandBuffer, uint32_t imageIndex);
  bool createCommandBuffers(VContext* pContext, OContext& object);
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
  VkShaderModule createShaderModule(VulkanContext* pContext, const Shader& shader);
  std::vector<char> readFile(const std::string& filename);
  bool createGraphicsPipeline(VulkanContext* pContext, OContext& object);

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
}

template <>
struct std::hash<Ths::Vk::Vertex>
{
  inline size_t vec(const glm::vec2& vec) const
  {
    size_t res = 17;
    res = res * 31 + hash<float>()(vec.x);
    res = res * 31 + hash<float>()(vec.y);
    return res;
  }

  inline size_t vec(const glm::vec3& vec) const
  {
    size_t res = 17;
    res = res * 31 + hash<float>()(vec.x);
    res = res * 31 + hash<float>()(vec.y);
    res = res * 31 + hash<float>()(vec.z);
    return res;
  }

  size_t operator()(const Ths::Vk::Vertex& v) const
  {
    size_t res = 17;
    res = res * 31 + vec(v.pos);
    res = res * 31 + vec(v.color);
    res = res * 31 + vec(v.texCoord);
    return res;
  }
};

template <>
struct std::equal_to<Ths::Vk::Vertex>
{
  inline bool operator()(const Ths::Vk::Vertex& l, const Ths::Vk::Vertex& r) const
  {
    return std::hash<Ths::Vk::Vertex>()(l) == std::hash<Ths::Vk::Vertex>()(r);
  }
};


#endif // __VULKAN_BASE_H__