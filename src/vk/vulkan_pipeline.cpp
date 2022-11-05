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
      LOG_ERROR("Failed to open: \n", filename, "\"!");
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
    auto vertShaderCode = readFile("shaders/vert.spv");
    auto fragShaderCode = readFile("shaders/frag.spv");
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

    // TODO: Continue here tomorrow!!!

    LOG_INIT_OK("Graphics Pipeline");
    return true;
  }
} // namespace Ths::Vk
