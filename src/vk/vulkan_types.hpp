#ifndef __VULKAN_TYPES_H__
#define __VULKAN_TYPES_H__

#include "../pch.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_vulkan.h"

#ifndef MAX_FRAMES_IN_FLIGHT
#define MAX_FRAMES_IN_FLIGHT 2
#endif

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

  typedef struct Transform
  {
    glm::vec3 translation;
    glm::vec3 rotation;
    glm::vec3 scale;
  } Transform;

   typedef struct ImageResource : public Ths::Resource
  {
    VkImage image;
    VkDeviceMemory imageMemory;
    uint32_t mipLevels;
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

  typedef struct ShaderResource : public Ths::Resource
  {
    std::vector<char> code;

    inline ShaderResource(char* code, size_t size)
    {
      this->code = std::vector(
        code,
        code+size
      );
    }

    inline ShaderResource(std::vector<char> code)
    {
      this->code = code;
    }

    inline ShaderResource() { };
  } ShaderResource;

  typedef struct PipelineResource : public Ths::Resource
  {
    VkPipelineLayout layout;
    VkPipeline pipeline;
  } PipelineResource;


  typedef struct MaterialResource : public Ths::Resource
  {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess; // "inverse roughness"
  } MaterialResource;

  typedef struct WorldEnviorment : public Ths::Resource
  {
    glm::vec3 ambientColor;
    float ambientStrength;
  } WorldEnviorment;
  
  typedef struct GPUContext
  {
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
  } GPUContext;


  typedef struct SceneContext : public GPUContext
  {
    typedef struct GPUSceneData
    {
      WorldEnviorment enviorment;
      VkBuffer enviormentBuffer;
    } GPUSceneData;
    
    GPUSceneData gpuData;
  } SceneContext;

  typedef struct Mesh
  {
    const char* basepath;
    const char* path;
    MeshResource* pMesh;

    void applyTransform(Transform t) = delete;
  } Mesh;

  typedef struct UniformBuffer
  {
    std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT> buffer;
    std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT> memory;
    std::array<void*, MAX_FRAMES_IN_FLIGHT> mapped;
    VkDeviceSize bufferSize;
  } UniformBuffer;

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

    SceneContext sceneContext;
    UniformBuffer sceneUniform;
  } VContext;

  typedef struct SwapchainSupportDetails
  {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  } SwapchainSupportDetails;

  typedef struct UniformBufferObject
  {
    /*
      If need be align to 16 - possible with alignas(16)
    */
    glm::vec4 ambientLight = {0.93f, 0.88f, 0.71f, 0.1f};
    glm::vec4 lightPos = {-2.56f, 1.95f, 1.32f, 0.0f};
    glm::vec4 specular = {32.0f, 0.5f, 0.5f, 0.5f};
    glm::vec4 cameraPos;
    // glm::mat4 model;
    // glm::mat4 view;
    // glm::mat4 proj;
  } UniformBufferObject;

  typedef struct MeshPushConstants
  {
    glm::vec4 data;
    // glm::mat4 render_matrix;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
  } MeshPushConstants;


  typedef struct Material
  {
    const char* path;
    TextureResource* pTexture;

    ShaderResource* pVertexShader;
    ShaderResource* pFragmentShader;

    // inline Material(ShaderResource vertexShader, ShaderResource fragmentShader)
    //  : vertexShader(vertexShader), fragmentShader(fragmentShader)
    // {}

    inline void destroy(VContext* pContext)
    {
      // if (pTexture->sampler) vkDestroySampler(pContext->device, pTexture->sampler, nullptr);
      // pTexture->sampler = VK_NULL_HANDLE;
      // if (pTexture->view) vkDestroyImageView(pContext->device, pTexture->view, nullptr);
      // pTexture->view = VK_NULL_HANDLE;
      // if (pTexture->image) vkDestroyImage(pContext->device, pTexture->image, nullptr);
      // pTexture->image = VK_NULL_HANDLE;
      // if (pTexture->imageMemory) vkFreeMemory(pContext->device, pTexture->imageMemory, nullptr);
      // pTexture->imageMemory = VK_NULL_HANDLE;
      // delete pVertexShader;
      // delete pFragmentShader;
    }
  } Material;

  typedef struct DescriptorSetInfo
  {
  public:
    VkDescriptorPoolSize poolSize;
    Material* pMaterial;
    VkBuffer* pBuffer;
    VkShaderStageFlags stage;
    uint32_t bufferSize;
  } DescriptorSetInfo;

  typedef struct ObjectContext : public GPUContext
  {
    Material* material; // ? Change to ref
    Mesh* mesh; // ? Change to ref
    Transform* transform; // ? Change to ref

    PipelineResource* pPipeline;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    MaterialResource gpuMaterial;
    
    inline void destroy(VContext* pContext)
    {
      vkDestroyDescriptorSetLayout(pContext->device, descriptorSetLayout, nullptr);
      vkDestroyDescriptorPool(pContext->device, descriptorPool, nullptr);

      vkDestroyPipelineLayout(pContext->device, pPipeline->layout, nullptr);
      vkDestroyPipeline(pContext->device, pPipeline->pipeline, nullptr);

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