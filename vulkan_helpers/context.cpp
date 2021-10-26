#include "context.h"
#include "descriptors.h"

#include "systems/camera_system.h"
#include "systems/mesh_system.h"
#include "components/device_camera_component.h"
#include "components/device_transform_component.h"
#include "components/renderable_component.h"
#include "components/mesh_component.h"
#include "coordinator.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <set>
#include <array>
#include <unordered_map>
#include <chrono>

static FContext Context{};

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

    try {
        CreateInstance();
        FunctionLoader->LoadFunctions(Instance);
        SetupDebugMessenger();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDevice();
        CreateCommandPool();
        Swapchain = std::make_shared<FSwapchain>(*this, PhysicalDevice, LogicalDevice, Surface, Window, GraphicsQueueIndex, PresentQueueIndex, VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_PRESENT_MODE_MAILBOX_KHR);
        CreateDepthAndAAImages();
        CreateRenderPass();
        CreateDescriptorSetLayouts();
        CreateGraphicsPipeline();
        CreateFramebuffers();
        CreateTextureImage(TexturePath);
        CreateTextureImageView();
        CreateTextureSampler();
        LoadModelDataToGPU();
        CreateUniformBuffers();
        CreateDescriptorPool();
        CreateDescriptorSet();
        CreateCommandBuffers();
        CreateSyncObjects();
    }
    catch (std::runtime_error &Error) {
        std::cout << Error.what() << std::endl;
        throw;
    }
}

void FContext::CreateInstance()
{
    /// Check supported Layers
    {
        uint32_t LayerCount;
        vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);

        std::vector<VkLayerProperties> AvailableLayers(LayerCount);
        vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

        for (const auto &LayerName : ValidationLayers) {
            bool LayerFound = false;

            for (const auto &LayerProperties : AvailableLayers) {
                if (LayerName == LayerProperties.layerName) {
                    LayerFound = true;
                    break;
                }
            }
            if (!LayerFound) {
                throw std::runtime_error("Validation layers requested, but not available!");
            }
        }
    }

    // Fill in instance creation data
    VkApplicationInfo AppInfo{};
    AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    AppInfo.pApplicationName = "Hello Triangle";
    AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    AppInfo.pEngineName = "No Engine";
    AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    AppInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    CreateInfo.pApplicationInfo = &AppInfo;

    // Resolve and add extensions and layers
    uint32_t Counter = 0;
    auto ExtensionsRequiredByGLFW = glfwGetRequiredInstanceExtensions(&Counter);
    for (uint32_t i = 0; i < Counter; ++i)
    {
        InstanceExtensions.push_back(ExtensionsRequiredByGLFW[i]);
    }

    if (!ValidationLayers.empty())
    {
        InstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // Generate char* names for layers and extensions
    std::vector<const char*> CharExtensions;
    for (const auto& Extension : InstanceExtensions)
    {
        CharExtensions.push_back(Extension.c_str());
    }
    std::vector<const char*> CharLayers;
    for (const auto& Layer : ValidationLayers)
    {
        CharLayers.push_back(Layer.c_str());
    }

    CreateInfo.enabledExtensionCount = static_cast<uint32_t>(InstanceExtensions.size());
    CreateInfo.ppEnabledExtensionNames = CharExtensions.data();

    if (!ValidationLayers.empty())
    {
        CreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
        CreateInfo.ppEnabledLayerNames = CharLayers.data();

        DebugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        DebugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        DebugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        DebugCreateInfo.pfnUserCallback = DebugCallback;

        CreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &DebugCreateInfo;
    }
    else
    {
        CreateInfo.enabledLayerCount = 0;

        CreateInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&CreateInfo, nullptr, &Instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create instance!");
    }
}

