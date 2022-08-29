#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

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

#include <stdexcept>
#include <iostream>
#include <set>
#include <unordered_map>

static FVulkanContext Context{};

PFN_vkCreateDebugUtilsMessengerEXT FVulkanContext::vkCreateDebugUtilsMessengerEXT = nullptr;
PFN_vkDestroyDebugUtilsMessengerEXT FVulkanContext::vkDestroyDebugUtilsMessengerEXT = nullptr;
PFN_vkSetDebugUtilsObjectNameEXT FVulkanContext::vkSetDebugUtilsObjectNameEXT = nullptr;
PFN_vkCreateAccelerationStructureKHR FVulkanContext::vkCreateAccelerationStructureKHR  = nullptr;
PFN_vkDestroyAccelerationStructureKHR FVulkanContext::vkDestroyAccelerationStructureKHR = nullptr;
PFN_vkGetAccelerationStructureBuildSizesKHR FVulkanContext::vkGetAccelerationStructureBuildSizesKHR = nullptr;
PFN_vkCmdBuildAccelerationStructuresKHR FVulkanContext::vkCmdBuildAccelerationStructuresKHR = nullptr;
PFN_vkCmdWriteAccelerationStructuresPropertiesKHR FVulkanContext::vkCmdWriteAccelerationStructuresPropertiesKHR = nullptr;
PFN_vkGetAccelerationStructureDeviceAddressKHR FVulkanContext::vkGetAccelerationStructureDeviceAddressKHR = nullptr;
PFN_vkCmdCopyAccelerationStructureKHR FVulkanContext::vkCmdCopyAccelerationStructureKHR = nullptr;
PFN_vkGetRayTracingShaderGroupHandlesKHR FVulkanContext::vkGetRayTracingShaderGroupHandlesKHR = nullptr;
PFN_vkCreateRayTracingPipelinesKHR FVulkanContext::vkCreateRayTracingPipelinesKHR = nullptr;
PFN_vkCmdTraceRaysKHR FVulkanContext::vkCmdTraceRaysKHR = nullptr;

namespace LAYOUT_SETS
{
    const std::string PER_FRAME_LAYOUT_NAME = "Per-frame layout";
    const std::string PER_RENDERABLE_LAYOUT_NAME = "Per-renderable layout";
    const std::string PASSTHROUGH_LAYOUT_NAME = "Passthrough layout";
}

namespace LAYOUTS
{
    const std::string TEXTURE_SAMPLER_LAYOUT_NAME = "Texture sampler layout";
    const std::string CAMERA_LAYOUT_NAME = "Camera layout";
    const std::string TRANSFORM_LAYOUT_NAME = "Transform layout";
    const std::string RENDERABLE_LAYOUT_NAME = "Renderable layout";
}

FVulkanContext& GetContext()
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

