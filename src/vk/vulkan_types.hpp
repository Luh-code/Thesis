#ifndef __VULKAN_TYPES_H__
#define __VULKAN_TYPES_H__

#include "../pch.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_vulkan.h"

namespace Ths::Vk
{
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

  typedef struct Transform
  {
    glm::vec3 translation;
    glm::vec3 rotation;
    glm::vec3 scale;
  } Transform;

  typedef struct MeshPushConstants
  {
    glm::vec4 data;
    glm::mat4 render_matrix;
  } MeshPushConstants;

  typedef struct ImageResource : public Ths::Resource
  {
    VkImage image;
    VkDeviceMemory imageMemory;
  } ImageResource;

  typedef struct TextureResource : public ImageResource
  {
    VkSampler sampler;
    VkImageView view;
  } TextureResource;

  typedef struct MeshResource : public Ths::Resource
  {
    std::vector<Ths::Vk::Vertex> vertices;
    std::vector<uint32_t> indices;
  } MeshResource;

  typedef struct Shader
  {
    std::vector<char> code;

    inline Shader(char* code, size_t size)
    {
      this->code = std::vector(
        code,
        code+size
      );
    }

    inline Shader(std::vector<char> code)
    {
      this->code = code;
    }
  } Shader;

  typedef struct Mesh
  {
    const char* basepath;
    const char* path;
    MeshResource* pMesh;

    void applyTransform(Transform t) = delete;
  } Mesh;

  typedef struct Material
  {
    const char* path;
    TextureResource* pTexture;

    Shader* vertexShader;
    Shader* fragmentShader;

    // inline Material(Shader vertexShader, Shader fragmentShader)
    //  : vertexShader(vertexShader), fragmentShader(fragmentShader)
    // {}

    inline void destroy(VContext* pContext)
    {
      vkDestroySampler(pContext->device, pTexture->sampler, nullptr);
      vkDestroyImageView(pContext->device, pTexture->view, nullptr);
      vkDestroyImage(pContext->device, pTexture->image, nullptr);
      vkFreeMemory(pContext->device, pTexture->imageMemory, nullptr);

      delete vertexShader;
      delete fragmentShader;
    }
  } Material;

  typedef struct ObjectContext
  {
    Material* material; // ? Change to ref
    Mesh* mesh; // ? Change to ref
    Transform* transform; // ? Change to ref

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
    
    inline void destroy(VContext* pContext)
    {
      vkDestroyDescriptorSetLayout(pContext->device, descriptorSetLayout, nullptr);
      vkDestroyDescriptorPool(pContext->device, descriptorPool, nullptr);
      
      vkDestroyImageView(pContext->device, textureImageView, nullptr);
      vkDestroySampler(pContext->device, textureSampler, nullptr);

      vkDestroyPipelineLayout(pContext->device, pipelineLayout, nullptr);
      vkDestroyPipeline(pContext->device, graphicsPipeline, nullptr);

      vkDestroyBuffer(pContext->device, vertexBuffer, nullptr);
      vkFreeMemory(pContext->device, vertexBufferMemory, nullptr);
      vkDestroyBuffer(pContext->device, indexBuffer, nullptr);
      vkFreeMemory(pContext->device, indexBufferMemory, nullptr);
    }
  } OContext;

  typedef struct ImGuiData
  {
    ImGuiIO& io;
    VkPipelineCache pipelineCache;
    VkDescriptorPool descriptorPool;
  } ImGuiData;
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

#endif // __VULKAN_TYPES_H__