void FContext::SetupDebugMessenger()
{
    if (ValidationLayers.empty())
    {
        return;
    }

    FunctionLoader->vkCreateDebugUtilsMessengerEXT(Instance, &DebugCreateInfo, nullptr, &DebugMessenger);
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
    uint32_t DeviceCount = 0;
    vkEnumeratePhysicalDevices(Instance, &DeviceCount, nullptr);

    if (DeviceCount == 0)
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> Devices(DeviceCount);
    vkEnumeratePhysicalDevices(Instance, &DeviceCount, Devices.data());

    for (const auto& Device : Devices)
    {
        if (CheckDeviceExtensionsSupport(Device) && CheckDeviceQueueSupport(Device))
        {
            PhysicalDevice = Device;
            QueuePhysicalDeviceProperties();
            break;
        }
    }

    if (PhysicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

void FContext::QueuePhysicalDeviceProperties()
{
    VkPhysicalDeviceProperties PhysicalDeviceProperties;

    vkGetPhysicalDeviceProperties(PhysicalDevice, &PhysicalDeviceProperties);

    VkSampleCountFlags Count = PhysicalDeviceProperties.limits.framebufferColorSampleCounts & PhysicalDeviceProperties.limits.framebufferDepthSampleCounts;

    if (Count & VK_SAMPLE_COUNT_64_BIT)
    {
        MSAASamples = VK_SAMPLE_COUNT_64_BIT;
    }
    else if (Count & VK_SAMPLE_COUNT_32_BIT)
    {
        MSAASamples = VK_SAMPLE_COUNT_32_BIT;
    }
    else if (Count & VK_SAMPLE_COUNT_16_BIT)
    {
        MSAASamples = VK_SAMPLE_COUNT_16_BIT;
    }
    else if (Count & VK_SAMPLE_COUNT_8_BIT)
    {
        MSAASamples = VK_SAMPLE_COUNT_8_BIT;
    }
    else if (Count & VK_SAMPLE_COUNT_4_BIT)
    {
        MSAASamples = VK_SAMPLE_COUNT_4_BIT;
    }
    else if (Count & VK_SAMPLE_COUNT_2_BIT)
    {
        MSAASamples = VK_SAMPLE_COUNT_2_BIT;
    }
    else
    {
        MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    }
}

bool FContext::CheckDeviceExtensionsSupport(VkPhysicalDevice Device)
{
    uint32_t ExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(Device, nullptr, &ExtensionCount, nullptr);

    std::vector<VkExtensionProperties>AvailableExtensions(ExtensionCount);
    vkEnumerateDeviceExtensionProperties(Device, nullptr, &ExtensionCount, AvailableExtensions.data());

    std::set<std::string> RequiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

    for (const auto& Extension : AvailableExtensions)
    {
        RequiredExtensions.erase(Extension.extensionName);
    }

    return RequiredExtensions.empty();
}

bool FContext::CheckDeviceQueueSupport(VkPhysicalDevice Device)
{
    uint32_t QueueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> QueueFamilyProperties(QueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, QueueFamilyProperties.data());

    for (uint32_t i = 0; i < QueueFamilyCount; ++i) {

        if (GraphicsQueueIndex == UINT32_MAX) {
            if (bGraphicsCapabilityRequired) {
                if ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT) {
                    GraphicsQueueIndex = i;
                }
            }
        }

        if (ComputeQueueIndex == UINT32_MAX) {
            if (bComputeCapabilityRequired) {
                if ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT) {
                    GraphicsQueueIndex = i;
                }
            }
        }

        if (TransferQueueIndex == UINT32_MAX) {
            if (bTransferCapabilityRequired) {
                if ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT) {
                    TransferQueueIndex = i;
                }
            }
        }

        if (SparseBindingQueueIndex == UINT32_MAX) {
            if (bSparseBindingCapabilityRequired) {
                if ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) ==
                    VK_QUEUE_SPARSE_BINDING_BIT) {
                    SparseBindingQueueIndex = i;
                }
            }
        }

        if (PresentQueueIndex == UINT32_MAX) {
            if (bPresentCapabilityRequired) {
                VkBool32 PresentSupport = false;

                vkGetPhysicalDeviceSurfaceSupportKHR(Device, i, Surface, &PresentSupport);

                if (PresentSupport) {
                    PresentQueueIndex = i;
                }
            }
        }
    }

    if ((bGraphicsCapabilityRequired && (GraphicsQueueIndex == UINT32_MAX)) ||
        (bComputeCapabilityRequired && (ComputeQueueIndex == UINT32_MAX)) ||
        (bTransferCapabilityRequired && (TransferQueueIndex == UINT32_MAX)) ||
        (bSparseBindingCapabilityRequired && (SparseBindingQueueIndex == UINT32_MAX)) ||
        (bPresentCapabilityRequired && (PresentQueueIndex == UINT32_MAX)))
    {
        return false;
    }

    return true;
}

