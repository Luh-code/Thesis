#include "vulkan_base.h"

namespace Ths::Vk
{
  VkShaderModule createShaderModule(VulkanContext* pContext, const std::vector<char>& code)
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

  static std::vector<char> readFile(const std::string& filename)
  {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
      LOG_ERROR("Failed to open: \"", filename, "\"!");
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    LOG_DEBUG("Loaded \"", filename, "\", size(", fileSize, ")");
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
  }

  bool createGraphicsPipeline(VulkanContext* pContext)
  {
    LOG_INIT("Graphics Pipeline");
    auto vertShaderCode = readFile("D:/Projects/Thesis/src/vk/shaders/vert.spv");
    auto fragShaderCode = readFile("D:/Projects/Thesis/src/vk/shaders/frag.spv");
    if (vertShaderCode.size() == 0 || fragShaderCode.size() == 0)
    {
      LOG_ERROR("An error occured whilst loading shaders!");
      LOG_INIT_AB("Graphics Pipeline");
      return false;
    }

    VkShaderModule vertModule = createShaderModule(pContext, vertShaderCode);
    VkShaderModule fragModule = createShaderModule(pContext, fragShaderCode);

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
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

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
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // Specifies vertex order, to determine polygon facing
    rasterizer.depthBiasEnable = VK_FALSE; // Biases depthbuffer, normally disabled, somethimes used for shadowmaps
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    // No multisampling for now
    VkPipelineMultisampleStateCreateInfo multisampling {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    // TODO: Add depth/stencil buffer
    // * Use VkPipelineDepthStencilStateCreateInfo for this ^

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
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    VKF(vkCreatePipelineLayout(pContext->device, &pipelineLayoutInfo, nullptr, &pContext->pipelineLayout))
    {
      LOG_ERROR("An Error occured whilst creating a VkPipelineLayout: ", res);
      LOG_INIT_AB("Graphics Pipeline");
      return false;
    }

    vkDestroyShaderModule(pContext->device, vertModule, nullptr);
    vkDestroyShaderModule(pContext->device, fragModule, nullptr);

    LOG_INIT_OK("Graphics Pipeline");
    return true;
  }
} // namespace Ths::Vk