void FVulkanContext::Init(GLFWwindow *Window, FController *Controller)
{
    this->Window = Window;
    this->Controller = Controller;
    ImageManager = std::make_shared<FImageManager>();
    ImageManager->Init(*this);

    try {
        CreateInstance();
        LoadFunctionPointers();
        SetupDebugMessenger();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDevice();
        CommandBufferManager = std::make_shared<FCommandBufferManager>(LogicalDevice, this, GraphicsQueue, GraphicsQueueIndex);
        Swapchain = std::make_shared<FSwapchain>(*this, PhysicalDevice, LogicalDevice, Surface, Window, GraphicsQueueIndex, PresentQueueIndex, VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_PRESENT_MODE_MAILBOX_KHR);
        CreateDepthAndAAImages();
        CreateRenderPass();
        CreatePassthroughRenderPass();
        CreateImguiRenderpasss();
        CreateDescriptorSetLayouts();
        CreateGraphicsPipeline();
        CreatePassthroughPipeline();
        ImageManager->LoadImageFromFile(TextureImage, TexturePath);
        CreateRenderFramebuffers();
        CreatePassthroughFramebuffers();
        CreateImguiFramebuffers();
        CreateTextureSampler();
        LoadModelDataToGPU();
        CreateUniformBuffers();
        CreateDescriptorPool();
        CreateImguiDescriptorPool();
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

void FVulkanContext::CreateInstance()
{
    VulkanContextOptions.AddInstanceLayer("VK_LAYER_KHRONOS_validation");

#ifndef NDEBUG
    VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo = {};
    DebugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    DebugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    DebugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    DebugCreateInfo.pfnUserCallback = DebugCallback;
    VulkanContextOptions.AddInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, &DebugCreateInfo, sizeof(VkDebugUtilsMessengerCreateInfoEXT));
#endif

    // Resolve and add extensions and layers
    uint32_t Counter = 0;
    auto ExtensionsRequiredByGLFW = glfwGetRequiredInstanceExtensions(&Counter);
    for (uint32_t i = 0; i < Counter; ++i)
    {
        VulkanContextOptions.AddInstanceExtension(ExtensionsRequiredByGLFW[i]);
    }

    Instance = CreateVkInstance("Hello Triangle", {1, 0, 0}, "No Engine", {1, 0, 0}, VK_API_VERSION_1_0, VulkanContextOptions);
}

void FVulkanContext::LoadFunctionPointers()
{
    V::LoadVkFunctions(Instance);
}

void FVulkanContext::SetupDebugMessenger()
{
#ifndef NDEBUG
    auto* DebugCreateInfo = VulkanContextOptions.GetExtensionStructurePtr<VkDebugUtilsMessengerCreateInfoEXT>(VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT);

    V::vkCreateDebugUtilsMessengerEXT(Instance, DebugCreateInfo, nullptr, &DebugMessenger);
#endif
}

void FVulkanContext::CreateSurface()
{
    VkResult Result = glfwCreateWindowSurface(Instance, Window, nullptr, &Surface);
    assert((Result == VK_SUCCESS) && "Failed to create window surface!");
}

void FVulkanContext::PickPhysicalDevice()
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

void FVulkanContext::QueuePhysicalDeviceProperties()
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

bool FVulkanContext::CheckInstanceLayersSupport(const std::vector<const char*>& Layers)
{
    /// Check supported Layers
    uint32_t LayerCount;
    vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);

    std::vector<VkLayerProperties> AvailableLayers(LayerCount);
    vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

    for (const auto &LayerName : Layers) {
        bool LayerFound = false;

        for (const auto &LayerProperties : AvailableLayers) {
            if (std::string(LayerName) == std::string(LayerProperties.layerName)) {
                LayerFound = true;
                break;
            }
        }
        if (!LayerFound) {
            return false;
        }
    }

    return true;
}

bool FVulkanContext::CheckDeviceExtensionsSupport(VkPhysicalDevice Device)
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

VkInstance FVulkanContext::CreateVkInstance(const std::string& AppName, const FVersion3& AppVersion, const std::string& EngineName, const FVersion3& EngineVersion, uint32_t ApiVersion, FVulkanContextOptions& Options)
{
    /// Check whether instance supports requested layers
    auto CharLayers = VulkanContextOptions.GetInstanceLayers();
    if (!CheckInstanceLayersSupport(CharLayers))
    {
        assert(0 && "Validation layers requested, but not available!");
    }

    /// Fill in instance creation data
    VkApplicationInfo AppInfo{};
    AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    AppInfo.pApplicationName = AppName.c_str();
    AppInfo.applicationVersion = VK_MAKE_VERSION(AppVersion.Major, AppVersion.Minor, AppVersion.Patch);
    AppInfo.pEngineName = EngineName.c_str();
    AppInfo.engineVersion = VK_MAKE_VERSION(EngineVersion.Major, EngineVersion.Minor, EngineVersion.Patch);
    AppInfo.apiVersion = ApiVersion;

    /// Generate char* names for layers and extensions
    std::vector<const char*> CharExtensions = VulkanContextOptions.GetInstanceExtensionsList();

    VkInstanceCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    CreateInfo.pApplicationInfo = &AppInfo;

    CreateInfo.enabledExtensionCount = static_cast<uint32_t>(CharExtensions.size());
    CreateInfo.ppEnabledExtensionNames = CharExtensions.data();
    VulkanContextOptions.BuildInstancePNextChain(reinterpret_cast<BaseVulkanStructure*>(&CreateInfo));

    CreateInfo.enabledLayerCount = CharLayers.size();
    CreateInfo.ppEnabledLayerNames = CharLayers.data();

    VkInstance ResultingInstance;
    VkResult Result = vkCreateInstance(&CreateInfo, nullptr, &ResultingInstance);
    assert((Result == VK_SUCCESS) && "Failed to create instance!");
    return ResultingInstance;
}