void FContext::CreateLogicalDevice()
{
    std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
    std::set<uint32_t> UniqueQueueFamilies{};

    if (GraphicsQueueIndex != UINT32_MAX)
    {
        UniqueQueueFamilies.insert(GraphicsQueueIndex);
    }

    if (ComputeQueueIndex != UINT32_MAX)
    {
        UniqueQueueFamilies.insert(ComputeQueueIndex);
    }

    if (TransferQueueIndex != UINT32_MAX)
    {
        UniqueQueueFamilies.insert(TransferQueueIndex);
    }

    if (SparseBindingQueueIndex != UINT32_MAX)
    {
        UniqueQueueFamilies.insert(SparseBindingQueueIndex);
    }

    if (PresentQueueIndex != UINT32_MAX)
    {
        UniqueQueueFamilies.insert(PresentQueueIndex);
    }

    float QueuePriority = 1.f;
    for (auto QueueFamilyIndex : UniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo QueueCreateInfo{};
        QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        QueueCreateInfo.queueFamilyIndex = QueueFamilyIndex;
        QueueCreateInfo.queueCount = 1;
        QueueCreateInfo.pQueuePriorities = &QueuePriority;
        QueueCreateInfos.push_back(QueueCreateInfo);
    }

    VkPhysicalDeviceFeatures DeviceFeatures{};
    DeviceFeatures.samplerAnisotropy = VK_TRUE;
    DeviceFeatures.sampleRateShading = VK_TRUE;

    VkDeviceCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    CreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
    CreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());
    CreateInfo.pEnabledFeatures = &DeviceFeatures;
    CreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
    std::vector<const char*> CharExtensions;
    for (const auto& Extension : DeviceExtensions)
    {
        CharExtensions.push_back(Extension.c_str());
    }
    CreateInfo.ppEnabledExtensionNames = CharExtensions.data();

    std::vector<const char*> CharLayers;
    for (const auto& Layer : ValidationLayers)
    {
        CharLayers.push_back(Layer.c_str());
    }

    if (!ValidationLayers.empty())
    {
        CreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
        CreateInfo.ppEnabledLayerNames = CharLayers.data();
    }
    else
    {
        CreateInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(PhysicalDevice, &CreateInfo, nullptr, &LogicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device!");
    }

    if (bGraphicsCapabilityRequired)
    {
        vkGetDeviceQueue(LogicalDevice, GraphicsQueueIndex, 0, &GraphicsQueue);
    }

    if (bComputeCapabilityRequired)
    {
        vkGetDeviceQueue(LogicalDevice, ComputeQueueIndex, 0, &ComputeQueue);
    }

    if (bTransferCapabilityRequired)
    {
        vkGetDeviceQueue(LogicalDevice, TransferQueueIndex, 0, &TransferQueue);
    }

    if (bSparseBindingCapabilityRequired)
    {
        vkGetDeviceQueue(LogicalDevice, SparseBindingQueueIndex, 0, &SparseBindingQueue);
    }

    if (bPresentCapabilityRequired)
    {
        vkGetDeviceQueue(LogicalDevice, PresentQueueIndex, 0, &PresentQueue);
    }

    ResourceAllocator = std::make_shared<FResourceAllocator>(PhysicalDevice, LogicalDevice, this);
}

