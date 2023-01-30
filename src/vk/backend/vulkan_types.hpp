#ifndef __VULKAN_TYPES_H__
#define __VULKAN_TYPES_H__

#include "../../thesis.h"
#include "../../pch.h"
#include "../../engine/data/resource_base.hpp"

#ifndef MAX_FRAMES_IN_FLIGHT
#define MAX_FRAMES_IN_FLIGHT 2
#endif

namespace Ths::Vk
{
  //----- SystemContext -----

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

  typedef struct Queues
  {
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;
  } Queues;

  typedef struct SystemContext
  {
    VkInstance instance;

    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    
    QueueFamilyIndices QueueFamilyIndices;
    Queues queues;
  } SystemContext;

  //----- ApplicationContext -----

  typedef struct ApplicationContext
  {
    SystemContext& systemContext;

    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews
  } ApplicationContext;

  //----- RenderContext -----

  typedef struct RenderContext
  {
    ApplicationContext& applicationContext;

    std::vector<VkRenderPass> renderPasses;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame;
    bool frameBufferResized;
  } RenderContext;

  //----- SceneContext -----

  typedef struct SceneContext
  {
    RenderContext& renderContext;

    Ths::ecs::Coordinator crd;
    Ths::ecs::RenderSystem renderSystem = {.sceneContext = *this};
  } SceneContext;

  //----- DescriptorSets -----

  typedef struct DescriptorSetCreateInfo
  {
    VkDescriptorType type;
    VkShaderStageFlagBits stage;

    VkSampler* pSampler;
    VkBuffer* pBuffer;
    uint32_t bufferSize;
  } DescriptorSetCreateInfo;

  //----- Resources -----

  typedef struct GPUResource
  {
    virtual constexpr DescriptorSetCreateInfo getDescriptorSetCreateInfo() = 0;
  } GPUResource;

  typedef struct Mesh : public Resource
  {
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
  } Mesh;

  typedef struct Shader : public Resource
  {
    std::vector<char> code;
  } Shader;

  typedef struct Image : public Resource
  {
    VkImage image;
    VkDeviceMemory imageMemory;
    uint32_t mipLevels;
  } Image;

  typedef struct Texture : public Image, public GPUResource
  {
    VkSampler sampler;
    VkImageView view;

    virtual constexpr DescriptorSetCreateInfo getDescriptorSetCreateInfo() override
    {
      return {
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pSampler = &this->sampler,
        .pBuffer = nullptr,
        .bufferSize = 0,
      };
    }
  } Texture;

  typedef struct Material : public Resource
  {
    absl::flat_hash_map<uint32_t, GPUResource*> bindingToResourceMap =
    {
      {0, pAlbedoTexture},
    };

    Shader& vertexShader;
    Shader& fragmentShader;

    Texture* pAlbedoTexture;
  } Material;

  typedef struct Pipeline : public Resource
  {
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
  } Pipeline;

  //----- Components -----

  typedef struct Renderable
  {
    Mesh& mesh;
    Material& material;
    Pipeline& pipeline;
  } Renderable;
}

#endif // __VULKAN_TYPES_H__