bool FVulkanContext::CheckDeviceQueueSupport(VkPhysicalDevice Device)
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

void FVulkanContext::CreateLogicalDevice()
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
    DescriptorSetManager = std::make_shared<FDescriptorSetManager>(LogicalDevice);
}

void FVulkanContext::CreateDepthAndAAImages()
{
    /// Create Image and ImageView for AA
    auto Width = Swapchain->GetWidth();
    auto Height = Swapchain->GetHeight();

    ImageManager->CreateImage(ColorImage, Width, Height, false, MSAASamples, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              VK_IMAGE_ASPECT_COLOR_BIT);
    V::SetName(LogicalDevice, (*ImageManager)(ColorImage).Image, "V_ColorImage");
    V::SetName(LogicalDevice, (*ImageManager)(ColorImage).View, "V_ColorImagView");

    ImageManager->CreateImage(NormalsImage, Width, Height, false, MSAASamples, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                            VK_IMAGE_ASPECT_COLOR_BIT);
    V::SetName(LogicalDevice, (*ImageManager)(NormalsImage).Image, "V_NormalsImage");
    V::SetName(LogicalDevice, (*ImageManager)(NormalsImage).View, "V_NormalsImageView");

    ImageManager->CreateImage(RenderableIndexImage, Width, Height, false, MSAASamples, VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL,
                                                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                    VK_IMAGE_ASPECT_COLOR_BIT);
    V::SetName(LogicalDevice, (*ImageManager)(RenderableIndexImage).Image, "V_RenderableIndexImage");
    V::SetName(LogicalDevice, (*ImageManager)(RenderableIndexImage).View, "V_RenderableIndexImageView");

    ImageManager->CreateImage(UtilityImageR32, Width, Height, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                               VK_IMAGE_ASPECT_COLOR_BIT);
    V::SetName(LogicalDevice, (*ImageManager)(UtilityImageR32).Image, "V_UtilityImageR32");
    V::SetName(LogicalDevice, (*ImageManager)(UtilityImageR32).View, "V_UtilityImageR32View");

    auto& UtilityImgR32 = (*ImageManager)(UtilityImageR32);
    UtilityImgR32.Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    ImageManager->CreateImage(ResolvedColorImage, Width, Height, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              VK_IMAGE_ASPECT_COLOR_BIT);
    V::SetName(LogicalDevice, (*ImageManager)(ResolvedColorImage).Image, "V_ResolvedColorImage");
    V::SetName(LogicalDevice, (*ImageManager)(ResolvedColorImage).View, "V_ResolvedColorImageView");

    ImageManager->CreateImage(UtilityImageR8G8B8A8_SRGB, Width, Height, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                                                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                         VK_IMAGE_ASPECT_COLOR_BIT);
    V::SetName(LogicalDevice, (*ImageManager)(UtilityImageR8G8B8A8_SRGB).Image, "V_UtilityImageR8G8B8A8_SRGB");
    V::SetName(LogicalDevice, (*ImageManager)(UtilityImageR8G8B8A8_SRGB).View, "V_UtilityImageR8G8B8A8_SRGBView");

    /// Create Image and ImageView for Depth
    VkFormat DepthFormat = FindDepthFormat();
    ImageManager->CreateImage(DepthImage, Width, Height, false, MSAASamples, DepthFormat, VK_IMAGE_TILING_OPTIMAL,
                                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                          VK_IMAGE_ASPECT_DEPTH_BIT);


    V::SetName(LogicalDevice, (*ImageManager)(DepthImage).Image, "V_DepthImage");
    V::SetName(LogicalDevice, (*ImageManager)(DepthImage).View, "V_DepthImageView");
    auto& DepthImg = (*ImageManager)(DepthImage);
    DepthImg.Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void FVulkanContext::CreatePassthroughRenderPass()
{
    PassthroughRenderPass = std::make_shared<FRenderPass>();
    PassthroughRenderPass->AddImageAsAttachment(Swapchain->Images[0], AttachmentType::Color, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    PassthroughRenderPass->Construct(LogicalDevice);
    V::SetName(LogicalDevice, PassthroughRenderPass->RenderPass, "V_PassthroughRenderpass");
}

void FVulkanContext::CreateRenderPass()
{
    RenderPass = std::make_shared<FRenderPass>();
    RenderPass->AddImageAsAttachment((*ImageManager)(ColorImage), AttachmentType::Color, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    RenderPass->AddImageAsAttachment((*ImageManager)(NormalsImage), AttachmentType::Color, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    RenderPass->AddImageAsAttachment((*ImageManager)(RenderableIndexImage), AttachmentType::Color, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    RenderPass->AddImageAsAttachment((*ImageManager)(DepthImage), AttachmentType::DepthStencil, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    RenderPass->AddImageAsAttachment((*ImageManager)(ResolvedColorImage), AttachmentType::Resolve, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    RenderPass->Construct(LogicalDevice);
    V::SetName(LogicalDevice, RenderPass->RenderPass, "V_RenderRenderpass");
}

VkFormat FVulkanContext::FindSupportedFormat(const std::vector<VkFormat>& Candidates, VkImageTiling Tiling, VkFormatFeatureFlags Features)
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

void FVulkanContext::CreateDescriptorSetLayouts()
{
    DescriptorSetManager->AddDescriptorLayout(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME, 0, LAYOUTS::CAMERA_LAYOUT_NAME,
                                              {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT});
    DescriptorSetManager->AddDescriptorLayout(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME, 0, LAYOUTS::TEXTURE_SAMPLER_LAYOUT_NAME,
                                              {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT});

    DescriptorSetManager->AddDescriptorLayout(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME, 1, LAYOUTS::TRANSFORM_LAYOUT_NAME,
                                              {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT});
    DescriptorSetManager->AddDescriptorLayout(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME, 1, LAYOUTS::RENDERABLE_LAYOUT_NAME,
                                              {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT});

    DescriptorSetManager->AddDescriptorLayout(LAYOUT_SETS::PASSTHROUGH_LAYOUT_NAME, 0, LAYOUTS::TEXTURE_SAMPLER_LAYOUT_NAME,
                                              {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT});

    DescriptorSetManager->CreateDescriptorSetLayouts();
}

void FVulkanContext::CreatePassthroughPipeline()
{
    PassthroughPipeline.AddShader("../shaders/passthrough_vert.spv", eShaderType::VERTEX);
    PassthroughPipeline.AddShader("../shaders/passthrough_frag.spv", eShaderType::FRAGMENT);

    PassthroughPipeline.SetExtent2D(Swapchain->GetExtent2D());
    PassthroughPipeline.SetWidth(Swapchain->GetWidth());
    PassthroughPipeline.SetHeight(Swapchain->GetHeight());
    PassthroughPipeline.SetBlendAttachmentsCount(1);
    PassthroughPipeline.AddDescriptorSetLayout(DescriptorSetManager->GetVkDescriptorSetLayout(LAYOUT_SETS::PASSTHROUGH_LAYOUT_NAME));
    PassthroughPipeline.CreateGraphicsPipeline(LogicalDevice, PassthroughRenderPass->RenderPass);
}

void FVulkanContext::CreateGraphicsPipeline()
{
    GraphicsPipeline.AddShader("../shaders/triangle_vert.spv", eShaderType::VERTEX);
    GraphicsPipeline.AddShader("../shaders/triangle_frag.spv", eShaderType::FRAGMENT);

    auto AttributeDescriptions = FVertex::GetAttributeDescriptions();
    for (auto& Entry : AttributeDescriptions)
    {
        GraphicsPipeline.AddVertexInputAttributeDescription(Entry);
    }
    GraphicsPipeline.AddVertexInputBindingDescription(FVertex::GetBindingDescription());

    GraphicsPipeline.SetMSAA(Context.MSAASamples);
    GraphicsPipeline.SetExtent2D(Swapchain->GetExtent2D());
    GraphicsPipeline.SetWidth(Swapchain->GetWidth());
    GraphicsPipeline.SetHeight(Swapchain->GetHeight());
    GraphicsPipeline.SetBlendAttachmentsCount(3);
    GraphicsPipeline.AddDescriptorSetLayout(DescriptorSetManager->GetVkDescriptorSetLayout(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME));
    GraphicsPipeline.AddDescriptorSetLayout(DescriptorSetManager->GetVkDescriptorSetLayout(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME));
    GraphicsPipeline.CreateGraphicsPipeline(LogicalDevice, RenderPass->RenderPass);
}

VkFormat FVulkanContext::FindDepthFormat()
{
    return FindSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool FVulkanContext::HasStensilComponent(VkFormat Format)
{
    return Format == VK_FORMAT_D32_SFLOAT_S8_UINT || Format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void FVulkanContext::CreateRenderFramebuffers()
{
    SwapChainFramebuffers.resize(Swapchain->Size());
    for (std::size_t i = 0; i < Swapchain->Size(); ++i) {
        std::vector<VkImageView> Attachments = {(*ImageManager)(ColorImage).View, (*ImageManager)(NormalsImage).View, (*ImageManager)(RenderableIndexImage).View,
                                                (*ImageManager)(DepthImage).View, (*ImageManager)(ResolvedColorImage).View};

        VkFramebufferCreateInfo FramebufferCreateInfo{};
        FramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        FramebufferCreateInfo.renderPass = RenderPass->RenderPass;
        FramebufferCreateInfo.attachmentCount = static_cast<uint32_t>(Attachments.size());
        FramebufferCreateInfo.pAttachments = Attachments.data();
        FramebufferCreateInfo.width = Swapchain->GetWidth();
        FramebufferCreateInfo.height = Swapchain->GetHeight();
        FramebufferCreateInfo.layers = 1;

        if (vkCreateFramebuffer(LogicalDevice, &FramebufferCreateInfo, nullptr, &SwapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create framebuffer!");
        }

        V::SetName(LogicalDevice, SwapChainFramebuffers[i], "V_Render_fb_" + std::to_string(i));
    }
}

void FVulkanContext::CreatePassthroughFramebuffers()
{
    PassthroughFramebuffers.resize(Swapchain->Size());
    for (std::size_t i = 0; i < PassthroughFramebuffers.size(); ++i)
    {
        std::vector<VkImageView> Attachments = {Swapchain->Images[i].View};

        VkFramebufferCreateInfo FramebufferCreateInfo{};
        FramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        FramebufferCreateInfo.renderPass = PassthroughRenderPass->RenderPass;
        FramebufferCreateInfo.attachmentCount = static_cast<uint32_t>(Attachments.size());
        FramebufferCreateInfo.pAttachments = Attachments.data();
        FramebufferCreateInfo.width = Swapchain->GetWidth();
        FramebufferCreateInfo.height = Swapchain->GetHeight();
        FramebufferCreateInfo.layers = 1;

        if (vkCreateFramebuffer(LogicalDevice, &FramebufferCreateInfo, nullptr, &PassthroughFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create passthrough framebuffer!");
        }

        V::SetName(LogicalDevice, PassthroughFramebuffers[i], "V_Passthrough_fb_" + std::to_string(i));
    }
}

void FVulkanContext::CreateImguiFramebuffers()
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

        V::SetName(LogicalDevice, ImGuiFramebuffers[i], "V_Imgui_fb_" + std::to_string(i));

    }
}

void FVulkanContext::LoadModelDataToGPU()
{
    auto MeshSystem = ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMeshSystem>();

    for(auto Mesh : *MeshSystem)
    {
        MeshSystem->LoadToGPU(Mesh);
    }

}

void FVulkanContext::CreateTextureSampler()
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

void FVulkanContext::CreateUniformBuffers()
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

void FVulkanContext::CreateDescriptorPool()
{
    auto ModelsCount = ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMeshSystem>()->Size();
    auto NumberOfSwapChainImages = Swapchain->Size();

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->AddDescriptorSet(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME, NumberOfSwapChainImages);
    DescriptorSetManager->AddDescriptorSet(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME, NumberOfSwapChainImages * ModelsCount);
    DescriptorSetManager->AddDescriptorSet(LAYOUT_SETS::PASSTHROUGH_LAYOUT_NAME, NumberOfSwapChainImages);

    DescriptorSetManager->ReserveDescriptorPool();
}

void FVulkanContext::CreateImguiDescriptorPool()
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

void FVulkanContext::CreateImguiRenderpasss()
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

void FVulkanContext::CreateDescriptorSet()
{
    auto& Coordinator = ECS::GetCoordinator();
    auto MeshSystem = Coordinator.GetSystem<ECS::SYSTEMS::FMeshSystem>();

    /// Create descriptor sets
    DescriptorSetManager->CreateAllDescriptorSets();

    for (size_t i = 0; i < Swapchain->Size(); ++i)
    {
        uint32_t j = 0;
        for (auto Mesh : *MeshSystem)
        {
            VkDescriptorBufferInfo TransformBufferInfo{};
            TransformBufferInfo.buffer = DeviceTransformBuffers[i].Buffer;
            TransformBufferInfo.offset = sizeof(ECS::COMPONENTS::FDeviceTransformComponent) * j;
            TransformBufferInfo.range = sizeof(ECS::COMPONENTS::FDeviceTransformComponent);
            DescriptorSetManager->UpdateDescriptorSetInfo(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME, LAYOUTS::TRANSFORM_LAYOUT_NAME, j * Swapchain->Size() + i, TransformBufferInfo);

            VkDescriptorBufferInfo RenderableBufferInfo{};
            RenderableBufferInfo.buffer = DeviceRenderableBuffers[i].Buffer;
            RenderableBufferInfo.offset = sizeof(ECS::COMPONENTS::FDeviceRenderableComponent) * j;
            RenderableBufferInfo.range = sizeof(ECS::COMPONENTS::FDeviceRenderableComponent);
            DescriptorSetManager->UpdateDescriptorSetInfo(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME, LAYOUTS::RENDERABLE_LAYOUT_NAME, j * Swapchain->Size() + i, RenderableBufferInfo);

            ++j;
        }
    }

    for (size_t i = 0; i < Swapchain->Size(); ++i)
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

    for (size_t i = 0; i < Swapchain->Size(); ++i)
    {
        VkDescriptorImageInfo ImageBufferInfo{};
        ImageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ImageBufferInfo.imageView = (*ImageManager)(ResolvedColorImage).View;
        ImageBufferInfo.sampler = TextureSampler;
        DescriptorSetManager->UpdateDescriptorSetInfo(LAYOUT_SETS::PASSTHROUGH_LAYOUT_NAME, LAYOUTS::TEXTURE_SAMPLER_LAYOUT_NAME, i, ImageBufferInfo);
    }
}

void FVulkanContext::CreateCommandBuffers()
{
    GraphicsCommandBuffers.resize(SwapChainFramebuffers.size());

    for (std::size_t i = 0; i < GraphicsCommandBuffers.size(); ++i)
    {
        GraphicsCommandBuffers[i] = CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
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
            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline.GetPipeline());

            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline.GetPipelineLayout(), 0, 1, &DescriptorSetManager->GetSet(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME, i), 0,
                                    nullptr);
            auto& Coordinator = ECS::GetCoordinator();
            auto MeshSystem = Coordinator.GetSystem<ECS::SYSTEMS::FMeshSystem>();

            uint32_t j = 0;
            for (auto Entity : *MeshSystem)
            {
                vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline.GetPipelineLayout(), 1, 1,
                                        &DescriptorSetManager->GetSet(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME, j * Swapchain->Size() + i), 0,
                                        nullptr);

                MeshSystem->Bind(Entity, CommandBuffer);
                MeshSystem->Draw(Entity, CommandBuffer);
                ++j;
            }
            vkCmdEndRenderPass(CommandBuffer);
        });
    }

    PassthroughCommandBuffers.resize(PassthroughFramebuffers.size());

    for (std::size_t i = 0; i < PassthroughCommandBuffers.size(); ++i)
    {
        PassthroughCommandBuffers[i] = CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            VkImageMemoryBarrier Barrier{};
            Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            Barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            Barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            Barrier.image = (*ImageManager)(ResolvedColorImage).Image;
            Barrier.subresourceRange.baseMipLevel = 0;
            Barrier.subresourceRange.levelCount = 1;
            Barrier.subresourceRange.baseArrayLayer = 0;
            Barrier.subresourceRange.layerCount = 1;
            Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            Barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &Barrier);

            VkRenderPassBeginInfo RenderPassInfo{};
            RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            RenderPassInfo.renderPass = PassthroughRenderPass->RenderPass;
            RenderPassInfo.framebuffer = PassthroughFramebuffers[i];
            RenderPassInfo.renderArea.offset = {0, 0};
            RenderPassInfo.renderArea.extent = Swapchain->GetExtent2D();

            std::vector<VkClearValue> ClearValues{1};
            ClearValues[0].color = {0.f, 0.f, 1.f, 1.f};
            RenderPassInfo.clearValueCount = static_cast<uint32_t>(ClearValues.size());
            RenderPassInfo.pClearValues = ClearValues.data();

            vkCmdBeginRenderPass(CommandBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PassthroughPipeline.GetPipeline());
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PassthroughPipeline.GetPipelineLayout(),
                                    0, 1, &DescriptorSetManager->GetSet(LAYOUT_SETS::PASSTHROUGH_LAYOUT_NAME, i),
                                    0, nullptr);

            vkCmdDraw(CommandBuffer, 3, 1, 0, 0);
            vkCmdEndRenderPass(CommandBuffer);
        });
    }
}