void FContext::CreateDepthAndAAImages()
{
    /// Create Image and ImageView for AA
    VkFormat ColorFormat = Swapchain->GetImageFormat();

    CreateImage(Swapchain->GetWidth(), Swapchain->GetHeight(), 1, MSAASamples, ColorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ColorImage, ColorImageMemory);

    ColorImageView = CreateImageView(ColorImage, ColorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

    // Create Image and ImageView for Depth
    VkFormat DepthFormat = FindDepthFormat();

    CreateImage(Swapchain->GetWidth(), Swapchain->GetHeight(), 1, MSAASamples, DepthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, DepthImage, DepthImageMemory);
    DepthImageView = CreateImageView(DepthImage, DepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    TransitionImageLayout(DepthImage, DepthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

VkImageView FContext::CreateImageView(VkImage Image, VkFormat Format, VkImageAspectFlags AspectFlags, uint32_t MipLevels)
{
    VkImageViewCreateInfo ViewInfo{};
    ViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ViewInfo.image = Image;
    ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ViewInfo.format = Format;
    ViewInfo.subresourceRange.aspectMask = AspectFlags;
    ViewInfo.subresourceRange.baseMipLevel = 0;
    ViewInfo.subresourceRange.levelCount = MipLevels;
    ViewInfo.subresourceRange.baseArrayLayer = 0;
    ViewInfo.subresourceRange.layerCount = 1;

    VkImageView ImageView;
    if (vkCreateImageView(LogicalDevice, &ViewInfo, nullptr, &ImageView) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create texture image view!");
    }

    return ImageView;
}

void FContext::CreateRenderPass()
{
    VkAttachmentDescription ColorAttachment{};
    ColorAttachment.format = Swapchain->GetImageFormat();
    ColorAttachment.samples = MSAASamples;
    ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference ColorAttachmentRef{};
    ColorAttachmentRef.attachment = 0;
    ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription ColorAttachmentResolve{};
    ColorAttachmentResolve.format = Swapchain->GetImageFormat();
    ColorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    ColorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    ColorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ColorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    ColorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    ColorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ColorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference ColorAttachmentResolveRef{};
    ColorAttachmentResolveRef.attachment = 2;
    ColorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription DepthAttachment{};
    DepthAttachment.format = FindSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                                 VK_IMAGE_TILING_OPTIMAL,
                                                 VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    DepthAttachment.samples = MSAASamples;
    DepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    DepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    DepthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    DepthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    DepthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    DepthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference DepthAttachmentRef{};
    DepthAttachmentRef.attachment = 1;
    DepthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDependency Dependency{};
    Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    Dependency.dstSubpass = 0;
    Dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    Dependency.srcAccessMask = 0;
    Dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkSubpassDescription Subpass{};
    Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    Subpass.colorAttachmentCount = 1;
    Subpass.pColorAttachments = &ColorAttachmentRef;
    Subpass.pDepthStencilAttachment = &DepthAttachmentRef;
    Subpass.pResolveAttachments = &ColorAttachmentResolveRef;

    std::array<VkAttachmentDescription, 3> Attachments = {ColorAttachment, DepthAttachment, ColorAttachmentResolve};
    VkRenderPassCreateInfo RenderPassInfo{};
    RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    RenderPassInfo.attachmentCount = static_cast<uint32_t>(Attachments.size());
    RenderPassInfo.pAttachments = Attachments.data();
    RenderPassInfo.subpassCount = 1;
    RenderPassInfo.pSubpasses = &Subpass;
    RenderPassInfo.dependencyCount = 1;
    RenderPassInfo.pDependencies  = &Dependency;

    if (vkCreateRenderPass(LogicalDevice, &RenderPassInfo, nullptr, &RenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create render pass!");
    }
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
    FDescriptorSetLayout DescriptorSetLayout;

    DescriptorSetLayout.AddDescriptorLayout("Transform layout", {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT});
    DescriptorSetLayout.AddDescriptorLayout("Renderable layout", {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT});
    RenderableDescriptorSetLayout = DescriptorSetLayout.CreateDescriptorSetLayout(LogicalDevice);
    DescriptorSetLayout.Reset();

    DescriptorSetLayout.AddDescriptorLayout("Camera layout", {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT});
    DescriptorSetLayout.AddDescriptorLayout("Sampler layout", {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT});
    FrameDescriptorSetLayout = DescriptorSetLayout.CreateDescriptorSetLayout(LogicalDevice);
    DescriptorSetLayout.Reset();
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

    VkPipelineColorBlendStateCreateInfo ColorBlending{};
    ColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColorBlending.logicOpEnable = VK_FALSE;
    ColorBlending.logicOp = VK_LOGIC_OP_COPY;
    ColorBlending.attachmentCount = 1;
    ColorBlending.pAttachments = &ColorBlendAttachment;
    ColorBlending.blendConstants[0] = 0.f;
    ColorBlending.blendConstants[1] = 0.f;
    ColorBlending.blendConstants[2] = 0.f;
    ColorBlending.blendConstants[3] = 0.f;


    VkPipelineLayoutCreateInfo PipelineLayoutInfo{};
    std::vector<VkDescriptorSetLayout> PipelineSetLayouts = {FrameDescriptorSetLayout, RenderableDescriptorSetLayout};
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
    PipelineInfo.renderPass = RenderPass;
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

void FContext::CreateCommandPool()
{
    VkCommandPoolCreateInfo PoolInfo{};
    PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    PoolInfo.queueFamilyIndex = GraphicsQueueIndex;
    PoolInfo.flags = 0;

    if (vkCreateCommandPool(LogicalDevice, &PoolInfo, nullptr, &CommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void FContext::CreateImage(uint32_t Width, uint32_t Height, uint32_t MipLevels, VkSampleCountFlagBits NumSamples, VkFormat Format, VkImageTiling Tiling, VkImageUsageFlags Usage, VkMemoryPropertyFlags Properties, VkImage& Image, VkDeviceMemory& ImageMemory)
{
    VkImageCreateInfo ImageInfo{};
    ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageInfo.imageType = VK_IMAGE_TYPE_2D;
    ImageInfo.extent.width = Width;
    ImageInfo.extent.height = Height;
    ImageInfo.extent.depth = 1;
    ImageInfo.mipLevels = MipLevels;
    ImageInfo.arrayLayers = 1;
    ImageInfo.format = Format;
    ImageInfo.tiling = Tiling;
    ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageInfo.usage = Usage;
    ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ImageInfo.samples = NumSamples;

    if (vkCreateImage(LogicalDevice, &ImageInfo, nullptr, &Image) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create image!");
    }

    VkMemoryRequirements MemRequirements;
    vkGetImageMemoryRequirements(LogicalDevice, Image, &MemRequirements);

    VkMemoryAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    AllocInfo.allocationSize = MemRequirements.size;
    AllocInfo.memoryTypeIndex = ResourceAllocator->FindMemoryType(MemRequirements.memoryTypeBits, Properties);

    if (vkAllocateMemory(LogicalDevice, &AllocInfo, nullptr, &ImageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate image memory!");
    }

    vkBindImageMemory(LogicalDevice, Image, ImageMemory, 0);
}

VkFormat FContext::FindDepthFormat()
{
    return FindSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

void FContext::TransitionImageLayout(VkImage Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout, uint32_t MipLevels)
{
    VkCommandBuffer CommandBuffer = BeginSingleTimeCommands();

    VkImageMemoryBarrier Barrier{};
    Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    Barrier.oldLayout = OldLayout;
    Barrier.newLayout = NewLayout;
    Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.image = Image;
    Barrier.subresourceRange.baseMipLevel = 0;
    Barrier.subresourceRange.levelCount = MipLevels;
    Barrier.subresourceRange.baseArrayLayer = 0;
    Barrier.subresourceRange.layerCount = 1;

    if (NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (HasStensilComponent(Format))
        {
            Barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkPipelineStageFlags SourceStage;
    VkPipelineStageFlags DestinationStage;

    if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        Barrier.srcAccessMask = 0;
        Barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        DestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        SourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        DestinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if(OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        Barrier.srcAccessMask = 0;
        Barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        DestinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::invalid_argument("Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(CommandBuffer, SourceStage, DestinationStage, 0, 0, nullptr, 0, nullptr, 1, &Barrier);

    EndSingleTimeCommand(CommandBuffer);
}

bool FContext::HasStensilComponent(VkFormat Format)
{
    return Format == VK_FORMAT_D32_SFLOAT_S8_UINT || Format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkCommandBuffer FContext::BeginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandPool = CommandPool;
    AllocInfo.commandBufferCount = 1;

    VkCommandBuffer CommandBuffer;
    vkAllocateCommandBuffers(LogicalDevice, &AllocInfo, &CommandBuffer);

    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(CommandBuffer, &BeginInfo);

    return CommandBuffer;
}

void FContext::EndSingleTimeCommand(VkCommandBuffer CommandBuffer)
{
    vkEndCommandBuffer(CommandBuffer);

    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CommandBuffer;

    vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(GraphicsQueue);

    vkFreeCommandBuffers(LogicalDevice, CommandPool, 1, &CommandBuffer);
}

void FContext::CreateFramebuffers()
{
    SwapChainFramebuffers.resize(Swapchain->Size());
    for (std::size_t i = 0; i < Swapchain->Size(); ++i) {
        std::array<VkImageView, 3> Attachments = {ColorImageView, DepthImageView, Swapchain->GetImageViews()[i]};

        VkFramebufferCreateInfo FramebufferInfo{};
        FramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        FramebufferInfo.renderPass = RenderPass;
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

void FContext::CreateTextureImage(std::string& TexturePath)
{
    int TexWidth, TexHeight, TexChannels;
    stbi_uc* Pixels = stbi_load(TexturePath.c_str(), &TexWidth, &TexHeight, &TexChannels, STBI_rgb_alpha);
    VkDeviceSize ImageSize = TexWidth * TexHeight * 4;
    MipLevels = static_cast<uint32_t>(std::floor(static_cast<float>(std::log2(std::max(TexWidth, TexHeight))))) + 1;

    if (!Pixels)
    {
        throw std::runtime_error("Failed to load texture image!");
    }

    FBuffer TempStagingBuffer = ResourceAllocator->CreateBuffer(ImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *Data;
    vkMapMemory(LogicalDevice, TempStagingBuffer.Memory, 0, ImageSize, 0, &Data);
    memcpy(Data, Pixels, static_cast<size_t>(ImageSize));
    vkUnmapMemory(LogicalDevice, TempStagingBuffer.Memory);
    stbi_image_free(Pixels);

    CreateImage(TexWidth, TexHeight, MipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, TextureImage, TextureImageMemory);

    TransitionImageLayout(TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, MipLevels);
    CopyBufferToImage(TempStagingBuffer, TextureImage, static_cast<uint32_t>(TexWidth), static_cast<uint32_t>(TexHeight));

    GenerateMipmaps(TextureImage, VK_FORMAT_R8G8B8A8_SRGB, TexWidth, TexHeight, MipLevels);
    ResourceAllocator->DestroyBuffer(TempStagingBuffer);
}

void FContext::CopyBufferToImage(FBuffer &Buffer, VkImage Image, uint32_t Width, uint32_t Height)
{
    VkCommandBuffer CommandBuffer = BeginSingleTimeCommands();

    VkBufferImageCopy Region{};
    Region.bufferOffset = 0;
    Region.bufferRowLength = 0;
    Region.bufferImageHeight = 0;

    Region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    Region.imageSubresource.mipLevel = 0;
    Region.imageSubresource.baseArrayLayer = 0;
    Region.imageSubresource.layerCount = 1;

    Region.imageOffset = {0, 0, 0};
    Region.imageExtent = {Width, Height, 1};

    vkCmdCopyBufferToImage(CommandBuffer, Buffer.Buffer, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region);

    EndSingleTimeCommand(CommandBuffer);
}

void FContext::GenerateMipmaps(VkImage Image, VkFormat ImageFormat, int32_t TexWidth, int32_t TexHeight, uint32_t mipLevels)
{
    VkFormatProperties FormatFroperties;
    vkGetPhysicalDeviceFormatProperties(PhysicalDevice, ImageFormat, &FormatFroperties);

    if (!(FormatFroperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    {
        throw std::runtime_error("Texture image format does not support linear blitting!");
    }

    VkCommandBuffer CommandBuffer = BeginSingleTimeCommands();

    VkImageMemoryBarrier Barrier{};
    Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    Barrier.image = Image;
    Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    Barrier.subresourceRange.baseArrayLayer = 0;
    Barrier.subresourceRange.layerCount = 1;
    Barrier.subresourceRange.levelCount = 1;

    int32_t MipWidth = TexWidth;
    int32_t MipHeight = TexHeight;

    for (uint32_t i = 1; i < MipLevels; ++i)
    {
        Barrier.subresourceRange.baseMipLevel = i - 1;
        Barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        Barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        Barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &Barrier);

        VkImageBlit Blit{};
        Blit.srcOffsets[0] = {0, 0, 0};
        Blit.srcOffsets[1] = {MipWidth, MipHeight, 1};
        Blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        Blit.srcSubresource.mipLevel = i - 1;
        Blit.srcSubresource.baseArrayLayer = 0;
        Blit.srcSubresource.layerCount = 1;
        Blit.dstOffsets[0] = {0, 0, 0};
        Blit.dstOffsets[1] = {MipWidth > 1 ? MipWidth / 2 : 1, MipHeight > 1 ? MipHeight / 2 : 1, 1};
        Blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        Blit.dstSubresource.mipLevel = i;
        Blit.dstSubresource.baseArrayLayer = 0;
        Blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(CommandBuffer, Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Blit, VK_FILTER_LINEAR);

        Barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        Barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        Barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,nullptr, 0, nullptr, 1, &Barrier);

        if (MipWidth > 1)
        {
            MipWidth /= 2;
        }

        if (MipHeight > 1)
        {
            MipHeight /= 2;
        }
    }

    Barrier.subresourceRange.baseMipLevel = MipLevels - 1;
    Barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    Barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,nullptr, 0, nullptr, 1, &Barrier);

    EndSingleTimeCommand(CommandBuffer);
}

void FContext::LoadModelDataToGPU()
{
    auto MeshSystem = ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMeshSystem>();

    for(auto Mesh : *MeshSystem)
    {
        MeshSystem->LoadToGPU(Mesh);
    }

}

void FContext::CreateTextureImageView()
{
    TextureImageView = CreateImageView(TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, MipLevels);
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
    VkDeviceSize RenderableBufferSize = Coordinator.Size<ECS::COMPONENTS::FRenderableComponent>();

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

    AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(Swapchain->Size() * ModelsCount));
    AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(Swapchain->Size() * ModelsCount));
    AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(Swapchain->Size() * ModelsCount));
    AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(Swapchain->Size()));

    std::vector<VkDescriptorPoolSize> PoolSizes{};
    for (auto Entry : Descriptors)
    {
        PoolSizes.push_back({Entry.first, Entry.second});
    }

    VkDescriptorPoolCreateInfo PoolInfo{};
    PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    PoolInfo.poolSizeCount = static_cast<uint32_t>(PoolSizes.size());
    PoolInfo.pPoolSizes = PoolSizes.data();
    PoolInfo.maxSets = static_cast<uint32_t>(Swapchain->Size() * ModelsCount + Swapchain->Size());

    if (vkCreateDescriptorPool(LogicalDevice, &PoolInfo, nullptr, &DescriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

void FContext::CreateDescriptorSet()
{
    auto ModelsCount = ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMeshSystem>()->Size();

    /// Create and update descriptor sets for renderable objects
    std::vector<VkDescriptorSetLayout> RenderableLayouts(Swapchain->Size() * ModelsCount, RenderableDescriptorSetLayout);
    VkDescriptorSetAllocateInfo RenderableAllocInfo{};
    RenderableAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    RenderableAllocInfo.descriptorPool = DescriptorPool;
    RenderableAllocInfo.descriptorSetCount = static_cast<uint32_t>(RenderableLayouts.size());
    RenderableAllocInfo.pSetLayouts = RenderableLayouts.data();

    RenderebleDescriptorSet.resize(RenderableLayouts.size());
    if (vkAllocateDescriptorSets(LogicalDevice, &RenderableAllocInfo, RenderebleDescriptorSet.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < Swapchain->Size(); ++i)
    {
        for (uint32_t j = 0; j < ModelsCount; ++j)
        {
            VkDescriptorBufferInfo TransformBufferInfo{};
            TransformBufferInfo.buffer = DeviceTransformBuffers[i].Buffer;
            TransformBufferInfo.offset = sizeof(ECS::COMPONENTS::FDeviceTransformComponent) * j;
            TransformBufferInfo.range = sizeof(ECS::COMPONENTS::FDeviceTransformComponent);

            VkDescriptorBufferInfo RenderableBufferInfo{};
            RenderableBufferInfo.buffer = DeviceRenderableBuffers[i].Buffer;
            RenderableBufferInfo.offset = sizeof(Renderable) * j;
            RenderableBufferInfo.range = sizeof(Renderable);

            std::vector<VkWriteDescriptorSet> DescriptorWrites{2};
            DescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            DescriptorWrites[0].dstSet = RenderebleDescriptorSet[j * Swapchain->Size() + i];
            DescriptorWrites[0].dstBinding = 0;
            DescriptorWrites[0].dstArrayElement = 0;
            DescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            DescriptorWrites[0].descriptorCount = 1;
            DescriptorWrites[0].pBufferInfo = &TransformBufferInfo;

            DescriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            DescriptorWrites[1].dstSet = RenderebleDescriptorSet[j * Swapchain->Size() + i];
            DescriptorWrites[1].dstBinding = 1;
            DescriptorWrites[1].dstArrayElement = 0;
            DescriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            DescriptorWrites[1].descriptorCount = 1;
            DescriptorWrites[1].pBufferInfo = &RenderableBufferInfo;

            vkUpdateDescriptorSets(LogicalDevice, static_cast<uint32_t>(DescriptorWrites.size()), DescriptorWrites.data(), 0, nullptr);
        }
    }

    /// Create and update descriptor sets that will be bound once per frame
    std::vector<VkDescriptorSetLayout> FrameLayouts(Swapchain->Size(), FrameDescriptorSetLayout);
    VkDescriptorSetAllocateInfo FrameAllocInfo{};
    FrameAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    FrameAllocInfo.descriptorPool = DescriptorPool;
    FrameAllocInfo.descriptorSetCount = static_cast<uint32_t>(FrameLayouts.size());
    FrameAllocInfo.pSetLayouts = FrameLayouts.data();

    FrameDescriptorSet.resize(FrameLayouts.size());
    if (vkAllocateDescriptorSets(LogicalDevice, &FrameAllocInfo, FrameDescriptorSet.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < Swapchain->Size(); ++i)
    {

        VkDescriptorBufferInfo CameraBufferInfo{};
        CameraBufferInfo.buffer = DeviceCameraBuffers[i].Buffer;
        CameraBufferInfo.offset = 0;
        CameraBufferInfo.range = sizeof(ECS::COMPONENTS::FDeviceCameraComponent);

        VkDescriptorImageInfo ImageBufferInfo{};
        ImageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ImageBufferInfo.imageView = TextureImageView;
        ImageBufferInfo.sampler = TextureSampler;

        std::array<VkWriteDescriptorSet, 2> DescriptorWrites{};

        DescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        DescriptorWrites[0].dstSet = FrameDescriptorSet[i];
        DescriptorWrites[0].dstBinding = 0;
        DescriptorWrites[0].dstArrayElement = 0;
        DescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        DescriptorWrites[0].descriptorCount = 1;
        DescriptorWrites[0].pBufferInfo = &CameraBufferInfo;

        DescriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        DescriptorWrites[1].dstSet = FrameDescriptorSet[i];
        DescriptorWrites[1].dstBinding = 1;
        DescriptorWrites[1].dstArrayElement = 0;
        DescriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        DescriptorWrites[1].descriptorCount = 1;
        DescriptorWrites[1].pImageInfo = &ImageBufferInfo;

        vkUpdateDescriptorSets(LogicalDevice, static_cast<uint32_t>(DescriptorWrites.size()), DescriptorWrites.data(), 0, nullptr);
    }
}

void FContext::CreateCommandBuffers()
{
    CommandBuffers.resize(SwapChainFramebuffers.size());

    VkCommandBufferAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.commandPool = CommandPool;
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandBufferCount = (uint32_t) CommandBuffers.size();

    if (vkAllocateCommandBuffers(LogicalDevice, &AllocInfo, CommandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffers!");
    }

    for (std::size_t i = 0; i <CommandBuffers.size(); ++i)
    {
        VkCommandBufferBeginInfo BeginInfo{};
        BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        BeginInfo.flags = 0;
        BeginInfo.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(CommandBuffers[i], &BeginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo RenderPassInfo{};
        RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        RenderPassInfo.renderPass = RenderPass;
        RenderPassInfo.framebuffer = SwapChainFramebuffers[i];
        RenderPassInfo.renderArea.offset = {0, 0};
        RenderPassInfo.renderArea.extent = Swapchain->GetExtent2D();

        std::array<VkClearValue, 2> ClearValues{};
        ClearValues[0].color = {0.f, 0.f, 0.f, 1.f};
        ClearValues[1].depthStencil = {1.f, 0};
        RenderPassInfo.clearValueCount = static_cast<uint32_t>(ClearValues.size());
        RenderPassInfo.pClearValues = ClearValues.data();

        vkCmdBeginRenderPass(CommandBuffers[i], &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline);

        vkCmdBindDescriptorSets(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout, 0, 1, &FrameDescriptorSet[i], 0,
                                nullptr);

        auto MeshSystem = ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMeshSystem>();

        uint32_t j = 0;
        for (auto Entity : *MeshSystem)
        {
            vkCmdBindDescriptorSets(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout, 1, 1, &RenderebleDescriptorSet[j * Swapchain->Size() + i], 0,
                                    nullptr);
            auto& Coordinator = ECS::GetCoordinator();
            auto MeshSystem = Coordinator.GetSystem<ECS::SYSTEMS::FMeshSystem>();

            MeshSystem->Bind(Entity, CommandBuffers[i]);
            MeshSystem->Draw(Entity, CommandBuffers[i]);
            ++j;
        }
        vkCmdEndRenderPass(CommandBuffers[i]);

        if (vkEndCommandBuffer(CommandBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to record command buffer!");
        }
    }
}

void FContext::CreateSyncObjects()
{
    ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    RenderingFinishedFences.resize(MAX_FRAMES_IN_FLIGHT);
    ImagesInFlight.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo SemaphoreInfo{};
    SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo FenceInfo{};
    FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (vkCreateSemaphore(LogicalDevice, &SemaphoreInfo, nullptr, &ImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(LogicalDevice, &SemaphoreInfo, nullptr, &RenderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(LogicalDevice, &FenceInfo, nullptr, &RenderingFinishedFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create synchronization objects for a frame!");
        }
    }
}

void FContext::Render()
{
    /// Previous rendering iteration of the frame might still be in use, so we wait for it
    vkWaitForFences(LogicalDevice, 1, &RenderingFinishedFences[CurrentFrame], VK_TRUE, UINT64_MAX);

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
    ImagesInFlight[ImageIndex] = RenderingFinishedFences[CurrentFrame];

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

    /// Submit rendering. When rendering finished, apropriate fence will be signalled
    if (vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, RenderingFinishedFences[CurrentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }
}

void FContext::Present()
{
    VkSemaphore SignalSemaphores[] = {RenderFinishedSemaphores[CurrentFrame]};

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
    vkDestroyImageView(LogicalDevice, ColorImageView, nullptr);
    vkDestroyImage(LogicalDevice, ColorImage, nullptr);
    vkFreeMemory(LogicalDevice, ColorImageMemory, nullptr);

    vkDestroyImageView(LogicalDevice, DepthImageView, nullptr);
    vkDestroyImage(LogicalDevice, DepthImage, nullptr);
    vkFreeMemory(LogicalDevice, DepthImageMemory, nullptr);

    for (auto Framebuffer : SwapChainFramebuffers)
    {
        vkDestroyFramebuffer(LogicalDevice, Framebuffer, nullptr);
    }

    vkFreeCommandBuffers(LogicalDevice, CommandPool, static_cast<uint32_t>(CommandBuffers.size()), CommandBuffers.data());

    vkDestroyPipeline(LogicalDevice, GraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(LogicalDevice, PipelineLayout, nullptr);
    vkDestroyRenderPass(LogicalDevice, RenderPass, nullptr);

    for (size_t i = 0; i < Swapchain->Size(); ++i)
    {
        ResourceAllocator->DestroyBuffer(DeviceTransformBuffers[i]);
        ResourceAllocator->DestroyBuffer(DeviceCameraBuffers[i]);
        ResourceAllocator->DestroyBuffer(DeviceRenderableBuffers[i]);
    }

    Swapchain = nullptr;

    vkDestroyDescriptorPool(LogicalDevice, DescriptorPool, nullptr);
}

void FContext::UpdateUniformBuffer(uint32_t CurrentImage)
{
    auto& Coordinator = ECS::GetCoordinator();
    auto CameraSystem = Coordinator.GetSystem<ECS::SYSTEMS::FCameraSystem>();

    auto DeviceTransformComponentsData = Coordinator.Data<ECS::COMPONENTS::FDeviceTransformComponent>();
    auto DeviceTransformComponentsSize = Coordinator.Size<ECS::COMPONENTS::FDeviceTransformComponent>();

    auto DeviceCameraComponentsData = Coordinator.Data<ECS::COMPONENTS::FDeviceCameraComponent>();
    auto DeviceCameraComponentsSize = Coordinator.Size<ECS::COMPONENTS::FDeviceCameraComponent>();

    auto RenderableComponentData = Coordinator.Data<ECS::COMPONENTS::FRenderableComponent>();
    auto RenderableComponentSize = Coordinator.Size<ECS::COMPONENTS::FRenderableComponent>();

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

void FContext::AddDescriptor(VkDescriptorType Type, uint32_t Count)
{
    Descriptors[Type] += Count;
}

void FContext::FreeData(FBuffer Buffer)
{
    ResourceAllocator->DestroyBuffer(Buffer);
}

FBuffer FContext::LoadDataIntoGPU(void* Data, uint32_t Size)
{
    VkDeviceSize BufferSize = Size;

    /// Create staging buffer
    FBuffer StagingBuffer = ResourceAllocator->CreateBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* StagingData;
    vkMapMemory(LogicalDevice, StagingBuffer.Memory, 0, BufferSize, 0, &StagingData);
    memcpy(StagingData, Data, (std::size_t)BufferSize);
    vkUnmapMemory(LogicalDevice, StagingBuffer.Memory);

    FBuffer Buffer = ResourceAllocator->CreateBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    ResourceAllocator->CopyBuffer(StagingBuffer, Buffer, BufferSize);
    ResourceAllocator->DestroyBuffer(StagingBuffer);

    return Buffer;
}

void FContext::DestroyDebugUtilsMessengerEXT()
{
    FunctionLoader->vkDestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
}

void FContext::CleanUp()
{
    CleanUpSwapChain();

    vkDestroySampler(LogicalDevice, TextureSampler, nullptr);
    vkDestroyImageView(LogicalDevice, TextureImageView, nullptr);

    vkDestroyImage(LogicalDevice, TextureImage, nullptr);
    vkFreeMemory(LogicalDevice, TextureImageMemory, nullptr);

    vkDestroyDescriptorSetLayout(LogicalDevice, FrameDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(LogicalDevice, RenderableDescriptorSetLayout, nullptr);

    auto& Coordinator = ECS::GetCoordinator();
    Coordinator.GetSystem<ECS::SYSTEMS::FMeshSystem>()->FreeAllDeviceData();

    for(std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(LogicalDevice, RenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(LogicalDevice, ImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(LogicalDevice, RenderingFinishedFences[i], nullptr);
    }

    vkDestroyCommandPool(LogicalDevice, CommandPool, nullptr);
    vkDestroyDevice(LogicalDevice, nullptr);

    if (!ValidationLayers.empty())
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
