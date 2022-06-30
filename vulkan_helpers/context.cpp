#include "context.h"

#include "systems/camera_system.h"
#include "systems/mesh_system.h"
#include "components/device_camera_component.h"
#include "components/device_transform_component.h"
#include "components/device_renderable_component.h"
#include "components/mesh_component.h"
#include "coordinator.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "vulkan_wrappers.h"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <set>
#include <array>
#include <unordered_map>

static FContext Context{};

namespace LAYOUT_SETS
{
    const std::string PER_FRAME_LAYOUT_NAME = "Per-frame layout";
    const std::string PER_RENDERABLE_LAYOUT_NAME = "Per-renderable layout";
}

namespace LAYOUTS
{
    const std::string TEXTURE_SAMPLER_LAYOUT_NAME = "Texture sampler layout";
    const std::string CAMERA_LAYOUT_NAME = "Camera layout";
    const std::string TRANSFORM_LAYOUT_NAME = "Transform layout";
    const std::string RENDERABLE_LAYOUT_NAME = "Renderable layout";
}

FContext& GetContext()
{
    return Context;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT MessageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallBackData,
        void* pUserData)
{
    std::cerr << "Validation layer: " << pCallBackData->pMessage << std::endl;

    return VK_FALSE;
}

void FContext::Init(GLFWwindow *Window, FController *Controller)
{
    this->Window = Window;
    this->Controller = Controller;
    FunctionLoader = std::make_shared<FVulkanFunctionLoader>();
    ImageManager = std::make_shared<FImageManager>();

    try {
        CreateInstance();
        FunctionLoader->LoadFunctions(Instance);
        SetupDebugMessenger();
        CreateSurface();
        PickPhysicalDevice();
        ImageManager->Init(*this);
        CreateLogicalDevice();
        CommandBufferManager = std::make_shared<FCommandBufferManager>(LogicalDevice, this, GraphicsQueue, GraphicsQueueIndex);
        Swapchain = std::make_shared<FSwapchain>(*this, PhysicalDevice, LogicalDevice, Surface, Window, GraphicsQueueIndex, PresentQueueIndex, VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_PRESENT_MODE_MAILBOX_KHR);
        CreateDepthAndAAImages();
        CreateRenderPass();
        CreateDescriptorSetLayouts();
        CreateGraphicsPipeline();
        ImageManager->LoadImageFromFile(TextureImage, TexturePath);
        CreateFramebuffers();
        CreateTextureSampler();
        LoadModelDataToGPU();
        CreateUniformBuffers();
        CreateDescriptorPool();
        CreateDescriptorSet();
        CreateCommandBuffers();
        CreateSyncObjects();
        CreateImguiContext();
    }
    catch (std::runtime_error &Error) {
        std::cout << Error.what() << std::endl;
        throw;
    }
}

void FContext::CreateInstance()
{
    /// Add instance layers
    InstanceCreationOptions.AddLayer("VK_LAYER_KHRONOS_validation");

    /// Add GLFW instance extensions
    uint32_t Counter = 0;
    auto ExtensionsRequiredByGLFW = glfwGetRequiredInstanceExtensions(&Counter);
    for (uint32_t i = 0; i < Counter; ++i)
    {
        InstanceCreationOptions.AddInstanceExtension(ExtensionsRequiredByGLFW[i]);
    }

    /// Add other extensions
    DebugUtilsMessengerCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    DebugUtilsMessengerCreateInfoEXT.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    DebugUtilsMessengerCreateInfoEXT.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    DebugUtilsMessengerCreateInfoEXT.pfnUserCallback = DebugCallback;
    InstanceCreationOptions.AddInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, &DebugUtilsMessengerCreateInfoEXT, sizeof(VkDebugUtilsMessengerCreateInfoEXT));

    Instance = V::CreateInstance("Hello Triangle", {1, 0, 0}, "No Engine", {1, 0, 0}, VK_API_VERSION_1_0, InstanceCreationOptions);
}

void FContext::SetupDebugMessenger()
{
    if (!DebugMessenger)
    {
        return;
    }

    FunctionLoader->vkCreateDebugUtilsMessengerEXT(Instance, &DebugUtilsMessengerCreateInfoEXT, nullptr, &DebugMessenger);
}