void FVulkanContext::CreateSyncObjects()
{
    ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    PassthroughFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
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
            vkCreateSemaphore(LogicalDevice, &SemaphoreInfo, nullptr, &PassthroughFinishedSemaphore[i]) != VK_SUCCESS ||
            vkCreateSemaphore(LogicalDevice, &SemaphoreInfo, nullptr, &ImGuiFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(LogicalDevice, &FenceInfo, nullptr, &ImGuiFinishedFences[i]) != VK_SUCCESS ||
            vkCreateFence(LogicalDevice, &FenceInfo, nullptr, &RenderingFinishedFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create synchronization objects for a frame!");
        }
    }
}

void FVulkanContext::CreateImguiContext()
{
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

void FVulkanContext::Render()
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
    SubmitInfo.pCommandBuffers = &GraphicsCommandBuffers[ImageIndex];

    VkSemaphore SignalSemaphores[] = {RenderFinishedSemaphores[CurrentFrame]};
    SubmitInfo.signalSemaphoreCount = 1;
    SubmitInfo.pSignalSemaphores = SignalSemaphores;

    /// Reset frame state to unsignaled, just before rendering
    vkResetFences(LogicalDevice, 1, &RenderingFinishedFences[CurrentFrame]);

    /// Submit rendering. When rendering finished, appropriate fence will be signalled
    if (vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }


    VkSubmitInfo PassThroughSubmitInfo{};
    PassThroughSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore PassthroughWaitSemaphores[] = {RenderFinishedSemaphores[CurrentFrame]};
    VkPipelineStageFlags PassthroughWaitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    PassThroughSubmitInfo.waitSemaphoreCount = 1;
    PassThroughSubmitInfo.pWaitSemaphores = PassthroughWaitSemaphores;
    PassThroughSubmitInfo.pWaitDstStageMask = PassthroughWaitStages;
    PassThroughSubmitInfo.commandBufferCount = 1;
    PassThroughSubmitInfo.pCommandBuffers = &PassthroughCommandBuffers[ImageIndex];

    VkSemaphore PassthroughSignalSemaphores[] = {PassthroughFinishedSemaphore[CurrentFrame]};
    PassThroughSubmitInfo.signalSemaphoreCount = 1;
    PassThroughSubmitInfo.pSignalSemaphores = PassthroughSignalSemaphores;

    /// Submit rendering. When rendering finished, appropriate fence will be signalled
    if (vkQueueSubmit(GraphicsQueue, 1, &PassThroughSubmitInfo, RenderingFinishedFences[CurrentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }
}

void FVulkanContext::RenderImGui()
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

        VkSemaphore WaitSemaphores[] = {PassthroughFinishedSemaphore[CurrentFrame]};
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

void FVulkanContext::Present()
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

void FVulkanContext::WaitIdle()
{
    vkDeviceWaitIdle(LogicalDevice);
}

void FVulkanContext::RecreateSwapChain()
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
    CreateRenderFramebuffers();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSet();
    CreateCommandBuffers();
}

void FVulkanContext::CleanUpSwapChain()
{
    ImageManager->RemoveImage(ColorImage);
    ImageManager->RemoveImage(ResolvedColorImage);
    ImageManager->RemoveImage(UtilityImageR8G8B8A8_SRGB);
    ImageManager->RemoveImage(NormalsImage);
    ImageManager->RemoveImage(RenderableIndexImage);
    ImageManager->RemoveImage(UtilityImageR32);
    ImageManager->RemoveImage(DepthImage);

    for (auto Framebuffer : SwapChainFramebuffers)
    {
        vkDestroyFramebuffer(LogicalDevice, Framebuffer, nullptr);
    }

    for (auto Framebuffer : PassthroughFramebuffers)
    {
        vkDestroyFramebuffer(LogicalDevice, Framebuffer, nullptr);
    }

    for (auto Framebuffer : ImGuiFramebuffers)
    {
        vkDestroyFramebuffer(LogicalDevice, Framebuffer, nullptr);
    }

    for (auto& CommandBuffer : GraphicsCommandBuffers)
    {
        CommandBufferManager->FreeCommandBuffer(CommandBuffer);
    }

    for (auto& CommandBuffer : PassthroughCommandBuffers)
    {
        CommandBufferManager->FreeCommandBuffer(CommandBuffer);
    }

    GraphicsPipeline.Delete();
    PassthroughPipeline.Delete();

    RenderPass = nullptr;
    PassthroughRenderPass = nullptr;
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

void FVulkanContext::UpdateUniformBuffer(uint32_t CurrentImage)
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

void FVulkanContext::LoadDataIntoBuffer(FBuffer &Buffer, void* DataToLoad, size_t Size)
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

void FVulkanContext::FreeData(FBuffer Buffer)
{
    ResourceAllocator->DestroyBuffer(Buffer);
}

void FVulkanContext::DestroyDebugUtilsMessengerEXT()
{
#ifndef NDEBUG
    V::vkDestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
#endif
}

void FVulkanContext::CleanUp()
{
    CleanUpSwapChain();

    vkDestroySampler(LogicalDevice, TextureSampler, nullptr);
    ImageManager->RemoveImage(TextureImage);

    DescriptorSetManager->DestroyDescriptorSetLayout(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME);
    DescriptorSetManager->DestroyDescriptorSetLayout(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME);
    DescriptorSetManager->DestroyDescriptorSetLayout(LAYOUT_SETS::PASSTHROUGH_LAYOUT_NAME);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    auto& Coordinator = ECS::GetCoordinator();
    Coordinator.GetSystem<ECS::SYSTEMS::FMeshSystem>()->FreeAllDeviceData();

    for(std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(LogicalDevice, RenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(LogicalDevice, ImageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(LogicalDevice, PassthroughFinishedSemaphore[i], nullptr);
        vkDestroySemaphore(LogicalDevice, ImGuiFinishedSemaphores[i], nullptr);
        vkDestroyFence(LogicalDevice, RenderingFinishedFences[i], nullptr);
        vkDestroyFence(LogicalDevice, ImGuiFinishedFences[i], nullptr);
    }

    CommandBufferManager = nullptr;
    vkDestroyDevice(LogicalDevice, nullptr);

    if (!ValidationLayers.empty())
    {
        DestroyDebugUtilsMessengerEXT();
    }

    vkDestroySurfaceKHR(Instance, Surface, nullptr);
    vkDestroyInstance(Instance, nullptr);
}

FVulkanContext::~FVulkanContext()
{
}
