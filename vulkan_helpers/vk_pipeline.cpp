#include "vk_context.h"
#include "vk_pipeline.h"
#include "vk_utils.h"
#include "vk_functions.h"

#include <map>

void FPipeline::AddShader(const std::string& Path, eShaderType ShaderType)
{
    ShaderModules.push_back({CreateShaderFromFile(Path), ShaderType});
}

void FPipeline::AddVertexInputBindingDescription(const VkVertexInputBindingDescription& VertexInputBindingDescription)
{
    VertexInputBindingDescriptionVector.push_back(VertexInputBindingDescription);
}

void FPipeline::AddVertexInputAttributeDescription(const VkVertexInputAttributeDescription& VertexInputAttributeDescription)
{
    VertexInputAttributeDescriptionVector.push_back(VertexInputAttributeDescription);
}

void FPipeline::SetMSAA(VkSampleCountFlagBits SampleCountFlagBits)
{
    MSAASamples = SampleCountFlagBits;
}

void FPipeline::SetWidth(uint32_t Wdth)
{
    Width = Wdth;
}

void FPipeline::SetHeight(uint32_t Hght)
{
    Height = Hght;
}

void FPipeline::SetBlendAttachmentsCount(uint32_t Count)    ///TODO: Find a better solution
{
    BlendCount = Count;
}

void FPipeline::SetExtent2D(VkExtent2D Extent2D)
{
    Extent = Extent2D;
}

void FPipeline::AddDescriptorSetLayout(VkDescriptorSetLayout DescriptorSetLayout)
{
    DescriptorSetLayouts.push_back(DescriptorSetLayout);
}

VkPipelineLayout FPipeline::GetPipelineLayout()
{
    return PipelineLayout;
}

VkPipeline FPipeline::GetPipeline()
{
    return Pipeline;
}

static std::map<eShaderType, VkShaderStageFlagBits>  ShaderTypesMap =
        {
                {eShaderType::VERTEX, VK_SHADER_STAGE_VERTEX_BIT},
                {eShaderType::FRAGMENT, VK_SHADER_STAGE_FRAGMENT_BIT},
                {eShaderType::COMPUTE, VK_SHADER_STAGE_COMPUTE_BIT},
                {eShaderType::RAYGEN, VK_SHADER_STAGE_RAYGEN_BIT_KHR},
                {eShaderType::RAYMISS, VK_SHADER_STAGE_MISS_BIT_KHR},
                {eShaderType::RAYCLOSEHIT, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR}
        };