void FContext::CreateSurface()
{
    if (glfwCreateWindowSurface(Instance, Window, nullptr, &Surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface!");
    }
}

void FContext::PickPhysicalDevice()
{
    auto PhysicalDevices = V::GetAllPhysicalDevices(Instance);

    for (const auto& PhysicalDeviceEntry : PhysicalDevices)
    {
        if (V::CheckDeviceExtensionSupport(PhysicalDeviceEntry, DeviceExtensions)
        && V::CheckQueueTypeSupport(PhysicalDeviceEntry, VK_QUEUE_GRAPHICS_BIT, GraphicsQueueIndex)
        && V::CheckPresentQueueSupport(PhysicalDeviceEntry, Surface, PresentQueueIndex))
        {
            PhysicalDevice = PhysicalDeviceEntry;
            QueuePhysicalDeviceProperties();
            break;
        }
    }

    assert(PhysicalDevice&& "Failed to find a suitable GPU!");
}

void FContext::QueuePhysicalDeviceProperties()
{
    MSAASamples = V::GetMaxMsaa(PhysicalDevice);
}

void FContext::CreateLogicalDevice()
{
    /// Fill in the device creation options
    V::FLogicalDeviceOptions Options{};
    for (auto& DeviceExtension : DeviceExtensions)
    {
        Options.AddDeviceExtension(DeviceExtension);
    }

    Options.RequestQueueSupport(GraphicsQueueIndex);
    Options.RequestQueueSupport(PresentQueueIndex);

    LogicalDevice = V::CreateLogicalDevice(PhysicalDevice, Options);

    /// Create queues
    if (GraphicsQueueIndex != UINT32_MAX)
    {
        vkGetDeviceQueue(LogicalDevice, GraphicsQueueIndex, 0, &GraphicsQueue);
    }

    if (PresentQueueIndex != UINT32_MAX)
    {
        vkGetDeviceQueue(LogicalDevice, PresentQueueIndex, 0, &PresentQueue);
    }

    ResourceAllocator = std::make_shared<FResourceAllocator>(PhysicalDevice, LogicalDevice, this);
    DescriptorSetManager = std::make_shared<FDescriptorSetManager>(LogicalDevice);
}

void FContext::CreateDepthAndAAImages()
{
    /// Create Image and ImageView for AA
    VkFormat ColorFormat = Swapchain->GetImageFormat();
    auto Width = Swapchain->GetWidth();
    auto Height = Swapchain->GetHeight();

    ImageManager->CreateImage(ColorImage, Width, Height, false, MSAASamples, ColorFormat, VK_IMAGE_TILING_OPTIMAL,
                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              VK_IMAGE_ASPECT_COLOR_BIT);

    ImageManager->CreateImage(NormalsImage, Width, Height, false, MSAASamples, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                            VK_IMAGE_ASPECT_COLOR_BIT);
    ImageManager->CreateImage(RenderableIndexImage, Width, Height, false, MSAASamples, VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL,
                                                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                    VK_IMAGE_ASPECT_COLOR_BIT);
    ImageManager->CreateImage(UtilityImageR32, Width, Height, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                               VK_IMAGE_ASPECT_COLOR_BIT);
    ImageManager->CreateImage(UtilityImageR8G8B8A8_SRGB, Width, Height, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                                                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                         VK_IMAGE_ASPECT_COLOR_BIT);

    /// Create Image and ImageView for Depth
    VkFormat DepthFormat = FindDepthFormat();
    ImageManager->CreateImage(DepthImage, Width, Height, false, MSAASamples, DepthFormat, VK_IMAGE_TILING_OPTIMAL,
                                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                          VK_IMAGE_ASPECT_DEPTH_BIT);



    auto& DepthImg = (*ImageManager)(DepthImage);
    DepthImg.Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void FContext::CreateRenderPass()
{
    RenderPass = std::make_shared<FRenderPass>();
    RenderPass->AddImageAsAttachment((*ImageManager)(ColorImage), AttachmentType::Color);
    RenderPass->AddImageAsAttachment((*ImageManager)(NormalsImage), AttachmentType::Color);
    RenderPass->AddImageAsAttachment((*ImageManager)(RenderableIndexImage), AttachmentType::Color);
    RenderPass->AddImageAsAttachment((*ImageManager)(DepthImage), AttachmentType::DepthStencil);
    RenderPass->AddImageAsAttachment(Swapchain->Images[0], AttachmentType::Resolve);

    RenderPass->Construct(LogicalDevice);
}

VkFormat FContext::FindSupportedFormat(const std::vector<VkFormat>& Candidates, VkImageTiling Tiling, VkFormatFeatureFlags Features)
{
    for (VkFormat Format : Candidates)
    {
        VkFormatProperties Props;
        vkGetPhysicalDeviceFormatProperties(PhysicalDevice, Format, &Props);

        if (Tiling == VK_IMAGE_TILING_LINEAR && (Props.linearTilingFeatures & Features) == Features)
        {
            return Format;
        }
        else if (Tiling == VK_IMAGE_TILING_OPTIMAL && (Props.optimalTilingFeatures & Features) == Features)
        {
            return Format;
        }
    }

    throw std::runtime_error("Failed to find supported format!");
}

void FContext::CreateDescriptorSetLayouts()
{
    DescriptorSetManager->AddDescriptorLayout(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME, 0, LAYOUTS::CAMERA_LAYOUT_NAME,
                                              {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT});
    DescriptorSetManager->AddDescriptorLayout(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME, 0, LAYOUTS::TEXTURE_SAMPLER_LAYOUT_NAME,
                                              {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT});

    DescriptorSetManager->AddDescriptorLayout(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME, 1, LAYOUTS::TRANSFORM_LAYOUT_NAME,
                                              {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT});
    DescriptorSetManager->AddDescriptorLayout(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME, 1, LAYOUTS::RENDERABLE_LAYOUT_NAME,
                                              {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT});

    DescriptorSetManager->CreateDescriptorSetLayouts();
}

VkShaderModule FContext::CreateShaderFromFile(const std::string& FileName)
{
    auto ShaderCode = ReadFile(FileName);

    VkShaderModuleCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    CreateInfo.codeSize = ShaderCode.size();
    CreateInfo.pCode = reinterpret_cast<const uint32_t *>(ShaderCode.data());

    VkShaderModule ShaderModule;
    if (vkCreateShaderModule(LogicalDevice, &CreateInfo, nullptr, &ShaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module!");
    }

    return ShaderModule;
}

void FContext::CreateGraphicsPipeline()
{
    VkShaderModule VertexShaderModule = CreateShaderFromFile("../shaders/triangle_vert.spv");
    VkShaderModule FragmentShaderModule = CreateShaderFromFile("../shaders/triangle_frag.spv");

    VkPipelineShaderStageCreateInfo VertShaderStageInfo{};
    VertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    VertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    VertShaderStageInfo.module = VertexShaderModule;
    VertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo FragmentShaderStageInfo{};
    FragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    FragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    FragmentShaderStageInfo.module = FragmentShaderModule;
    FragmentShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo ShaderStages[] = {VertShaderStageInfo, FragmentShaderStageInfo};


    VkPipelineVertexInputStateCreateInfo VertexInputInfo{};
    VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto BindingDescription = FVertex::GetBindingDescription();
    auto AttributeDescriptions = FVertex::GetAttributeDescriptions();

    VertexInputInfo.vertexBindingDescriptionCount = 1;
    VertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(AttributeDescriptions.size());
    VertexInputInfo.pVertexBindingDescriptions = &BindingDescription;
    VertexInputInfo.pVertexAttributeDescriptions = AttributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo InputAssembly{};
    InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    InputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport Viewport{};
    Viewport.x = 0.f;
    Viewport.y = 0.f;
    Viewport.width = (float)Swapchain->GetWidth();
    Viewport.height = (float)Swapchain->GetHeight();
    Viewport.minDepth = 0.f;
    Viewport.maxDepth = 1.f;

    VkRect2D Scissors{};
    Scissors.offset = {0, 0};
    Scissors.extent = Swapchain->GetExtent2D();

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

    std::vector<VkPipelineColorBlendAttachmentState> ColorBlendingAttachments{ColorBlendAttachment, ColorBlendAttachment, ColorBlendAttachment};

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
    std::vector<VkDescriptorSetLayout> PipelineSetLayouts = {DescriptorSetManager->GetVkDescriptorSetLayout(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME),
                                                             DescriptorSetManager->GetVkDescriptorSetLayout(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME)};
    PipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(PipelineSetLayouts.size());
    PipelineLayoutInfo.pSetLayouts = PipelineSetLayouts.data();
    PipelineLayoutInfo.pushConstantRangeCount = 0;
    PipelineLayoutInfo.pPushConstantRanges = nullptr;

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

    VkGraphicsPipelineCreateInfo PipelineInfo{};
    PipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineInfo.stageCount = 2;
    PipelineInfo.pStages = ShaderStages;
    PipelineInfo.pVertexInputState = &VertexInputInfo;
    PipelineInfo.pInputAssemblyState = &InputAssembly;
    PipelineInfo.pViewportState = &ViewportState;
    PipelineInfo.pRasterizationState = &Rasterizer;
    PipelineInfo.pMultisampleState = &Multisampling;
    PipelineInfo.pDepthStencilState = nullptr;
    PipelineInfo.pColorBlendState = &ColorBlending;
    PipelineInfo.pDepthStencilState = &DepthStencil;
    PipelineInfo.layout = PipelineLayout;
    PipelineInfo.renderPass = RenderPass->RenderPass;
    PipelineInfo.subpass = 0;
    PipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    PipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(LogicalDevice, VK_NULL_HANDLE, 1, &PipelineInfo, nullptr, &GraphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(LogicalDevice, FragmentShaderModule, nullptr);
    vkDestroyShaderModule(LogicalDevice, VertexShaderModule, nullptr);
}

VkFormat FContext::FindDepthFormat()
{
    return FindSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool FContext::HasStensilComponent(VkFormat Format)
{
    return Format == VK_FORMAT_D32_SFLOAT_S8_UINT || Format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void FContext::CreateFramebuffers()
{
    SwapChainFramebuffers.resize(Swapchain->Size());
    for (std::size_t i = 0; i < Swapchain->Size(); ++i) {
        std::vector<VkImageView> Attachments = {(*ImageManager)(ColorImage).View, (*ImageManager)(NormalsImage).View, (*ImageManager)(RenderableIndexImage).View,
                                                (*ImageManager)(DepthImage).View,
                                                Swapchain->GetImages()[i].View};

        VkFramebufferCreateInfo FramebufferInfo{};
        FramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        FramebufferInfo.renderPass = RenderPass->RenderPass;
        FramebufferInfo.attachmentCount = static_cast<uint32_t>(Attachments.size());
        FramebufferInfo.pAttachments = Attachments.data();
        FramebufferInfo.width = Swapchain->GetWidth();
        FramebufferInfo.height = Swapchain->GetHeight();
        FramebufferInfo.layers = 1;

        if (vkCreateFramebuffer(LogicalDevice, &FramebufferInfo, nullptr, &SwapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }

}

void FContext::LoadModelDataToGPU()
{
    auto MeshSystem = ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMeshSystem>();

    for(auto Mesh : *MeshSystem)
    {
        MeshSystem->LoadToGPU(Mesh);
    }

}

void FContext::CreateTextureSampler()
{
    VkSamplerCreateInfo SamplerInfo{};
    SamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    SamplerInfo.magFilter = VK_FILTER_LINEAR;
    SamplerInfo.minFilter = VK_FILTER_LINEAR;
    SamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    SamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    SamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    SamplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties Properties{};
    vkGetPhysicalDeviceProperties(PhysicalDevice, &Properties);
    SamplerInfo.maxAnisotropy = Properties.limits.maxSamplerAnisotropy;
    SamplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    SamplerInfo.unnormalizedCoordinates = VK_FALSE;
    SamplerInfo.compareEnable = VK_FALSE;
    SamplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    SamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    SamplerInfo.mipLodBias = 0.f;
    SamplerInfo.minLod = 0.f;
    SamplerInfo.maxLod = static_cast<float>(MipLevels);

    if (vkCreateSampler(LogicalDevice, &SamplerInfo, nullptr, &TextureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create texture sampler!");
    }
}

void FContext::CreateUniformBuffers()
{
    auto& Coordinator = ECS::GetCoordinator();
    VkDeviceSize TransformBufferSize = Coordinator.Size<ECS::COMPONENTS::FDeviceTransformComponent>();
    VkDeviceSize CameraBufferSize = Coordinator.Size<ECS::COMPONENTS::FDeviceCameraComponent>();
    VkDeviceSize RenderableBufferSize = Coordinator.Size<ECS::COMPONENTS::FDeviceRenderableComponent>();

    DeviceTransformBuffers.resize(Swapchain->Size());
    DeviceCameraBuffers.resize(Swapchain->Size());
    DeviceRenderableBuffers.resize(Swapchain->Size());

    for (size_t i = 0; i < Swapchain->Size(); ++i)
    {
        DeviceTransformBuffers[i] = ResourceAllocator->CreateBuffer(TransformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        DeviceCameraBuffers[i] = ResourceAllocator->CreateBuffer(CameraBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        DeviceRenderableBuffers[i] = ResourceAllocator->CreateBuffer(RenderableBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }
}

void FContext::CreateDescriptorPool()
{
    auto ModelsCount = ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMeshSystem>()->Size();
    uint32_t NumberOfSwapChainImages = static_cast<uint32_t>(Swapchain->Size());

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->AddDescriptorSet(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME, NumberOfSwapChainImages);
    DescriptorSetManager->AddDescriptorSet(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME, NumberOfSwapChainImages * ModelsCount);

    DescriptorSetManager->ReserveDescriptorPool();
}

void FContext::CreateDescriptorSet()
{
    auto& Coordinator = ECS::GetCoordinator();
    auto MeshSystem = Coordinator.GetSystem<ECS::SYSTEMS::FMeshSystem>();

    /// Create descriptor sets
    DescriptorSetManager->CreateAllDescriptorSets();

    uint32_t NumberOfSwapChainImages = static_cast<uint32_t>(Swapchain->Size());

    for (uint32_t i = 0; i < Swapchain->Size(); ++i)
    {
        uint32_t j = 0;
        for (auto Mesh : *MeshSystem)
        {
            VkDescriptorBufferInfo TransformBufferInfo{};
            TransformBufferInfo.buffer = DeviceTransformBuffers[i].Buffer;
            TransformBufferInfo.offset = static_cast<uint32_t>(sizeof(ECS::COMPONENTS::FDeviceTransformComponent) * j);
            TransformBufferInfo.range = sizeof(ECS::COMPONENTS::FDeviceTransformComponent);
            DescriptorSetManager->UpdateDescriptorSetInfo(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME, LAYOUTS::TRANSFORM_LAYOUT_NAME, j * NumberOfSwapChainImages + i, TransformBufferInfo);

            VkDescriptorBufferInfo RenderableBufferInfo{};
            RenderableBufferInfo.buffer = DeviceRenderableBuffers[i].Buffer;
            RenderableBufferInfo.offset = static_cast<uint32_t>(sizeof(ECS::COMPONENTS::FDeviceRenderableComponent) * j);
            RenderableBufferInfo.range = sizeof(ECS::COMPONENTS::FDeviceRenderableComponent);
            DescriptorSetManager->UpdateDescriptorSetInfo(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME, LAYOUTS::RENDERABLE_LAYOUT_NAME, j * NumberOfSwapChainImages + i, RenderableBufferInfo);

            ++j;
        }
    }

    for (uint32_t i = 0; i < Swapchain->Size(); ++i)
    {
        VkDescriptorBufferInfo CameraBufferInfo{};
        CameraBufferInfo.buffer = DeviceCameraBuffers[i].Buffer;
        CameraBufferInfo.offset = 0;
        CameraBufferInfo.range = sizeof(ECS::COMPONENTS::FDeviceCameraComponent);
        DescriptorSetManager->UpdateDescriptorSetInfo(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME, LAYOUTS::CAMERA_LAYOUT_NAME, i, CameraBufferInfo);

        VkDescriptorImageInfo ImageBufferInfo{};
        ImageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ImageBufferInfo.imageView = (*ImageManager)(TextureImage).View;
        ImageBufferInfo.sampler = TextureSampler;
        DescriptorSetManager->UpdateDescriptorSetInfo(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME, LAYOUTS::TEXTURE_SAMPLER_LAYOUT_NAME, i, ImageBufferInfo);
    }
}

void FContext::CreateCommandBuffers()
{
    CommandBuffers.resize(SwapChainFramebuffers.size());

    for (std::uint32_t i = 0; i <CommandBuffers.size(); ++i)
    {
        CommandBuffers[i] = CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            VkRenderPassBeginInfo RenderPassInfo{};
            RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            RenderPassInfo.renderPass = RenderPass->RenderPass;
            RenderPassInfo.framebuffer = SwapChainFramebuffers[i];
            RenderPassInfo.renderArea.offset = {0, 0};
            RenderPassInfo.renderArea.extent = Swapchain->GetExtent2D();

            std::vector<VkClearValue> ClearValues{7};
            ClearValues[0].color = {0.f, 0.f, 0.f, 1.f};
            ClearValues[1].color = {0.0f, 0.f, 0.f, 1.f};
            ClearValues[2].color = {0, 0, 0, 0};
            ClearValues[3].depthStencil = {1.f, 0};
            ClearValues[4].color = {0.f, 0.f, 0.f, 0.f};
            RenderPassInfo.clearValueCount = static_cast<uint32_t>(ClearValues.size());
            RenderPassInfo.pClearValues = ClearValues.data();

            vkCmdBeginRenderPass(CommandBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline);

            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout, 0, 1, &DescriptorSetManager->GetSet(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME, i), 0,
                                    nullptr);
            auto& Coordinator = ECS::GetCoordinator();
            auto MeshSystem = Coordinator.GetSystem<ECS::SYSTEMS::FMeshSystem>();

            uint32_t j = 0;
            for (auto Entity : *MeshSystem)
            {
                vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout, 1, 1,
                                        &DescriptorSetManager->GetSet(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME, j * static_cast<uint32_t>(Swapchain->Size()) + i), 0,
                                        nullptr);

                MeshSystem->Bind(Entity, CommandBuffer);
                MeshSystem->Draw(Entity, CommandBuffer);
                ++j;
            }
            vkCmdEndRenderPass(CommandBuffer);
        });
    }
}

void FContext::CreateSyncObjects()
{
    ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    RenderingFinishedFences.resize(MAX_FRAMES_IN_FLIGHT);
    ImagesInFlight.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
    ImGuiFinishedFences.resize(MAX_FRAMES_IN_FLIGHT);
    ImGuiFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo SemaphoreInfo{};
    SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo FenceInfo{};
    FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (vkCreateSemaphore(LogicalDevice, &SemaphoreInfo, nullptr, &ImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(LogicalDevice, &SemaphoreInfo, nullptr, &RenderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(LogicalDevice, &SemaphoreInfo, nullptr, &ImGuiFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(LogicalDevice, &FenceInfo, nullptr, &ImGuiFinishedFences[i]) != VK_SUCCESS ||
            vkCreateFence(LogicalDevice, &FenceInfo, nullptr, &RenderingFinishedFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create synchronization objects for a frame!");
        }
    }
}

void FContext::CreateImguiContext()
{
    {
        VkDescriptorPoolSize PoolSizes[] =
                {
                        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
                };
        VkDescriptorPoolCreateInfo PoolInfo = {};
        PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        PoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        PoolInfo.maxSets = 1000 * IM_ARRAYSIZE(PoolSizes);
        PoolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(PoolSizes);
        PoolInfo.pPoolSizes = PoolSizes;
        if(vkCreateDescriptorPool(LogicalDevice, &PoolInfo, nullptr, &ImGuiDescriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create descriptor pool for ImGui!");
        }
    }

    {
        VkAttachmentDescription AttachmentDescription{};
        AttachmentDescription.format = Swapchain->GetImageFormat();
        AttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        AttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        AttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        AttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        AttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        AttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        AttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference AttachmentReference{};
        AttachmentReference.attachment = 0;
        AttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription SubpassDescription{};
        SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        SubpassDescription.colorAttachmentCount = 1;
        SubpassDescription.pColorAttachments = &AttachmentReference;

        VkSubpassDependency SubpassDependency{};
        SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        SubpassDependency.dstSubpass = 0;
        SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        SubpassDependency.srcAccessMask = 0;
        SubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo RenderPassCreateInfo{};
        RenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        RenderPassCreateInfo.attachmentCount = 1;
        RenderPassCreateInfo.pAttachments = &AttachmentDescription;
        RenderPassCreateInfo.subpassCount = 1;
        RenderPassCreateInfo.pSubpasses = &SubpassDescription;
        RenderPassCreateInfo.dependencyCount = 1;
        RenderPassCreateInfo.pDependencies = &SubpassDependency;

        if (vkCreateRenderPass(LogicalDevice, &RenderPassCreateInfo, nullptr, &ImGuiRenderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create renderpass for ImGui!");
        }
    }

    {
        ImGuiFramebuffers.resize(Swapchain->Size());

        for(uint32_t i = 0; i < Swapchain->Size(); ++i)
        {
            VkImageView Attachment[1];
            Attachment[0] = Swapchain->Images[i].View;

            VkFramebufferCreateInfo FramebufferCreateInfo{};
            FramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            FramebufferCreateInfo.renderPass = ImGuiRenderPass;
            FramebufferCreateInfo.attachmentCount = 1;
            FramebufferCreateInfo.pAttachments = Attachment;
            FramebufferCreateInfo.width = Swapchain->GetWidth();
            FramebufferCreateInfo.height = Swapchain->GetHeight();
            FramebufferCreateInfo.layers = 1;

            if (vkCreateFramebuffer(LogicalDevice, &FramebufferCreateInfo, nullptr, &ImGuiFramebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create framebuffers for ImGui!");
            }

        }
    }

    auto CheckResultFunction = [](VkResult Err)
            {
                if (Err == 0)
                    return;
                std::cout << "[vulkan] Error: VkResult = " << Err << std::endl;
                if (Err < 0)
                    abort();
            };

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& IO = ImGui::GetIO();

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(Window, true);
    ImGui_ImplVulkan_InitInfo InitInfo{};
    InitInfo.Instance = Instance;
    InitInfo.PhysicalDevice = PhysicalDevice;
    InitInfo.Device = LogicalDevice;
    InitInfo.QueueFamily = GraphicsQueueIndex;
    InitInfo.Queue = GraphicsQueue;
    InitInfo.DescriptorPool = ImGuiDescriptorPool;
    InitInfo.MinImageCount = MAX_FRAMES_IN_FLIGHT;
    InitInfo.ImageCount = MAX_FRAMES_IN_FLIGHT;
    InitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    InitInfo.CheckVkResultFn = CheckResultFunction;
    ImGui_ImplVulkan_Init(&InitInfo, ImGuiRenderPass);

    {
        CommandBufferManager->RunSingletimeCommand(ImGui_ImplVulkan_CreateFontsTexture);
    }
}

void FContext::Render()
{
    /// Previous rendering iteration of the frame might still be in use, so we wait for it
    vkWaitForFences(LogicalDevice, 1, &ImGuiFinishedFences[CurrentFrame], VK_TRUE, UINT64_MAX);

    /// Acquire next image from swapchain, also it's index and provide semaphore to signal when image is ready to be used
    VkResult Result = Swapchain->GetNextImage(CurrentImage, ImageAvailableSemaphores[CurrentFrame], ImageIndex);

    /// Run some checks
    if (Result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain();
        return;
    }
    if (Result != VK_SUCCESS && Result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    /// If this image is still in use, wait for it.
    if(ImagesInFlight[ImageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(LogicalDevice, 1, &ImagesInFlight[ImageIndex], VK_TRUE, UINT64_MAX);
    }

    /// Mark this image as is being in use
    ImagesInFlight[ImageIndex] = ImGuiFinishedFences[CurrentFrame];

    UpdateUniformBuffer(ImageIndex);

    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore WaitSemaphores[] = {ImageAvailableSemaphores[CurrentFrame]};
    VkPipelineStageFlags WaitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    SubmitInfo.waitSemaphoreCount = 1;
    SubmitInfo.pWaitSemaphores = WaitSemaphores;
    SubmitInfo.pWaitDstStageMask = WaitStages;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CommandBuffers[ImageIndex];

    VkSemaphore SignalSemaphores[] = {RenderFinishedSemaphores[CurrentFrame]};
    SubmitInfo.signalSemaphoreCount = 1;
    SubmitInfo.pSignalSemaphores = SignalSemaphores;

    /// Reset frame state to unsignaled, just before rendering
    vkResetFences(LogicalDevice, 1, &RenderingFinishedFences[CurrentFrame]);

    /// Submit rendering. When rendering finished, appropriate fence will be signalled
    if (vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, RenderingFinishedFences[CurrentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }
}

void FContext::RenderImGui()
{
    vkWaitForFences(LogicalDevice, 1, &RenderingFinishedFences[CurrentFrame], VK_TRUE, UINT64_MAX);

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar);
    ImGui::SetWindowPos({0.f, 0.f});
    ImGui::TextColored({0.f, 1.f, 0.f, 1.f}, "Test text");
    ImGui::End();

    auto CommandBuffer = CommandBufferManager->BeginSingleTimeCommand();

    CommandBufferManager->RecordCommand([&, this](VkCommandBuffer)
    {
        {
            VkRenderPassBeginInfo RenderPassBeginInfo{};
            RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            RenderPassBeginInfo.renderPass = ImGuiRenderPass;
            RenderPassBeginInfo.framebuffer = ImGuiFramebuffers[CurrentFrame % Swapchain->Size()];
            RenderPassBeginInfo.renderArea.extent = Swapchain->GetExtent2D();
            RenderPassBeginInfo.clearValueCount = 0;
            vkCmdBeginRenderPass(CommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        }

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), CommandBuffer);

        vkCmdEndRenderPass(CommandBuffer);
        vkEndCommandBuffer(CommandBuffer);

        VkSubmitInfo SubmitInfo{};
        SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore WaitSemaphores[] = {RenderFinishedSemaphores[CurrentFrame]};
        VkPipelineStageFlags WaitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        SubmitInfo.waitSemaphoreCount = 1;
        SubmitInfo.pWaitSemaphores = WaitSemaphores;
        SubmitInfo.pWaitDstStageMask = WaitStages;
        SubmitInfo.commandBufferCount = 1;
        SubmitInfo.pCommandBuffers = &CommandBuffer;

        VkSemaphore SignalSemaphores[] = {ImGuiFinishedSemaphores[CurrentFrame]};
        SubmitInfo.signalSemaphoreCount = 1;
        SubmitInfo.pSignalSemaphores = SignalSemaphores;

        vkResetFences(LogicalDevice, 1, &ImGuiFinishedFences[CurrentFrame]);

        if (vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, ImGuiFinishedFences[CurrentFrame]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to submit ImGui draw command buffer!");
        }
    });
}

void FContext::Present()
{
    VkSemaphore SignalSemaphores[] = {ImGuiFinishedSemaphores[CurrentFrame]};

    VkPresentInfoKHR PresentInfo{};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pWaitSemaphores = SignalSemaphores;
    VkSwapchainKHR SwapChains[] = {Swapchain->GetSwapchain()};
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = SwapChains;
    PresentInfo.pImageIndices = &ImageIndex;
    PresentInfo.pResults = nullptr;

    VkResult Result = vkQueuePresentKHR(PresentQueue, &PresentInfo);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR || bFramebufferResized)
    {
        bFramebufferResized = false;
        RecreateSwapChain();
    }
    else if (Result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    CurrentFrame = (CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void FContext::WaitIdle()
{
    vkDeviceWaitIdle(LogicalDevice);
}

void FContext::RecreateSwapChain()
{
    int Width = 0;
    int Height = 0;
    glfwGetFramebufferSize(Window, &Width, &Height);
    while (Width == 0 || Height == 0)
    {
        glfwGetFramebufferSize(Window, &Width, &Height);
        glfwPollEvents();
    }

    vkDeviceWaitIdle(LogicalDevice);

    CleanUpSwapChain();

    Swapchain = std::make_shared<FSwapchain>(*this, PhysicalDevice, LogicalDevice, Surface, Window, GraphicsQueueIndex, PresentQueueIndex, VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_PRESENT_MODE_MAILBOX_KHR);
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSet();
    CreateCommandBuffers();
}

void FContext::CleanUpSwapChain()
{
    ImageManager->RemoveImage(ColorImage);
    ImageManager->RemoveImage(UtilityImageR8G8B8A8_SRGB);
    ImageManager->RemoveImage(NormalsImage);
    ImageManager->RemoveImage(RenderableIndexImage);
    ImageManager->RemoveImage(UtilityImageR32);
    ImageManager->RemoveImage(DepthImage);

    for (auto Framebuffer : SwapChainFramebuffers)
    {
        vkDestroyFramebuffer(LogicalDevice, Framebuffer, nullptr);
    }

    for (auto Framebuffer : ImGuiFramebuffers)
    {
        vkDestroyFramebuffer(LogicalDevice, Framebuffer, nullptr);
    }

    for (auto& CommandBuffer : CommandBuffers)
    {
        CommandBufferManager->FreeCommandBuffer(CommandBuffer);
    }

    vkDestroyPipeline(LogicalDevice, GraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(LogicalDevice, PipelineLayout, nullptr);
    RenderPass = nullptr;
    vkDestroyRenderPass(LogicalDevice, ImGuiRenderPass, nullptr);

    for (size_t i = 0; i < Swapchain->Size(); ++i)
    {
        ResourceAllocator->DestroyBuffer(DeviceTransformBuffers[i]);
        ResourceAllocator->DestroyBuffer(DeviceCameraBuffers[i]);
        ResourceAllocator->DestroyBuffer(DeviceRenderableBuffers[i]);
    }

    Swapchain = nullptr;

    DescriptorSetManager->FreeDescriptorPool();
    vkDestroyDescriptorPool(LogicalDevice, ImGuiDescriptorPool, nullptr);
}

void FContext::UpdateUniformBuffer(uint32_t CurrentImage)
{
    auto& Coordinator = ECS::GetCoordinator();
    auto CameraSystem = Coordinator.GetSystem<ECS::SYSTEMS::FCameraSystem>();

    auto DeviceTransformComponentsData = Coordinator.Data<ECS::COMPONENTS::FDeviceTransformComponent>();
    auto DeviceTransformComponentsSize = Coordinator.Size<ECS::COMPONENTS::FDeviceTransformComponent>();

    auto DeviceCameraComponentsData = Coordinator.Data<ECS::COMPONENTS::FDeviceCameraComponent>();
    auto DeviceCameraComponentsSize = Coordinator.Size<ECS::COMPONENTS::FDeviceCameraComponent>();

    auto RenderableComponentData = Coordinator.Data<ECS::COMPONENTS::FDeviceRenderableComponent>();
    auto RenderableComponentSize = Coordinator.Size<ECS::COMPONENTS::FDeviceRenderableComponent>();

    LoadDataIntoBuffer(DeviceTransformBuffers[CurrentImage], DeviceTransformComponentsData, DeviceTransformComponentsSize);
    LoadDataIntoBuffer(DeviceCameraBuffers[CurrentImage], DeviceCameraComponentsData, DeviceCameraComponentsSize);
    LoadDataIntoBuffer(DeviceRenderableBuffers[CurrentImage], RenderableComponentData, RenderableComponentSize);
}

void FContext::LoadDataIntoBuffer(FBuffer &Buffer, void* DataToLoad, size_t Size)
{
    if (Size > Buffer.Size)
    {
        throw std::runtime_error("Loading data into buffer with data size greater that buffer size!");
    }
    void* Data;
    vkMapMemory(LogicalDevice, Buffer.Memory, 0, Size, 0, &Data);
    memcpy(Data, DataToLoad, Size);
    vkUnmapMemory(LogicalDevice, Buffer.Memory);

}

void FContext::FreeData(FBuffer Buffer)
{
    ResourceAllocator->DestroyBuffer(Buffer);
}

void FContext::DestroyDebugUtilsMessengerEXT()
{
    FunctionLoader->vkDestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
}

void FContext::CleanUp()
{
    CleanUpSwapChain();

    vkDestroySampler(LogicalDevice, TextureSampler, nullptr);
    ImageManager->RemoveImage(TextureImage);

    DescriptorSetManager->DestroyDescriptorSetLayout(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME);
    DescriptorSetManager->DestroyDescriptorSetLayout(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    auto& Coordinator = ECS::GetCoordinator();
    Coordinator.GetSystem<ECS::SYSTEMS::FMeshSystem>()->FreeAllDeviceData();

    for(std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(LogicalDevice, RenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(LogicalDevice, ImageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(LogicalDevice, ImGuiFinishedSemaphores[i], nullptr);
        vkDestroyFence(LogicalDevice, RenderingFinishedFences[i], nullptr);
        vkDestroyFence(LogicalDevice, ImGuiFinishedFences[i], nullptr);
    }

    CommandBufferManager = nullptr;
    vkDestroyDevice(LogicalDevice, nullptr);

    if (DebugMessenger)
    {
        DestroyDebugUtilsMessengerEXT();
    }

    vkDestroySurfaceKHR(Instance, Surface, nullptr);
    vkDestroyInstance(Instance, nullptr);
}

FContext::~FContext()
{
}

std::vector<char> ReadFile(const std::string& FileName)
{
    std::ifstream File(FileName, std::ios::ate | std::ios::binary);

    if (!File.is_open())
    {
        throw std::runtime_error("Failed to open file!");
    }

    std::size_t FileSize = (std::size_t)File.tellg();
    std::vector<char> Buffer(FileSize);

    File.seekg(0);
    File.read(Buffer.data(), FileSize);
    File.close();

    return Buffer;
}
