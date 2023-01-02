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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static FVulkanContext Context{};

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

    try {
        FillInContextOptions();
        CreateInstance();
        LoadFunctionPointers();
        SetupDebugMessenger();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDevice(PhysicalDevice);
        GetDeviceQueues();
        CommandBufferManager = std::make_shared<FCommandBufferManager>(LogicalDevice, this, GetQueue(VK_QUEUE_GRAPHICS_BIT),
                                                                       GetQueueIndex(VK_QUEUE_GRAPHICS_BIT));
        Swapchain = std::make_shared<FSwapchain>(*this, PhysicalDevice, LogicalDevice, Surface, Window, GetGraphicsQueueIndex(), GetPresentIndex(), VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_PRESENT_MODE_MAILBOX_KHR);
        CreateDepthAndAAImages();
        CreateRenderPass();
        CreatePassthroughRenderPass();
        CreateImguiRenderpasss();
        CreateDescriptorSetLayouts();
        CreateGraphicsPipeline();
        CreatePassthroughPipeline();
        TextureImage = LoadImageFromFile(TexturePath, "V_TextureImage");
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

void FVulkanContext::FillInContextOptions()
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

    VulkanContextOptions.AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

void FVulkanContext::CreateInstance()
{
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

std::vector<VkPhysicalDevice> FVulkanContext::EnumerateAllPhysicalDevices(VkInstance Instance)
{
    uint32_t DeviceCount = 0;
    vkEnumeratePhysicalDevices(Instance, &DeviceCount, nullptr);

    if (DeviceCount == 0)
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> Devices(DeviceCount);
    vkEnumeratePhysicalDevices(Instance, &DeviceCount, Devices.data());

    return Devices;
}

void FVulkanContext::PickPhysicalDevice()
{
    auto Devices = EnumerateAllPhysicalDevices(Instance);

    auto DeviceExtensionList = VulkanContextOptions.GetDeviceExtensionsList();

    std::set<std::string> RequiredExtensions (DeviceExtensionList.begin(), DeviceExtensionList.end());

    for (const auto& Device : Devices)
    {
        if (CheckDeviceExtensionsSupport(Device, RequiredExtensions) && (CheckDeviceQueueSupport(Device)))
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

bool FVulkanContext::CheckDeviceExtensionsSupport(VkPhysicalDevice Device, std::set<std::string>& RequiredExtensions)
{
    uint32_t ExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(Device, nullptr, &ExtensionCount, nullptr);

    std::vector<VkExtensionProperties>AvailableExtensions(ExtensionCount);
    vkEnumerateDeviceExtensionProperties(Device, nullptr, &ExtensionCount, AvailableExtensions.data());

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

std::vector<VkQueueFamilyProperties> FVulkanContext::EnumeratePhysicalDeviceQueueFamilyProperties(VkPhysicalDevice Device)
{
    uint32_t QueueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> QueueFamilyProperties(QueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, QueueFamilyProperties.data());

    return QueueFamilyProperties;
}

bool FVulkanContext::CheckDeviceQueueSupport(VkPhysicalDevice PhysicalDevice, VkQueueFlagBits QueueFlagBits, uint32_t& QueueFamilyIndex)
{
    auto Properties = EnumeratePhysicalDeviceQueueFamilyProperties(PhysicalDevice);

    for (uint32_t i = 0; i < Properties.size(); ++i)
    {
        if ((Properties[i].queueFlags & QueueFlagBits) == QueueFlagBits)
        {
            QueueFamilyIndex = i;
            return true;
        }
    }

    return false;
}

bool FVulkanContext::CheckDeviceQueuePresentSupport(VkPhysicalDevice PhysicalDevice, uint32_t& QueueFamilyIndex)
{
    auto Properties = EnumeratePhysicalDeviceQueueFamilyProperties(PhysicalDevice);

    VkBool32 PresentSupported = false;

    for (uint32_t i = 0; i < Properties.size(); ++i)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, i, Surface, &PresentSupported);
        if (PresentSupported)
        {
            QueueFamilyIndex = i;
            return true;
        }
    }

    return false;
}

bool FVulkanContext::CheckDeviceQueueSupport(VkPhysicalDevice PhysicalDevice)
{
    bool EverythingIsOK = true;

    for (auto& Entry : Queues)
    {
        EverythingIsOK = CheckDeviceQueueSupport(PhysicalDevice, Entry.first, Entry.second.QueueIndex);
        if (!EverythingIsOK)
        {
            return false;
        }
    }

    if (EverythingIsOK)
    {
        if (!CheckDeviceQueuePresentSupport(PhysicalDevice, PresentQueue.QueueIndex)) {
            return false;
        }
    }

    return true;
}

VkQueue FVulkanContext::GetQueue(VkQueueFlagBits QueueFlagBits)
{
    if (Queues.find(QueueFlagBits) == Queues.end())
    {
        throw std::runtime_error("Failed to find a requested queue!");
    }
    return Queues[QueueFlagBits].Queue;
}

uint32_t FVulkanContext::GetQueueIndex(VkQueueFlagBits QueueFlagBits)
{
    if (Queues.find(QueueFlagBits) == Queues.end())
    {
        throw std::runtime_error("Failed to find a requested queue!");
    }
    return Queues[QueueFlagBits].QueueIndex;
}

VkQueue FVulkanContext::GetGraphicsQueue()
{
    return GetQueue(VK_QUEUE_GRAPHICS_BIT);
}

uint32_t FVulkanContext::GetGraphicsQueueIndex()
{
    return GetQueueIndex(VK_QUEUE_GRAPHICS_BIT);
}

VkQueue FVulkanContext::GetComputeQueue()
{
    return GetQueue(VK_QUEUE_COMPUTE_BIT);
}

uint32_t FVulkanContext::GetComputeQueueIndex()
{
    return GetQueueIndex(VK_QUEUE_COMPUTE_BIT);
}

VkQueue FVulkanContext::GetTransferQueue()
{
    return GetQueue(VK_QUEUE_TRANSFER_BIT);
}

uint32_t FVulkanContext::GetTransferQueueIndex()
{
    return GetQueueIndex(VK_QUEUE_TRANSFER_BIT);
}

VkQueue FVulkanContext::GetSparseBindingQueue()
{
    return GetQueue(VK_QUEUE_SPARSE_BINDING_BIT);
}

uint32_t FVulkanContext::GetSparseBindingQueueIndex()
{
    return GetQueueIndex(VK_QUEUE_SPARSE_BINDING_BIT);
}

VkQueue FVulkanContext::GetPresentQueue()
{
    return PresentQueue.Queue;
}
uint32_t FVulkanContext::GetPresentIndex()
{
    return PresentQueue.QueueIndex;
}

FBuffer FVulkanContext::CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties)

{
    return ResourceAllocator->CreateBuffer(Size, Usage, Properties);
}

FMemoryPtr FVulkanContext::PushDataToBuffer(FBuffer& Buffer, VkDeviceSize Size, void* Data)
{
    return ResourceAllocator->PushDataToBuffer(Buffer, Size, Data);
}

void FVulkanContext::CopyBuffer(FBuffer &SrcBuffer, FBuffer &DstBuffer, VkDeviceSize Size, VkDeviceSize SourceOffset, VkDeviceSize DestinationOffset)
{
    return ResourceAllocator->CopyBuffer(SrcBuffer, DstBuffer, Size, SourceOffset, DestinationOffset);
}

void FVulkanContext::DestroyBuffer(FBuffer& Buffer)
{
    ResourceAllocator->DestroyBuffer(Buffer);
}


VkDevice FVulkanContext::CreateLogicalDevice(VkPhysicalDevice PhysicalDevice)
{
    std::set<uint32_t> QueueIndices;
    QueueIndices.insert(PresentQueue.QueueIndex);
    for (auto& Entry : Queues)
    {
        QueueIndices.insert(Entry.second.QueueIndex);
    }
    auto DeviceQueueCreateInfo = GetDeviceQueueCreateInfo(PhysicalDevice, QueueIndices);
    VkDeviceCreateInfo CreateInfo{};

    VkPhysicalDeviceFeatures DeviceFeatures{};
    DeviceFeatures.samplerAnisotropy = VK_TRUE;
    DeviceFeatures.sampleRateShading = VK_TRUE;

    CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    CreateInfo.pQueueCreateInfos = DeviceQueueCreateInfo.data();
    CreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueIndices.size());
    CreateInfo.pEnabledFeatures = &DeviceFeatures;
    VulkanContextOptions.BuildDevicePNextChain(reinterpret_cast<BaseVulkanStructure*>(&CreateInfo));
    auto DeviceExtensions = VulkanContextOptions.GetDeviceExtensionsList();
    CreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
    CreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();
    auto DeviceLayers = VulkanContextOptions.GetDeviceLayers();
    CreateInfo.enabledLayerCount = static_cast<uint32_t>(DeviceLayers.size());
    CreateInfo.ppEnabledLayerNames = DeviceLayers.data();

    if (vkCreateDevice(PhysicalDevice, &CreateInfo, nullptr, &LogicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device!");
    }

    return LogicalDevice;
}

void FVulkanContext::GetDeviceQueues()
{
    for (auto& Entry : Queues)
    {
        CheckDeviceQueueSupport(PhysicalDevice, Entry.first, Entry.second.QueueIndex);
        vkGetDeviceQueue(LogicalDevice, Entry.second.QueueIndex, 0, &Entry.second.Queue);
    }
    CheckDeviceQueuePresentSupport(PhysicalDevice, PresentQueue.QueueIndex);
    vkGetDeviceQueue(LogicalDevice, PresentQueue.QueueIndex, 0, &PresentQueue.Queue);

    ResourceAllocator = std::make_shared<FResourceAllocator>(PhysicalDevice, LogicalDevice, this);
    DescriptorSetManager = std::make_shared<FDescriptorSetManager>(LogicalDevice);
}

std::vector<VkDeviceQueueCreateInfo> FVulkanContext::GetDeviceQueueCreateInfo(VkPhysicalDevice PhysicalDevice, std::set<uint32_t> UniqueQueueFamilies)
{
    std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;

    static float QueuePriority = 1.f;
    for (auto QueueFamilyIndex : UniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo QueueCreateInfo{};
        QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        QueueCreateInfo.queueFamilyIndex = QueueFamilyIndex;
        QueueCreateInfo.queueCount = 1;
        QueueCreateInfo.pQueuePriorities = &QueuePriority;
        QueueCreateInfos.push_back(QueueCreateInfo);
    }

    return QueueCreateInfos;
}

void FVulkanContext::SaveImage(const FImage& Image)
{
    std::vector<char> Data;

    FetchImageData(Image, Data);

    stbi_write_bmp((Image.DebugName + ".png").c_str(), Image.Width, Image.Height, 4, Data.data());
}

template <typename T>
void FVulkanContext::FetchImageData(const FImage& Image, std::vector<T>& Data)
{
    uint32_t NumberOfComponents = 0;

    switch (Image.Format) {
        case VK_FORMAT_B8G8R8A8_SRGB:
        {
            NumberOfComponents = 4;
            break;
        }
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        {
            NumberOfComponents = 4;
            break;
        }
        case VK_FORMAT_R32_UINT:
        {
            NumberOfComponents = 1;
            break;
        }
    }

    auto& Context = GetContext();
    uint32_t Size = Image.Height * Image.Width * NumberOfComponents * sizeof(T);
    FBuffer Buffer = Context.CreateBuffer(Size, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    Context.ResourceAllocator->CopyImageToBuffer(Image, Buffer);

    Data.resize(Size);

    void* BufferData;
    vkMapMemory(Context.LogicalDevice, Buffer.MemoryRegion.Memory, 0, Buffer.BufferSize, 0, &BufferData);
    memcpy(Data.data(), BufferData, (std::size_t)Buffer.BufferSize);
    vkUnmapMemory(Context.LogicalDevice, Buffer.MemoryRegion.Memory);

    Context.DestroyBuffer(Buffer);
}

template void FVulkanContext::FetchImageData<uint32_t>(const FImage& Image, std::vector<uint32_t>& Data);

ImagePtr FVulkanContext::CreateImage2D(uint32_t Width, uint32_t Height, bool bMipMapsRequired, VkSampleCountFlagBits NumSamples, VkFormat Format,
                           VkImageTiling Tiling, VkImageUsageFlags Usage, VkMemoryPropertyFlags Properties,
                           VkImageAspectFlags AspectFlags, VkDevice Device, const std::string& DebugImageName)
{

    ImagePtr Image = std::make_shared<FImage>(Width, Height, bMipMapsRequired, NumSamples, Format, Tiling, Usage,
                                                 Properties, AspectFlags, Device, DebugImageName);
    return Image;
}

ImagePtr FVulkanContext::LoadImageFromFile(const std::string& Path, const std::string& DebugImageName)
{
    /// Load data from image file
    int TexWidth, TexHeight, TexChannels;
    stbi_uc* Pixels = stbi_load(Path.c_str(), &TexWidth, &TexHeight, &TexChannels, STBI_rgb_alpha);
    VkDeviceSize ImageSize = TexWidth * TexHeight * 4;

    if (!Pixels)
    {
        throw std::runtime_error("Failed to load texture image!");
    }

    /// Load data into staging buffer


    ImagePtr Image = std::make_shared<FImage>(TexWidth, TexHeight, true, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, DebugImageName);

    V::SetName(LogicalDevice, Image->Image, DebugImageName);
    V::SetName(LogicalDevice, Image->View, (std::string(DebugImageName) + "_ImageView").c_str());

    Image->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    ResourceAllocator->LoadDataToImage(*Image, ImageSize, Pixels);
    /// Generate MipMaps only after we loaded image data
    Image->GenerateMipMaps();

    return Image;
}

ImagePtr FVulkanContext::Wrap(VkImage ImageToWrap, VkFormat Format, VkImageAspectFlags AspectFlags, VkDevice LogicalDevice, const std::string& DebugImageName)
{
    ImagePtr Image = std::make_shared<FImage>(ImageToWrap, Format, AspectFlags, LogicalDevice, DebugImageName);

    return Image;
}

void FVulkanContext::CreateDepthAndAAImages()
{
    /// Create Image and ImageView for AA
    auto Width = Swapchain->GetWidth();
    auto Height = Swapchain->GetHeight();

    ColorImage = CreateImage2D(Width, Height, false, MSAASamples, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_ColorImage");


    NormalsImage = CreateImage2D(Width, Height, false, MSAASamples, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                            VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_NormalsImage");


    RenderableIndexImage = CreateImage2D(Width, Height, false, MSAASamples, VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL,
                                                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                    VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_RenderableIndexImage");

    UtilityImageR32 = CreateImage2D(Width, Height, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                               VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_UtilityImageR32");

    UtilityImageR32->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    ResolvedColorImage = CreateImage2D(Width, Height, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_ResolvedColorImage");

    UtilityImageR8G8B8A8_SRGB = CreateImage2D(Width, Height, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                                                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                         VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_UtilityImageR8G8B8A8_SRGB");

    /// Create Image and ImageView for Depth
    VkFormat DepthFormat = FindDepthFormat();
    DepthImage = CreateImage2D(Width, Height, false, MSAASamples, DepthFormat, VK_IMAGE_TILING_OPTIMAL,
                                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                          VK_IMAGE_ASPECT_DEPTH_BIT, LogicalDevice, "V_DepthImage");


    DepthImage->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

VkFramebuffer FVulkanContext::CreateFramebuffer(std::vector<ImagePtr> Images, VkRenderPass RenderPass, const std::string& debug_name)
{
    std::vector<VkImageView> Attachments;

    for (auto& Image : Images)
    {
        Attachments.push_back(Image->View);
    }

    VkFramebufferCreateInfo FramebufferCreateInfo{};
    FramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    FramebufferCreateInfo.renderPass = RenderPass;
    FramebufferCreateInfo.attachmentCount = static_cast<uint32_t>(Attachments.size());
    FramebufferCreateInfo.pAttachments = Attachments.data();
    FramebufferCreateInfo.width = Swapchain->GetWidth();
    FramebufferCreateInfo.height = Swapchain->GetHeight();
    FramebufferCreateInfo.layers = 1;

    VkFramebuffer Framebuffer = nullptr;

    if (vkCreateFramebuffer(LogicalDevice, &FramebufferCreateInfo, nullptr, &Framebuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create framebuffer: " + debug_name);
    }

    V::SetName(LogicalDevice, Framebuffer, debug_name);

    return Framebuffer;
}

void FVulkanContext::CreatePassthroughRenderPass()
{
    PassthroughRenderPass = std::make_shared<FRenderPass>();
    PassthroughRenderPass->AddImageAsAttachment(Swapchain->Images[0], AttachmentType::Color, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);

    PassthroughRenderPass->Construct(LogicalDevice);
    V::SetName(LogicalDevice, PassthroughRenderPass->RenderPass, "V_PassthroughRenderpass");
}

void FVulkanContext::CreateRenderPass()
{
    RenderPass = std::make_shared<FRenderPass>();
    RenderPass->AddImageAsAttachment(ColorImage, AttachmentType::Color, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
    RenderPass->AddImageAsAttachment(NormalsImage, AttachmentType::Color, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
    RenderPass->AddImageAsAttachment(RenderableIndexImage, AttachmentType::Color, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
    RenderPass->AddImageAsAttachment(DepthImage, AttachmentType::DepthStencil, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
    RenderPass->AddImageAsAttachment(ResolvedColorImage, AttachmentType::Resolve, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);

    RenderPass->Construct(LogicalDevice);
    V::SetName(LogicalDevice, RenderPass->RenderPass, "V_RenderRenderpass");
}

void FVulkanContext::CreateImguiRenderpasss()
{
    ImGuiRenderPass = std::make_shared<FRenderPass>();
    ImGuiRenderPass->AddImageAsAttachment(Swapchain->Images[0], AttachmentType::Color, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ATTACHMENT_LOAD_OP_LOAD);

    ImGuiRenderPass->Construct(LogicalDevice);
    V::SetName(LogicalDevice, ImGuiRenderPass->RenderPass, "V_ImGuiRenderPass");
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
        SwapChainFramebuffers[i] = CreateFramebuffer({ColorImage, NormalsImage, RenderableIndexImage, DepthImage, ResolvedColorImage}, RenderPass->RenderPass, "V_Render_fb_" + std::to_string(i));
    }
}

void FVulkanContext::CreatePassthroughFramebuffers()
{
    PassthroughFramebuffers.resize(Swapchain->Size());
    for (std::size_t i = 0; i < PassthroughFramebuffers.size(); ++i)
    {
        PassthroughFramebuffers[i] = CreateFramebuffer({Swapchain->Images[i]}, PassthroughRenderPass->RenderPass, "V_Passthrough_fb_" + std::to_string(i));
    }
}

void FVulkanContext::CreateImguiFramebuffers()
{
    ImGuiFramebuffers.resize(Swapchain->Size());

    for(uint32_t i = 0; i < Swapchain->Size(); ++i)
    {
        ImGuiFramebuffers[i] = CreateFramebuffer({Swapchain->Images[i]}, ImGuiRenderPass->RenderPass, "V_Imgui_fb_" + std::to_string(i));
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
        DeviceTransformBuffers[i] = CreateBuffer(TransformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        DeviceCameraBuffers[i] = CreateBuffer(CameraBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        DeviceRenderableBuffers[i] = CreateBuffer(RenderableBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
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
    V::SetName(LogicalDevice, ImGuiDescriptorPool, "V_ImGuiDescriptorPool");
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
        ImageBufferInfo.imageView = TextureImage->View;
        ImageBufferInfo.sampler = TextureSampler;
        DescriptorSetManager->UpdateDescriptorSetInfo(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME, LAYOUTS::TEXTURE_SAMPLER_LAYOUT_NAME, i, ImageBufferInfo);
    }

    for (size_t i = 0; i < Swapchain->Size(); ++i)
    {
        VkDescriptorImageInfo ImageBufferInfo{};
        ImageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ImageBufferInfo.imageView = ResolvedColorImage->View;
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

        V::SetName(LogicalDevice, GraphicsCommandBuffers[i], "V_GraphicsCommandBuffers" + std::to_string(i));
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
            Barrier.image = ResolvedColorImage->Image;
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

        V::SetName(LogicalDevice, PassthroughCommandBuffers[i], "V_PassthroughCommandBuffers" + std::to_string(i));
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
    InitInfo.QueueFamily = GetGraphicsQueueIndex();
    InitInfo.Queue = GetGraphicsQueue();
    InitInfo.DescriptorPool = ImGuiDescriptorPool;
    InitInfo.MinImageCount = MAX_FRAMES_IN_FLIGHT;
    InitInfo.ImageCount = MAX_FRAMES_IN_FLIGHT;
    InitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    InitInfo.CheckVkResultFn = CheckResultFunction;
    ImGui_ImplVulkan_Init(&InitInfo, ImGuiRenderPass->RenderPass);

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
    if (vkQueueSubmit(GetGraphicsQueue(), 1, &SubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
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
    if (vkQueueSubmit(GetGraphicsQueue(), 1, &PassThroughSubmitInfo, RenderingFinishedFences[CurrentFrame]) != VK_SUCCESS)
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

    V::SetName(LogicalDevice, CommandBuffer, "V_ImguiCommandBuffer" + std::to_string(CurrentFrame % Swapchain->Size()));

    CommandBufferManager->RecordCommand([&, this](VkCommandBuffer)
    {
        {
            VkRenderPassBeginInfo RenderPassBeginInfo{};
            RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            RenderPassBeginInfo.renderPass = ImGuiRenderPass->RenderPass;
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

        if (vkQueueSubmit(GetGraphicsQueue(), 1, &SubmitInfo, ImGuiFinishedFences[CurrentFrame]) != VK_SUCCESS)
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

    VkResult Result = vkQueuePresentKHR(PresentQueue.Queue, &PresentInfo);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR || bFramebufferResized)
    {
        bFramebufferResized = false;
        RecreateSwapChain();
        return;
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

    Swapchain = std::make_shared<FSwapchain>(*this, PhysicalDevice, LogicalDevice, Surface, Window, GetGraphicsQueueIndex(), GetPresentIndex(), VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_PRESENT_MODE_MAILBOX_KHR);
    CreateDepthAndAAImages();

    CreateRenderPass();
    CreatePassthroughRenderPass();
    CreateImguiRenderpasss();

    CreateGraphicsPipeline();
    CreatePassthroughPipeline();

    CreateRenderFramebuffers();
    CreatePassthroughFramebuffers();
    CreateImguiFramebuffers();

    CreateDescriptorPool();

    CreateDescriptorSet();

    CreateCommandBuffers();

    CurrentFrame = 0;
}

void FVulkanContext::CleanUpSwapChain()
{
    /// Remove all images which size's dependent on the swapchain's size
    ColorImage = nullptr;
    ResolvedColorImage = nullptr;
    UtilityImageR8G8B8A8_SRGB = nullptr;
    NormalsImage = nullptr;
    RenderableIndexImage = nullptr;
    UtilityImageR32 = nullptr;
    DepthImage = nullptr;

    /// Remove all framebuffers
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

    /// Remove all command buffers
    for (auto& CommandBuffer : GraphicsCommandBuffers)
    {
        CommandBufferManager->FreeCommandBuffer(CommandBuffer);
    }

    for (auto& CommandBuffer : PassthroughCommandBuffers)
    {
        CommandBufferManager->FreeCommandBuffer(CommandBuffer);
    }

    /// Remove pipelines
    GraphicsPipeline.Delete();
    PassthroughPipeline.Delete();

    /// Remove renderpasses
    RenderPass = nullptr;
    PassthroughRenderPass = nullptr;
    ImGuiRenderPass = nullptr;

    Swapchain = nullptr;

    DescriptorSetManager->Reset();
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

    ResourceAllocator->LoadDataToBuffer(DeviceTransformBuffers[CurrentImage], DeviceTransformComponentsSize, 0, DeviceTransformComponentsData);
    ResourceAllocator->LoadDataToBuffer(DeviceCameraBuffers[CurrentImage], DeviceCameraComponentsSize, 0, DeviceCameraComponentsData);
    ResourceAllocator->LoadDataToBuffer(DeviceRenderableBuffers[CurrentImage], RenderableComponentSize, 0, RenderableComponentData);
}

void FVulkanContext::DestroyDebugUtilsMessengerEXT()
{
#ifndef NDEBUG
    if (nullptr != VulkanContextOptions.GetExtensionStructurePtr<VkDebugUtilsMessengerCreateInfoEXT>(VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT))
    {
        V::vkDestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
    }
#endif
}

void FVulkanContext::CleanUp()
{
    /// Free all device buffers
    for (size_t i = 0; i < Swapchain->Size(); ++i)
    {
        DestroyBuffer(DeviceTransformBuffers[i]);
        DestroyBuffer(DeviceCameraBuffers[i]);
        DestroyBuffer(DeviceRenderableBuffers[i]);
    }

    CleanUpSwapChain();

    vkDestroyDescriptorPool(LogicalDevice, ImGuiDescriptorPool, nullptr);

    vkDestroySampler(LogicalDevice, TextureSampler, nullptr);
    TextureImage = nullptr;

    DescriptorSetManager->DestroyDescriptorSetLayout(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME);
    DescriptorSetManager->DestroyDescriptorSetLayout(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME);
    DescriptorSetManager->DestroyDescriptorSetLayout(LAYOUT_SETS::PASSTHROUGH_LAYOUT_NAME);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    auto& Coordinator = ECS::GetCoordinator();

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
    ResourceAllocator = nullptr;
    vkDestroyDevice(LogicalDevice, nullptr);

    DestroyDebugUtilsMessengerEXT();


    vkDestroySurfaceKHR(Instance, Surface, nullptr);
    vkDestroyInstance(Instance, nullptr);
}

FVulkanContext::~FVulkanContext()
{
}