VkPipeline FPipeline::CreateGraphicsPipeline(VkDevice Device, VkRenderPass RenderPass)
{
    std::vector<VkPipelineShaderStageCreateInfo> PipelineShaderStageCreateInfoVector{};

    for (auto& Shader : ShaderModules)
    {
        VkPipelineShaderStageCreateInfo ShaderStageInfo{};
        ShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ShaderStageInfo.module = Shader.ShaderModule;
        ShaderStageInfo.pName = "main";
        ShaderStageInfo.stage = ShaderTypesMap[Shader.ShaderType];

        PipelineShaderStageCreateInfoVector.push_back(ShaderStageInfo);
    }

    VkPipelineVertexInputStateCreateInfo VertexInputInfo{};
    VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VertexInputInfo.vertexBindingDescriptionCount = VertexInputBindingDescriptionVector.size();
    VertexInputInfo.vertexAttributeDescriptionCount = VertexInputAttributeDescriptionVector.size();
    VertexInputInfo.pVertexBindingDescriptions = (VertexInputInfo.vertexBindingDescriptionCount > 0) ? VertexInputBindingDescriptionVector.data() : nullptr;
    VertexInputInfo.pVertexAttributeDescriptions = (VertexInputInfo.vertexAttributeDescriptionCount > 0) ? VertexInputAttributeDescriptionVector.data() : nullptr;

    VkPipelineInputAssemblyStateCreateInfo InputAssembly{};
    InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    InputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport Viewport{};
    Viewport.x = 0.f;
    Viewport.y = 0.f;
    Viewport.width = Width;
    Viewport.height = Height;
    Viewport.minDepth = 0.f;
    Viewport.maxDepth = 1.f;

    VkRect2D Scissors{};
    Scissors.offset = {0, 0};
    Scissors.extent = Extent;

    VkPipelineViewportStateCreateInfo ViewportState{};
    ViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportState.viewportCount = 1;
    ViewportState.pViewports = &Viewport;
    ViewportState.scissorCount = 1;
    ViewportState.pScissors = &Scissors;

    VkPipelineRasterizationStateCreateInfo Rasterizer{};
    Rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    Rasterizer.depthClampEnable = VK_FALSE;
    Rasterizer.rasterizerDiscardEnable = VK_FALSE;
    Rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    Rasterizer.lineWidth = 1.f;
    Rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    Rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    Rasterizer.depthBiasEnable = VK_FALSE;
    Rasterizer.depthBiasConstantFactor = 0.f;
    Rasterizer.depthBiasClamp = 0.f;
    Rasterizer.depthBiasSlopeFactor = 0.f;

    VkPipelineMultisampleStateCreateInfo Multisampling{};
    Multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    Multisampling.sampleShadingEnable = VK_TRUE;
    Multisampling.rasterizationSamples = MSAASamples;
    Multisampling.minSampleShading = 0.2f;
    Multisampling.pSampleMask = nullptr;
    Multisampling.alphaToCoverageEnable = VK_FALSE;
    Multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState ColorBlendAttachment{};
    ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    ColorBlendAttachment.blendEnable = VK_FALSE;
    ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    std::vector<VkPipelineColorBlendAttachmentState> ColorBlendingAttachments;
    for (uint32_t i = 0; i < BlendCount; ++i)
    {
        ColorBlendingAttachments.push_back(ColorBlendAttachment);
    }

    VkPipelineColorBlendStateCreateInfo ColorBlending{};
    ColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColorBlending.logicOpEnable = VK_FALSE;
    ColorBlending.logicOp = VK_LOGIC_OP_COPY;
    ColorBlending.attachmentCount = static_cast<uint32_t>(ColorBlendingAttachments.size());
    ColorBlending.pAttachments = ColorBlendingAttachments.data();
    ColorBlending.blendConstants[0] = 0.f;
    ColorBlending.blendConstants[1] = 0.f;
    ColorBlending.blendConstants[2] = 0.f;
    ColorBlending.blendConstants[3] = 0.f;


    VkPipelineLayoutCreateInfo PipelineLayoutInfo{};
    PipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(DescriptorSetLayouts.size());
    PipelineLayoutInfo.pSetLayouts = DescriptorSetLayouts.data();
    PipelineLayoutInfo.pushConstantRangeCount = 0;
    PipelineLayoutInfo.pPushConstantRanges = nullptr;

    LogicalDevice = Device;

    if (vkCreatePipelineLayout(LogicalDevice, &PipelineLayoutInfo, nullptr, &PipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    VkPipelineDepthStencilStateCreateInfo DepthStencil{};
    DepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    DepthStencil.depthTestEnable = VK_TRUE;
    DepthStencil.depthWriteEnable = VK_TRUE;
    DepthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    DepthStencil.depthBoundsTestEnable = VK_FALSE;
    DepthStencil.minDepthBounds = 0.f;
    DepthStencil.maxDepthBounds = 1.f;
    DepthStencil.stencilTestEnable = VK_FALSE;
    DepthStencil.front = {};
    DepthStencil.back = {};

    RenderPass = RenderPass;

    VkGraphicsPipelineCreateInfo PipelineInfo{};
    PipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineInfo.stageCount = 2;
    PipelineInfo.pStages = PipelineShaderStageCreateInfoVector.data();
    PipelineInfo.pVertexInputState = &VertexInputInfo;
    PipelineInfo.pInputAssemblyState = &InputAssembly;
    PipelineInfo.pViewportState = &ViewportState;
    PipelineInfo.pRasterizationState = &Rasterizer;
    PipelineInfo.pMultisampleState = &Multisampling;
    PipelineInfo.pDepthStencilState = nullptr;
    PipelineInfo.pColorBlendState = &ColorBlending;
    PipelineInfo.pDepthStencilState = &DepthStencil;
    PipelineInfo.layout = PipelineLayout;
    PipelineInfo.renderPass = RenderPass;
    PipelineInfo.subpass = 0;
    PipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    PipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(LogicalDevice, VK_NULL_HANDLE, 1, &PipelineInfo, nullptr, &Pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }

    for (auto& Shader : ShaderModules)
    {
        vkDestroyShaderModule(LogicalDevice, Shader.ShaderModule, nullptr);
    }

    ShaderModules.clear();
    VertexInputBindingDescriptionVector.clear();
    VertexInputAttributeDescriptionVector.clear();
    DescriptorSetLayouts.clear();

    return Pipeline;
}

VkPipeline FPipeline::CreateRayTracingPipeline(VkDevice LogicalDevice)
{
    LogicalDevice = LogicalDevice;

    std::vector<VkPipelineShaderStageCreateInfo> Stages(ShaderModules.size());

    for (auto& ShaderModule : ShaderModules)
    {
        VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo{};
        PipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        PipelineShaderStageCreateInfo.pName = "main";
        PipelineShaderStageCreateInfo.module = ShaderModule.ShaderModule;
        PipelineShaderStageCreateInfo.stage = ShaderTypesMap[ShaderModule.ShaderType];
        Stages.push_back(PipelineShaderStageCreateInfo);
    }

    std::vector<VkRayTracingShaderGroupCreateInfoKHR> RayTracingShaderGroupCreateInfoVector;

    for (uint32_t i = 0; i < Stages.size(); ++i)
    {
        VkRayTracingShaderGroupCreateInfoKHR RayTracingShaderGroupCreateInfo{};
        RayTracingShaderGroupCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        RayTracingShaderGroupCreateInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
        RayTracingShaderGroupCreateInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
        RayTracingShaderGroupCreateInfo.generalShader = VK_SHADER_UNUSED_KHR;
        RayTracingShaderGroupCreateInfo.intersectionShader = VK_SHADER_UNUSED_KHR;

        switch (Stages[i].stage)
        {
            case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
            {
                RayTracingShaderGroupCreateInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
                RayTracingShaderGroupCreateInfo.generalShader = i;
                break;
            }
            case VK_SHADER_STAGE_MISS_BIT_KHR:
            {
                RayTracingShaderGroupCreateInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
                RayTracingShaderGroupCreateInfo.generalShader = i;
                break;
            }
            case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
            {
                RayTracingShaderGroupCreateInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
                RayTracingShaderGroupCreateInfo.closestHitShader = i;
                break;
            }
            default:
            {
                throw std::runtime_error("Failed to create ray tracing pipeline! Incorrect group info.");
            }
        }

        RayTracingShaderGroupCreateInfoVector.push_back(RayTracingShaderGroupCreateInfo);
    }

    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo{};
    PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutCreateInfo.pSetLayouts = DescriptorSetLayouts.data();
    PipelineLayoutCreateInfo.setLayoutCount = DescriptorSetLayouts.size();

    vkCreatePipelineLayout(LogicalDevice, &PipelineLayoutCreateInfo, nullptr, &PipelineLayout);

    VkRayTracingPipelineCreateInfoKHR RayTracingPipelineCreateInfo{};
    RayTracingPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    RayTracingPipelineCreateInfo.stageCount = static_cast<uint32_t>(Stages.size());
    RayTracingPipelineCreateInfo.pStages = Stages.data();
    RayTracingPipelineCreateInfo.groupCount = static_cast<uint32_t>(RayTracingShaderGroupCreateInfoVector.size());
    RayTracingPipelineCreateInfo.pGroups = RayTracingShaderGroupCreateInfoVector.data();
    RayTracingPipelineCreateInfo.maxPipelineRayRecursionDepth = 1;
    RayTracingPipelineCreateInfo.layout = PipelineLayout;

    V::vkCreateRayTracingPipelinesKHR(LogicalDevice, {}, {}, 1, &RayTracingPipelineCreateInfo, nullptr, &Pipeline);

    for (auto& Stage : Stages)
    {
        vkDestroyShaderModule(LogicalDevice, Stage.module, nullptr);
    }

    ShaderModules.clear();
    VertexInputBindingDescriptionVector.clear();
    VertexInputAttributeDescriptionVector.clear();
    DescriptorSetLayouts.clear();

    return Pipeline;
}

void FPipeline::Delete()
{
    vkDestroyPipeline(LogicalDevice, {Pipeline}, nullptr);
    vkDestroyPipelineLayout(LogicalDevice, PipelineLayout, nullptr);

    ShaderModules.clear();
    VertexInputBindingDescriptionVector.clear();
    VertexInputAttributeDescriptionVector.clear();
    DescriptorSetLayouts.clear();
}

FPipeline::~FPipeline()
{

}

VkShaderModule FPipeline::CreateShaderFromFile(const std::string& FileName)
{
    auto ShaderCode = ReadFile(FileName);

    VkShaderModuleCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    CreateInfo.codeSize = ShaderCode.size();
    CreateInfo.pCode = reinterpret_cast<const uint32_t *>(ShaderCode.data());

    VkShaderModule ShaderModule;
    if (vkCreateShaderModule(GetContext().LogicalDevice, &CreateInfo, nullptr, &ShaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module!");
    }

    return ShaderModule;
}