#include "vulkan_base.h"
#include "../pch.h"

namespace Ths::Vk
{
  int findMemoryType(VContext* pContext, uint32_t typeFilter, VkMemoryPropertyFlags properties)
  {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(pContext->physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
      if (typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        return i;
    
    LOG_ERROR("No suitable memory type found!");
    return -1;
  }

  void copyBuffer(VContext* pContext, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
  {
    VkCommandBuffer commandBuffer = beginSingleTimeTransferCommands(pContext);

    VkBufferCopy copyRegion {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    
    endSingleTimeTransferCommands(pContext, commandBuffer);
  }

  bool createBuffer(VContext* pContext, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
  {
    VkBufferCreateInfo bufferInfo {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    QueueFamilyIndices indices = findQueueFamilies(pContext->physicalDevice, pContext->surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.transferFamily.value()};
    bufferInfo.queueFamilyIndexCount = 2;
    bufferInfo.pQueueFamilyIndices = queueFamilyIndices;

    VKF(vkCreateBuffer(pContext->device, &bufferInfo, nullptr, &buffer))
    {
      LOG_ERROR("An error occured whilst creating vertex buffer: ", res);
      return false;
    }

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(pContext->device, buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(pContext, memoryRequirements.memoryTypeBits, properties);
    
    VKF(vkAllocateMemory(pContext->device, &allocInfo, nullptr, &bufferMemory))
    {
      LOG_ERROR("An error occured whilst allocating GPU memory: ", res);
      return false;
    }

    vkBindBufferMemory(pContext->device, buffer, bufferMemory, 0);

    return true;
  }

  bool createIndexBuffer(VContext* pContext, OContext& object)
  {
    VkDeviceSize bufferSize = sizeof(object.mesh->pMesh->indices[0]) * object.mesh->pMesh->indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(pContext, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      stagingBuffer, stagingBufferMemory);

    // ? maybe consider explicit flushing
    void* data;
    vkMapMemory(pContext->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, object.mesh->pMesh->indices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(pContext->device, stagingBufferMemory);

    createBuffer(pContext, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      object.indexBuffer, object.indexBufferMemory);

    
    copyBuffer(pContext, stagingBuffer, object.indexBuffer, bufferSize);

    vkDestroyBuffer(pContext->device, stagingBuffer, nullptr);
    vkFreeMemory(pContext->device, stagingBufferMemory, nullptr);

    return true;
  }

  bool createVertexBuffer(VContext* pContext, OContext& object)
  {
    VkDeviceSize bufferSize = sizeof(Vertex) * object.mesh->pMesh->vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(pContext, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      stagingBuffer, stagingBufferMemory);

    // ? maybe consider explicit flushing
    void* data;
    vkMapMemory(pContext->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, object.mesh->pMesh->vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(pContext->device, stagingBufferMemory);

    createBuffer(pContext, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      object.vertexBuffer, object.vertexBufferMemory);

    
    copyBuffer(pContext, stagingBuffer, object.vertexBuffer, bufferSize);

    vkDestroyBuffer(pContext->device, stagingBuffer, nullptr);
    vkFreeMemory(pContext->device, stagingBufferMemory, nullptr);

    return true;
  }

  bool createFramebuffers(VContext* pContext)
  {
    pContext->swapchainFramebuffers.resize(pContext->swapchainImageViews.size());
    for (size_t i = 0; i < pContext->swapchainImageViews.size(); i++)
    {
      std::array<VkImageView, 3> attachments = {
        pContext->colorImageView,
        pContext->depthImageView,
        pContext->swapchainImageViews[i]
        // pContext->swapchainImageViews[i],
        // pContext->depthImageView,
        // pContext->colorImageView
      };

      VkFramebufferCreateInfo framebufferInfo {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
      framebufferInfo.renderPass = pContext->renderPasses[0];
      framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
      framebufferInfo.pAttachments = attachments.data();
      framebufferInfo.width = pContext->swapchainExtent.width;
      framebufferInfo.height = pContext->swapchainExtent.height;
      framebufferInfo.layers = 1;

      VKF(vkCreateFramebuffer(pContext->device, &framebufferInfo, nullptr, &pContext->swapchainFramebuffers[i]))
      {
        LOG_ERROR("An error occured whilst creating framebuffer \"", i, "\": ", res);
        return false;
      }
    }
    return true;
  }

  bool createRenderPass(VContext* pContext, uint32_t idx)
  {
    LOG_INIT("Render Pass");
    VkAttachmentDescription colorAttachment {};
    colorAttachment.format = pContext->swapchainImageFormat;
    colorAttachment.samples = pContext->msaaSamples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Enable screen clearing
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment {};
    depthAttachment.format = findDepthFormat(pContext);
    depthAttachment.samples = pContext->msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Enable screen clearing
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = pContext->swapchainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference colorAttachmentResolveRef {};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;

    std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};

    VkRenderPassCreateInfo renderPassInfo {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    VkSubpassDependency dependency {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (pContext->renderPasses.size() <= idx) pContext->renderPasses.push_back(VK_NULL_HANDLE);

    VKF(vkCreateRenderPass(pContext->device, &renderPassInfo, nullptr, &pContext->renderPasses[idx]))
    {
      LOG_ERROR("An Error occured whilst creating a render pass: ", res);
      LOG_INIT_AB("Render Pass");
      return false;
    }
    LOG_INIT_OK("Render Pass");
    return true;
  }

  VkShaderModule createShaderModule(VulkanContext* pContext, const ShaderResource& shader)
  {
    return createShaderModule(pContext, shader.code);
  }

  VkShaderModule createShaderModule(VulkanContext* pContext, const std::vector<char> code)
  {
    VkShaderModuleCreateInfo createInfo {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    VKF(vkCreateShaderModule(pContext->device, &createInfo, nullptr, &shaderModule))
    {
      LOG_ERROR("An error occured whilst creating a VkShaderModule!");
    }
    return shaderModule;
  }

  std::vector<char> readFile(const std::string& filename)
  {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
      LOG_ERROR("Failed to open: \"", filename, "\"!");
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
#ifdef THESIS_VULKAN_EXTRA_DEBUG_INFO
    LOG_DEBUG("Loaded \"", filename, "\", size(", fileSize, ")");
#endif // THESIS_VULKAN_EXTRA_DEBUG_INFO
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
  }

  bool createGraphicsPipeline(VulkanContext* pContext, OContext& object, PipelineResource*& pPipeline, ShaderResource& vShader, ShaderResource& fShader)
  {
    pPipeline = new PipelineResource();
    return createGraphicsPipeline(pContext, object, *pPipeline, vShader, fShader);
  }
  bool createGraphicsPipeline(VulkanContext* pContext, OContext& object, PipelineResource& pipeline, ShaderResource& vShader, ShaderResource& fShader)
  {
    LOG_INIT("Graphics Pipeline");
    if (object.material->pVertexShader->code.size() == 0 || object.material->pFragmentShader->code.size() == 0)
    {
      LOG_ERROR("An error occured whilst loading shaders!");
      LOG_INIT_AB("Graphics Pipeline");
      return false;
    }

    VkShaderModule vertModule = createShaderModule(pContext, *object.material->pVertexShader);
    VkShaderModule fragModule = createShaderModule(pContext, *object.material->pFragmentShader);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
    
    // Defines input to graphics pipeline
    VkPipelineVertexInputStateCreateInfo vertexInputInfo {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Create Input assembler
    VkPipelineInputAssemblyStateCreateInfo inputAssembler {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssembler.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembler.primitiveRestartEnable = VK_FALSE;

    // Viewport, defines size and position of image in frame buffer
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(pContext->swapchainExtent.width);
    viewport.height = static_cast<float>(pContext->swapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor, can cut off parts of frame buffer
    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = pContext->swapchainExtent; // Set scissor to render whole frame buffer, and not cut off anything

    // Set viewport and scissors as dynamic state
    // otherwise use VkPipelineViewportStateCreateInfo
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizer.depthClampEnable = VK_FALSE; // If true, no fragments get discarded
    rasterizer.rasterizerDiscardEnable = VK_FALSE; // If true, no geometry is passed into the framebuffer
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // Defines how polygons are rendered. VK_POLYGON_MODE_LINE will render wireframe fe.
    rasterizer.lineWidth = 1.0f; // Describes thickness of lines. Highter than 1.0f requires the wideLines GPU feature
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // Bitmask for culling options
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // Specifies vertex order, to determine polygon facing
    rasterizer.depthBiasEnable = VK_FALSE; // Biases depthbuffer, normally disabled, somethimes used for shadowmaps
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisampling.sampleShadingEnable = VK_FALSE; // Enable for AA in textures
    multisampling.rasterizationSamples = pContext->msaaSamples;
    multisampling.minSampleShading = .2f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    // Set up color blending for, fe. translucency
    VkPipelineColorBlendAttachmentState colorBlendAttachment {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE; // Enable color blending
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    // Create color blending with afore defined color blending options
    VkPipelineColorBlendStateCreateInfo colorBlending {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates =
    {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &object.descriptorSetLayout;

    // Push constants
    VkPushConstantRange pushConstant;
    pushConstant.offset = 0;
    pushConstant.size = sizeof(MeshPushConstants);
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

    VKF(vkCreatePipelineLayout(pContext->device, &pipelineLayoutInfo, nullptr, &pipeline.layout))
    {
      LOG_ERROR("An Error occured whilst creating a VkPipelineLayout: ", res);
      LOG_INIT_AB("Graphics Pipeline");
      return false;
    }

    VkGraphicsPipelineCreateInfo pipelineInfo {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembler;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipeline.layout;
    pipelineInfo.renderPass = pContext->renderPasses[0];
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VKF(vkCreateGraphicsPipelines(pContext->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.pipeline))
    {
      LOG_ERROR("An error occured whilst creating a VkPipeline: ", res);
      LOG_INIT_AB("Graphics Pipeline");
      return false;
    }

    vkDestroyShaderModule(pContext->device, vertModule, nullptr);
    vkDestroyShaderModule(pContext->device, fragModule, nullptr);

    LOG_INIT_OK("Graphics Pipeline");
    return true;
  }

  // bool createGraphicsPipeline(VulkanContext* pContext)
  // {
  //   LOG_INIT("Graphics Pipeline");
  //   // auto vertShaderCode = readFile("D:/Projects/Thesis/src/vk/shaders/vert.spv");
  //   auto vertShaderCode = readFile("src/vk/shaders/vert.spv");
  //   // auto fragShaderCode = readFile("D:/Projects/Thesis/src/vk/shaders/frag.spv");
  //   auto fragShaderCode = readFile("src/vk/shaders/frag.spv");
  //   if (vertShaderCode.size() == 0 || fragShaderCode.size() == 0)
  //   {
  //     LOG_ERROR("An error occured whilst loading shaders!");
  //     LOG_INIT_AB("Graphics Pipeline");
  //     return false;
  //   }

  //   VkShaderModule vertModule = createShaderModule(pContext, vertShaderCode);
  //   VkShaderModule fragModule = createShaderModule(pContext, fragShaderCode);

  //   VkPipelineShaderStageCreateInfo vertShaderStageInfo {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
  //   vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  //   vertShaderStageInfo.module = vertModule;
  //   vertShaderStageInfo.pName = "main";

  //   VkPipelineShaderStageCreateInfo fragShaderStageInfo {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
  //   fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  //   fragShaderStageInfo.module = fragModule;
  //   fragShaderStageInfo.pName = "main";

  //   VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
    
  //   // Defines input to graphics pipeline
  //   VkPipelineVertexInputStateCreateInfo vertexInputInfo {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
  //   auto bindingDescription = Vertex::getBindingDescription();
  //   auto attributeDescriptions = Vertex::getAttributeDescriptions();
  //   vertexInputInfo.vertexBindingDescriptionCount = 1;
  //   vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
  //   vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  //   vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

  //   // Create Input assembler
  //   VkPipelineInputAssemblyStateCreateInfo inputAssembler {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  //   inputAssembler.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  //   inputAssembler.primitiveRestartEnable = VK_FALSE;

  //   // Viewport, defines size and position of image in frame buffer
  //   VkViewport viewport{};
  //   viewport.x = 0.0f;
  //   viewport.y = 0.0f;
  //   viewport.width = static_cast<float>(pContext->swapchainExtent.width);
  //   viewport.height = static_cast<float>(pContext->swapchainExtent.height);
  //   viewport.minDepth = 0.0f;
  //   viewport.maxDepth = 1.0f;

  //   // Scissor, can cut off parts of frame buffer
  //   VkRect2D scissor {};
  //   scissor.offset = {0, 0};
  //   scissor.extent = pContext->swapchainExtent; // Set scissor to render whole frame buffer, and not cut off anything

  //   // Set viewport and scissors as dynamic state
  //   // otherwise use VkPipelineViewportStateCreateInfo
  //   VkPipelineViewportStateCreateInfo viewportState{};
  //   viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  //   viewportState.viewportCount = 1;
  //   viewportState.pViewports = &viewport;
  //   viewportState.scissorCount = 1;
  //   viewportState.pScissors = &scissor;

  //   VkPipelineRasterizationStateCreateInfo rasterizer {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  //   rasterizer.depthClampEnable = VK_FALSE; // If true, no fragments get discarded
  //   rasterizer.rasterizerDiscardEnable = VK_FALSE; // If true, no geometry is passed into the framebuffer
  //   rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // Defines how polygons are rendered. VK_POLYGON_MODE_LINE will render wireframe fe.
  //   rasterizer.lineWidth = 1.0f; // Describes thickness of lines. Highter than 1.0f requires the wideLines GPU feature
  //   rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // Bitmask for culling options
  //   rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // Specifies vertex order, to determine polygon facing
  //   rasterizer.depthBiasEnable = VK_FALSE; // Biases depthbuffer, normally disabled, somethimes used for shadowmaps
  //   rasterizer.depthBiasConstantFactor = 0.0f;
  //   rasterizer.depthBiasClamp = 0.0f;
  //   rasterizer.depthBiasSlopeFactor = 0.0f;

  //   // No multisampling for now
  //   VkPipelineMultisampleStateCreateInfo multisampling {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  //   multisampling.sampleShadingEnable = VK_FALSE; // Enable for AA in textures
  //   multisampling.rasterizationSamples = pContext->msaaSamples;
  //   multisampling.minSampleShading = .2f;
  //   multisampling.pSampleMask = nullptr;
  //   multisampling.alphaToCoverageEnable = VK_FALSE;
  //   multisampling.alphaToOneEnable = VK_FALSE;

  //   VkPipelineDepthStencilStateCreateInfo depthStencil{};
  //   depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  //   depthStencil.depthTestEnable = VK_TRUE;
  //   depthStencil.depthWriteEnable = VK_TRUE;
  //   depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
  //   depthStencil.depthBoundsTestEnable = VK_FALSE;
  //   depthStencil.minDepthBounds = 0.0f;
  //   depthStencil.maxDepthBounds = 1.0f;
  //   depthStencil.stencilTestEnable = VK_FALSE;
  //   depthStencil.front = {};
  //   depthStencil.back = {};

  //   // Set up color blending for, fe. translucency
  //   VkPipelineColorBlendAttachmentState colorBlendAttachment {};
  //   colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  //   colorBlendAttachment.blendEnable = VK_TRUE; // Enable color blending
  //   colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  //   colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  //   colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  //   colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  //   colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  //   colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

  //   // Create color blending with afore defined color blending options
  //   VkPipelineColorBlendStateCreateInfo colorBlending {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
  //   colorBlending.logicOpEnable = VK_FALSE;
  //   colorBlending.logicOp = VK_LOGIC_OP_COPY;
  //   colorBlending.attachmentCount = 1;
  //   colorBlending.pAttachments = &colorBlendAttachment;
  //   colorBlending.blendConstants[0] = 0.0f;
  //   colorBlending.blendConstants[1] = 0.0f;
  //   colorBlending.blendConstants[2] = 0.0f;
  //   colorBlending.blendConstants[3] = 0.0f;

  //   std::vector<VkDynamicState> dynamicStates =
  //   {
  //     VK_DYNAMIC_STATE_VIEWPORT,
  //     VK_DYNAMIC_STATE_SCISSOR
  //   };

  //   VkPipelineDynamicStateCreateInfo dynamicState {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
  //   dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  //   dynamicState.pDynamicStates = dynamicStates.data();

  //   VkPipelineLayoutCreateInfo pipelineLayoutInfo {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
  //   pipelineLayoutInfo.setLayoutCount = 1;
  //   pipelineLayoutInfo.pSetLayouts = &pContext->descriptorSetLayout;
  //   pipelineLayoutInfo.pushConstantRangeCount = 0;
  //   pipelineLayoutInfo.pPushConstantRanges = nullptr;

  //   VKF(vkCreatePipelineLayout(pContext->device, &pipelineLayoutInfo, nullptr, &pContext->pipelineLayout))
  //   {
  //     LOG_ERROR("An Error occured whilst creating a VkPipelineLayout: ", res);
  //     LOG_INIT_AB("Graphics Pipeline");
  //     return false;
  //   }

  //   VkGraphicsPipelineCreateInfo pipelineInfo {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
  //   pipelineInfo.stageCount = 2;
  //   pipelineInfo.pStages = shaderStages;
  //   pipelineInfo.pVertexInputState = &vertexInputInfo;
  //   pipelineInfo.pInputAssemblyState = &inputAssembler;
  //   pipelineInfo.pViewportState = &viewportState;
  //   pipelineInfo.pRasterizationState = &rasterizer;
  //   pipelineInfo.pMultisampleState = &multisampling;
  //   pipelineInfo.pDepthStencilState = &depthStencil;
  //   pipelineInfo.pColorBlendState = &colorBlending;
  //   pipelineInfo.pDynamicState = &dynamicState;
  //   pipelineInfo.layout = pContext->pipelineLayout;
  //   pipelineInfo.renderPass = pContext->renderPasses[0];
  //   pipelineInfo.subpass = 0;
  //   pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  //   pipelineInfo.basePipelineIndex = -1;

  //   VKF(vkCreateGraphicsPipelines(pContext->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pContext->graphicsPipeline))
  //   {
  //     LOG_ERROR("An error occured whilst creating a VkPipeline: ", res);
  //     LOG_INIT_AB("Graphics Pipeline");
  //     return false;
  //   }

  //   vkDestroyShaderModule(pContext->device, vertModule, nullptr);
  //   vkDestroyShaderModule(pContext->device, fragModule, nullptr);

  //   LOG_INIT_OK("Graphics Pipeline");
  //   return true;
  // }
} // namespace Ths::Vk
