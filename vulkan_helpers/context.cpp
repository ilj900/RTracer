#include "context.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <set>
#include <array>
#include <unordered_map>
#include <chrono>

using namespace V;

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT MessageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallBackData,
        void* pUserData)
{
    std::cerr << "Validation layer: " << pCallBackData->pMessage << std::endl;

    return VK_FALSE;
}

size_t std::hash<FVertex>::operator()(FVertex const& Vertex) const
{
    return ((std::hash<FVector3>{}(Vertex.Pos) ^
             (std::hash<FVector3>{}(Vertex.Color) << 1)) >> 1) ^
           (std::hash<FVector2>{}(Vertex.TexCoord) << 1);
}

FContext::FContext(GLFWwindow *Window) :
    Window(Window)
{
}

void FContext::Init()
{
    try {
        CreateInstance();
        SetupDebugMessenger();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDevice();
        CreateSwapChain();
        CreateImageViews();
        CreateRenderPass();
        CreateDescriptorSetLayout();
        CreateGraphicsPipeline();
        CreateCommandPool();
        CreateColorResources();
        CreateDepthResources();
        CreateFramebuffers();
        CreateTextureImage(TexturePath);
        CreateTextureImageView();
        CreateTextureSampler();
        LoadModel(ModelPath);
        CreateVertexBuffer();
        CreateIndexBuffer();
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
    // Check supported Layers
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

    auto Function = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT");
    if (Function != nullptr)
    {
        Function(Instance, &DebugCreateInfo, nullptr, &DebugMessenger);
    }
    else
    {
        throw std::runtime_error("Failed to set up debug messenger!");
    }
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
}

void FContext::CreateSwapChain()
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &Capabilities);

    uint32_t FormatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &FormatCount, nullptr);
    if (FormatCount != 0)
    {
        Formats.resize(FormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &FormatCount, Formats.data());
    }

    uint32_t PresentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, nullptr);

    if (PresentModeCount != 0)
    {
        PresentModes.resize(PresentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, PresentModes.data());
    }

    VkPresentModeKHR PresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& AvailablePresentMode : PresentModes)
    {
        if (AvailablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            PresentMode = AvailablePresentMode;
        }
    }

    VkSurfaceFormatKHR SurfaceFormat = Formats[0];
    for (const auto& Format : Formats)
    {
        if (Format.format == VK_FORMAT_B8G8R8A8_SRGB && Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            SurfaceFormat = Format;
        }
    }

    VkExtent2D Extent;

    if (Capabilities.currentExtent.width != UINT32_MAX)
    {
        Extent = Capabilities.currentExtent;
    }
    else
    {
        int Width, Height;

        glfwGetFramebufferSize(Window, &Width, &Height);

        VkExtent2D ActualExtent = {static_cast<uint32_t>(Width), static_cast<uint32_t>(Height)};
        ActualExtent.width = std::max(Capabilities.minImageExtent.width, std::min(Capabilities.minImageExtent.width, ActualExtent.width));
        ActualExtent.height = std::max(Capabilities.minImageExtent.height, std::min(Capabilities.minImageExtent.height, ActualExtent.height));
        Extent = ActualExtent;
    }

    uint32_t ImageCount = Capabilities.minImageCount + 1;

    if (Capabilities.maxImageCount > 0 && ImageCount > Capabilities.maxImageCount)
    {
        ImageCount = Capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    CreateInfo.surface = Surface;

    CreateInfo.minImageCount = ImageCount;
    CreateInfo.imageFormat = SurfaceFormat.format;
    CreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
    CreateInfo.imageExtent = Extent;
    CreateInfo.imageArrayLayers = 1;
    CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t QueueFamilyIndices[] = {GraphicsQueueIndex, PresentQueueIndex};
    if (GraphicsQueueIndex != PresentQueueIndex)
    {
        CreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        CreateInfo.queueFamilyIndexCount = 2;
        CreateInfo.pQueueFamilyIndices = QueueFamilyIndices;
    }
    else
    {
        CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        CreateInfo.queueFamilyIndexCount = 0;
        CreateInfo.pQueueFamilyIndices = nullptr;
    }

    CreateInfo.preTransform = Capabilities.currentTransform;
    CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    CreateInfo.presentMode = PresentMode;
    CreateInfo.clipped = VK_TRUE;

    CreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(LogicalDevice, &CreateInfo, nullptr, &SwapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(LogicalDevice, SwapChain, &ImageCount, nullptr);
    SwapChainImages.resize(ImageCount);
    vkGetSwapchainImagesKHR(LogicalDevice, SwapChain, &ImageCount, SwapChainImages.data());
    SwapChainImageFormat = SurfaceFormat.format;
    SwapChainExtent = Extent;
}

void FContext::CreateImageViews()
{
    SwapChainImageViews.resize(SwapChainImages.size());

    for (std::size_t i = 0; i < SwapChainImages.size(); ++i)
    {
        SwapChainImageViews[i] = CreateImageView(SwapChainImages[i], SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
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
    ColorAttachment.format = SwapChainImageFormat;
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
    ColorAttachmentResolve.format = SwapChainImageFormat;
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

void FContext::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding UboLayoutBinding{};
    UboLayoutBinding.binding = 0;
    UboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    UboLayoutBinding.descriptorCount = 1;
    UboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    UboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding SamplerLayoutBinding{};
    SamplerLayoutBinding.binding = 1;
    SamplerLayoutBinding.descriptorCount = 1;
    SamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    SamplerLayoutBinding.pImmutableSamplers = nullptr;
    SamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> Bindings {UboLayoutBinding, SamplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo LayoutInfo{};
    LayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    LayoutInfo.bindingCount = static_cast<uint32_t>(Bindings.size());
    LayoutInfo.pBindings = Bindings.data();

    if (vkCreateDescriptorSetLayout(LogicalDevice, &LayoutInfo, nullptr, &DescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
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
    Viewport.width = (float)SwapChainExtent.width;
    Viewport.height = (float)SwapChainExtent.height;
    Viewport.minDepth = 0.f;
    Viewport.maxDepth = 1.f;

    VkRect2D Scissors{};
    Scissors.offset = {0, 0};
    Scissors.extent = SwapChainExtent;

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
    PipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutInfo.setLayoutCount = 1;
    PipelineLayoutInfo.pSetLayouts = &DescriptorSetLayout;
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

void FContext::CreateColorResources()
{
    VkFormat ColorFormat = SwapChainImageFormat;

    CreateImage(SwapChainExtent.width, SwapChainExtent.height, 1, MSAASamples, ColorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ColorImage, ColorImageMemory);

    ColorImageView = CreateImageView(ColorImage, ColorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
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
    AllocInfo.memoryTypeIndex = FindMemoryType(MemRequirements.memoryTypeBits, Properties);

    if (vkAllocateMemory(LogicalDevice, &AllocInfo, nullptr, &ImageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate image memory!");
    }

    vkBindImageMemory(LogicalDevice, Image, ImageMemory, 0);
}

uint32_t FContext::FindMemoryType(uint32_t TypeFilter, VkMemoryPropertyFlags Properties)
{
    VkPhysicalDeviceMemoryProperties MemProperties;
    vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &MemProperties);

    for (uint32_t i = 0; i < MemProperties.memoryTypeCount; ++i)
    {
        if (TypeFilter & (1 << i) && (MemProperties.memoryTypes[i].propertyFlags & Properties) == Properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

void FContext::CreateDepthResources()
{
    VkFormat DepthFormat = FindDepthFormat();

    CreateImage(SwapChainExtent.width, SwapChainExtent.height, 1, MSAASamples, DepthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, DepthImage, DepthImageMemory);
    DepthImageView = CreateImageView(DepthImage, DepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    TransitionImageLayout(DepthImage, DepthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
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
    SwapChainFramebuffers.resize(SwapChainImageViews.size());
    for (std::size_t i = 0; i < SwapChainImageViews.size(); ++i) {
        std::array<VkImageView, 3> Attachments = {ColorImageView, DepthImageView, SwapChainImageViews[i]};

        VkFramebufferCreateInfo FramebufferInfo{};
        FramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        FramebufferInfo.renderPass = RenderPass;
        FramebufferInfo.attachmentCount = static_cast<uint32_t>(Attachments.size());
        FramebufferInfo.pAttachments = Attachments.data();
        FramebufferInfo.width = SwapChainExtent.width;
        FramebufferInfo.height = SwapChainExtent.height;
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

    CreateBuffer(ImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingBuffer, StagingBufferMemory);

    void *Data;
    vkMapMemory(LogicalDevice, StagingBufferMemory, 0, ImageSize, 0, &Data);
    memcpy(Data, Pixels, static_cast<size_t>(ImageSize));
    vkUnmapMemory(LogicalDevice, StagingBufferMemory);
    stbi_image_free(Pixels);

    CreateImage(TexWidth, TexHeight, MipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, TextureImage, TextureImageMemory);

    TransitionImageLayout(TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, MipLevels);
    CopyBufferToImage(StagingBuffer, TextureImage, static_cast<uint32_t>(TexWidth), static_cast<uint32_t>(TexHeight));

    GenerateMipmaps(TextureImage, VK_FORMAT_R8G8B8A8_SRGB, TexWidth, TexHeight, MipLevels);
    vkDestroyBuffer(LogicalDevice, StagingBuffer, nullptr);
    vkFreeMemory(LogicalDevice, StagingBufferMemory, nullptr);
}

void FContext::CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties, VkBuffer& Buffer, VkDeviceMemory& BufferMemory)
{
    VkBufferCreateInfo BufferInfo{};
    BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferInfo.size = Size;
    BufferInfo.usage = Usage;
    BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(LogicalDevice, &BufferInfo, nullptr, &Buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create buffer!");
    }

    VkMemoryRequirements MemRequirements;
    vkGetBufferMemoryRequirements(LogicalDevice, Buffer, &MemRequirements);

    VkMemoryAllocateInfo  AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    AllocInfo.allocationSize = MemRequirements.size;
    AllocInfo.memoryTypeIndex = FindMemoryType(MemRequirements.memoryTypeBits, Properties);

    if (vkAllocateMemory(LogicalDevice, &AllocInfo, nullptr, &BufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate buffer memory!");
    }

    vkBindBufferMemory(LogicalDevice, Buffer, BufferMemory, 0);
}

void FContext::CopyBufferToImage(VkBuffer Buffer, VkImage Image, uint32_t Width, uint32_t Height)
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

    vkCmdCopyBufferToImage(CommandBuffer, Buffer, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region);

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

void FContext::LoadModel(const std::string& ModelPath)
{
    tinyobj::attrib_t Attrib;
    std::vector<tinyobj::shape_t> Shapes;
    std::vector<tinyobj::material_t> Materials;
    std::string Warn, Err;

    if (!tinyobj::LoadObj(&Attrib, &Shapes, &Materials, &Warn, &Err, ModelPath.c_str()))
    {
        throw std::runtime_error(Warn + Err);
    }

    std::unordered_map<FVertex, uint32_t> UniqueVertices{};


    for (const auto& Shape : Shapes)
    {
        for (const auto& Index : Shape.mesh.indices)
        {
            FVertex Vert{};

            Vert.Pos = {
                    Attrib.vertices[3 * Index.vertex_index + 0],
                    Attrib.vertices[3 * Index.vertex_index + 1],
                    Attrib.vertices[3 * Index.vertex_index + 2]
            };

            Vert.TexCoord = {
                    Attrib.texcoords[2 * Index.texcoord_index + 0],
                    1.f - Attrib.texcoords[2 * Index.texcoord_index + 1],
            };

            Vert.Color = {1.f, 1.f, 1.f};

            if (UniqueVertices.find(Vert) == UniqueVertices.end())
            {
                UniqueVertices[Vert] = static_cast<uint32_t>(Vertices.size());
                Vertices.push_back(Vert);
            }

            Indices.push_back(UniqueVertices[Vert]);
        }
    }
}

void FContext::CreateVertexBuffer()
{
    VkDeviceSize BufferSize = sizeof(Vertices[0]) * Vertices.size();

    VkBuffer StagingBuffer;
    VkDeviceMemory StagingBufferMemory;
    CreateBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingBuffer, StagingBufferMemory);

    void* Data;
    vkMapMemory(LogicalDevice, StagingBufferMemory, 0, BufferSize, 0, &Data);
    memcpy(Data, Vertices.data(), (std::size_t)BufferSize);
    vkUnmapMemory(LogicalDevice, StagingBufferMemory);

    CreateBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VertexBuffer, VertexBufferMemory);

    CopyBuffer(StagingBuffer, VertexBuffer, BufferSize);

    vkDestroyBuffer(LogicalDevice, StagingBuffer, nullptr);
    vkFreeMemory(LogicalDevice, StagingBufferMemory, nullptr);
}

void FContext::CopyBuffer(VkBuffer SrcBuffer, VkBuffer DstBuffer, VkDeviceSize Size)
{
    VkCommandBuffer CommandBuffer = BeginSingleTimeCommands();

    VkBufferCopy CopyRegion{};
    CopyRegion.size = Size;
    vkCmdCopyBuffer(CommandBuffer, SrcBuffer, DstBuffer, 1, &CopyRegion);

    EndSingleTimeCommand(CommandBuffer);
}

void FContext::CreateIndexBuffer()
{
    VkDeviceSize BufferSize = sizeof(Indices[0]) * Indices.size();

    VkBuffer StagingBuffer;
    VkDeviceMemory StagingBufferMemory;
    CreateBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingBuffer, StagingBufferMemory);

    void* Data;
    vkMapMemory(LogicalDevice, StagingBufferMemory, 0, BufferSize, 0, &Data);
    memcpy(Data, Indices.data(), (std::size_t)BufferSize);
    vkUnmapMemory(LogicalDevice, StagingBufferMemory);

    CreateBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, IndexBuffer, IndexBufferMemory);

    CopyBuffer(StagingBuffer, IndexBuffer, BufferSize);
    vkDestroyBuffer(LogicalDevice, StagingBuffer, nullptr);
    vkFreeMemory(LogicalDevice, StagingBufferMemory, nullptr);
}

void FContext::CreateUniformBuffers()
{
    VkDeviceSize BufferSize = sizeof(UniformBufferObject);

    UniformBuffers.resize(SwapChainImages.size());
    UniformBuffersMemory.resize(SwapChainImages.size());

    for (size_t i = 0; i < SwapChainImages.size(); ++i)
    {
        CreateBuffer(BufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, UniformBuffers[i], UniformBuffersMemory[i]);
    }
}

void FContext::CreateDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> PoolSizes{};
    PoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    PoolSizes[0].descriptorCount = static_cast<uint32_t>(SwapChainImages.size());
    PoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    PoolSizes[1].descriptorCount = static_cast<uint32_t>(SwapChainImages.size());

    VkDescriptorPoolCreateInfo PoolInfo{};
    PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    PoolInfo.poolSizeCount = static_cast<uint32_t>(PoolSizes.size());
    PoolInfo.pPoolSizes = PoolSizes.data();
    PoolInfo.maxSets = static_cast<uint32_t>(SwapChainImages.size());

    if (vkCreateDescriptorPool(LogicalDevice, &PoolInfo, nullptr, &DescriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

void FContext::CreateDescriptorSet()
{
    std::vector<VkDescriptorSetLayout> Layouts(SwapChainImages.size(), DescriptorSetLayout);
    VkDescriptorSetAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    AllocInfo.descriptorPool = DescriptorPool;
    AllocInfo.descriptorSetCount = static_cast<uint32_t>(SwapChainImages.size());
    AllocInfo.pSetLayouts = Layouts.data();

    DescriptorSets.resize(SwapChainImages.size());
    if (vkAllocateDescriptorSets(LogicalDevice, &AllocInfo, DescriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < SwapChainImages.size(); ++i)
    {
        VkDescriptorBufferInfo BufferInfo{};
        BufferInfo.buffer = UniformBuffers[i];
        BufferInfo.offset = 0;
        BufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo ImageInfo{};
        ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ImageInfo.imageView = TextureImageView;
        ImageInfo.sampler = TextureSampler;

        std::array<VkWriteDescriptorSet, 2> DescriptorWrites{};
        DescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        DescriptorWrites[0].dstSet = DescriptorSets[i];
        DescriptorWrites[0].dstBinding = 0;
        DescriptorWrites[0].dstArrayElement = 0;
        DescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        DescriptorWrites[0].descriptorCount = 1;
        DescriptorWrites[0].pBufferInfo = &BufferInfo;

        DescriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        DescriptorWrites[1].dstSet = DescriptorSets[i];
        DescriptorWrites[1].dstBinding = 1;
        DescriptorWrites[1].dstArrayElement = 0;
        DescriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        DescriptorWrites[1].descriptorCount = 1;
        DescriptorWrites[1].pImageInfo = &ImageInfo;

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
        RenderPassInfo.renderArea.extent = SwapChainExtent;

        std::array<VkClearValue, 2> ClearValues{};
        ClearValues[0].color = {0.f, 0.f, 0.f, 1.f};
        ClearValues[1].depthStencil = {1.f, 0};
        RenderPassInfo.clearValueCount = static_cast<uint32_t>(ClearValues.size());
        RenderPassInfo.pClearValues = ClearValues.data();

        vkCmdBeginRenderPass(CommandBuffers[i], &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline);
        VkBuffer VertexBuffers[] = {VertexBuffer};
        VkDeviceSize Offsets[] = {0};
        vkCmdBindVertexBuffers(CommandBuffers[i], 0, 1, VertexBuffers, Offsets);
        vkCmdBindIndexBuffer(CommandBuffers[i], IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout, 0, 1, &DescriptorSets[i], 0,
                                nullptr);
        vkCmdDrawIndexed(CommandBuffers[i], static_cast<uint32_t>(Indices.size()), 1, 0, 0, 0);
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
    InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
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
            vkCreateFence(LogicalDevice, &FenceInfo, nullptr, &InFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create synchronization objects for a frame!");
        }
    }
}

void FContext::DrawFrame()
{
    vkWaitForFences(LogicalDevice, 1, &InFlightFences[CurrentFrame], VK_TRUE, UINT64_MAX);
    uint32_t ImageIndex;
    VkResult Result = vkAcquireNextImageKHR(LogicalDevice, SwapChain, UINT64_MAX, ImageAvailableSemaphores[CurrentFrame], VK_NULL_HANDLE, &ImageIndex);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain();
        return;
    }
    else if (Result != VK_SUCCESS && Result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    if(ImagesInFlight[ImageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(LogicalDevice, 1, &ImagesInFlight[ImageIndex], VK_TRUE, UINT64_MAX);
    }

    ImagesInFlight[ImageIndex] = InFlightFences[CurrentFrame];

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

    vkResetFences(LogicalDevice, 1, &InFlightFences[CurrentFrame]);

    if (vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, InFlightFences[CurrentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR PresentInfo{};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pWaitSemaphores = SignalSemaphores;
    VkSwapchainKHR SwapChains[] = {SwapChain};
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = SwapChains;
    PresentInfo.pImageIndices = &ImageIndex;
    PresentInfo.pResults = nullptr;

    Result = vkQueuePresentKHR(PresentQueue, &PresentInfo);

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

    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateColorResources();
    CreateDepthResources();
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

    for (auto ImageView : SwapChainImageViews)
    {
        vkDestroyImageView(LogicalDevice, ImageView, nullptr);
    }

    vkDestroySwapchainKHR(LogicalDevice, SwapChain, nullptr);

    for (size_t i = 0; i < SwapChainImages.size(); ++i)
    {
        vkDestroyBuffer(LogicalDevice, UniformBuffers[i], nullptr);
        vkFreeMemory(LogicalDevice, UniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(LogicalDevice, DescriptorPool, nullptr);
}

void FContext::UpdateUniformBuffer(uint32_t CurrentImage)
{
    static auto StartTime = std::chrono::high_resolution_clock::now();

    auto CurrentTime = std::chrono::high_resolution_clock::now();
    float Time = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - StartTime).count();

    UniformBufferObject UBO{};
    UBO.Model = Rotate(Time * 1.f, FVector3(0.f, 0.f, 1.f));
    UBO.View = LookAt(FVector3(2.f, 2.f, 2.f), FVector3(0.f, 0.f, 0.f), FVector3(0.f, 0.f, 1.f));
    UBO.Projection = GetPerspective(0.785398f, SwapChainExtent.width / (float) SwapChainExtent.height, 0.1f, 10.f);

    void* Data;
    vkMapMemory(LogicalDevice, UniformBuffersMemory[CurrentImage], 0, sizeof(UBO), 0, &Data);
    memcpy(Data, &UBO, sizeof(UBO));
    vkUnmapMemory(LogicalDevice, UniformBuffersMemory[CurrentImage]);
}

void FContext::DestroyDebugUtilsMessengerEXT()
{
    auto Function = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT");
    if (Function != nullptr)
    {
        Function(Instance, DebugMessenger, nullptr);
    }
}

void FContext::CleanUp()
{
    CleanUpSwapChain();

    vkDestroySampler(LogicalDevice, TextureSampler, nullptr);
    vkDestroyImageView(LogicalDevice, TextureImageView, nullptr);

    vkDestroyImage(LogicalDevice, TextureImage, nullptr);
    vkFreeMemory(LogicalDevice, TextureImageMemory, nullptr);

    vkDestroyDescriptorSetLayout(LogicalDevice, DescriptorSetLayout, nullptr);

    vkDestroyBuffer(LogicalDevice, IndexBuffer, nullptr);
    vkFreeMemory(LogicalDevice, IndexBufferMemory, nullptr);

    vkDestroyBuffer(LogicalDevice, VertexBuffer, nullptr);
    vkFreeMemory(LogicalDevice, VertexBufferMemory, nullptr);

    for(std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(LogicalDevice, RenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(LogicalDevice, ImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(LogicalDevice, InFlightFences[i], nullptr);
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
