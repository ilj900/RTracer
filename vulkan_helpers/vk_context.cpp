#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

#include "systems/camera_system.h"
#include "components/device_camera_component.h"
#include "components/device_transform_component.h"
#include "components/device_renderable_component.h"
#include "coordinator.h"

#include <stdexcept>
#include <iostream>
#include <set>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static FVulkanContext Context{};

FVulkanContext& GetContext()
{
    return Context;
}

VKAPI_ATTR VkBool32 VKAPI_CALL FVulkanContext::DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT MessageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallBackData,
        void* pUserData)
{
    std::cerr << "Validation layer: " << pCallBackData->pMessage << '\n' << std::endl;

    return VK_FALSE;
}

void FVulkanContext::Init(GLFWwindow* Window, int Width, int Height)
{
    try
    {
        CreateUniformBuffers();
        TextureImage = LoadImageFromFile(TexturePath, "V_TextureImage");

        CreatePipelines();
        CreateSyncObjects();
    }
    catch (std::runtime_error &Error) {
        std::cout << Error.what() << std::endl;
        throw;
    }
}

#ifndef NDEBUG
VkDebugUtilsMessengerEXT FVulkanContext::CreateDebugMessenger(FVulkanContextOptions& VulkanContextOptions)
{
    VkDebugUtilsMessengerEXT DebugUtilsMessengerEXT;
    auto* DebugCreateInfo = VulkanContextOptions.GetExtensionStructurePtr<VkDebugUtilsMessengerCreateInfoEXT>(VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT);

    V::vkCreateDebugUtilsMessengerEXT(Instance, DebugCreateInfo, nullptr, &DebugUtilsMessengerEXT);

    return DebugUtilsMessengerEXT;
}

void FVulkanContext::SetDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT DebugUtilsMessengerEXT)
{
    this->DebugMessenger = DebugUtilsMessengerEXT;
}
#endif

VkSurfaceKHR FVulkanContext::CreateSurface(GLFWwindow* Window)
{
    VkSurfaceKHR Surface;
    VkResult Result = glfwCreateWindowSurface(Instance, Window, nullptr, &Surface);
    assert((Result == VK_SUCCESS) && "Failed to create window surface!");
    return Surface;
}

void FVulkanContext::DestroySurface(VkSurfaceKHR* Surface)
{
    vkDestroySurfaceKHR(Instance, *Surface, nullptr);
    *Surface = VK_NULL_HANDLE;
}

void FVulkanContext::SetSurface(VkSurfaceKHR Surface)
{
    this->Surface = Surface;
}

VkSurfaceKHR FVulkanContext::GetSurface()
{
    return Surface;
}

void FVulkanContext::SetWindow(GLFWwindow* Window)
{
    this->Window = Window;
}

GLFWwindow* FVulkanContext::GetWindow()
{
    return Window;
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

VkPhysicalDevice FVulkanContext::PickPhysicalDevice(FVulkanContextOptions& VulkanContextOptions, VkSurfaceKHR Surface)
{
    auto Devices = EnumerateAllPhysicalDevices(Instance);

    auto DeviceExtensionList = VulkanContextOptions.GetDeviceExtensionsList();

    std::set<std::string> RequiredExtensions (DeviceExtensionList.begin(), DeviceExtensionList.end());

    for (const auto& Device : Devices)
    {
        if (CheckDeviceExtensionsSupport(Device, RequiredExtensions) && (CheckDeviceQueueSupport(Device, Surface)))
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

    return PhysicalDevice;
}

void FVulkanContext::SetPhysicalDevice(VkPhysicalDevice PhysicalDevice)
{
    this->PhysicalDevice = PhysicalDevice;
}

VkPhysicalDevice FVulkanContext::GetPhysicalDevice()
{
    return PhysicalDevice;
}

void FVulkanContext::InitManagerResources(int Width, int Height, VkSurfaceKHR Surface)
{
    ResourceAllocator = std::make_shared<FResourceAllocator>(PhysicalDevice, LogicalDevice, this);

    DescriptorSetManager = std::make_shared<FDescriptorSetManager>(LogicalDevice);

    CommandBufferManager = std::make_shared<FCommandBufferManager>(LogicalDevice, this, GetQueue(VK_QUEUE_GRAPHICS_BIT),
                                                                   GetQueueIndex(VK_QUEUE_GRAPHICS_BIT));
    Swapchain = std::make_shared<FSwapchain>(*this, Width, Height, PhysicalDevice, LogicalDevice, Surface, GetGraphicsQueueIndex(), GetPresentIndex(), VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_PRESENT_MODE_MAILBOX_KHR);
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

VkInstance FVulkanContext::CreateVkInstance(const std::string& AppName, const FVersion3& AppVersion, const std::string& EngineName, const FVersion3& EngineVersion, uint32_t ApiVersion, FVulkanContextOptions& VulkanContextOptions)
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

void FVulkanContext::SetInstance(VkInstance Instance)
{
    this->Instance = Instance;
}

VkInstance FVulkanContext::GetInstance()
{
    return this->Instance;
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

bool FVulkanContext::CheckDeviceQueuePresentSupport(VkPhysicalDevice PhysicalDevice, uint32_t& QueueFamilyIndex, VkSurfaceKHR Surface)
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

bool FVulkanContext::CheckDeviceQueueSupport(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface)
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
        if (!CheckDeviceQueuePresentSupport(PhysicalDevice, PresentQueue.QueueIndex, Surface)) {
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

FBuffer FVulkanContext::CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties, const std::string& DebugName)
{
    return ResourceAllocator->CreateBuffer(Size, Usage, Properties, DebugName);
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

VkDevice FVulkanContext::CreateLogicalDevice(VkPhysicalDevice PhysicalDevice, FVulkanContextOptions& VulkanContextOptions)
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

    VkDevice LogicalDevice;

    if (vkCreateDevice(PhysicalDevice, &CreateInfo, nullptr, &LogicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device!");
    }

    return LogicalDevice;
}

void FVulkanContext::SetLogicalDevice(VkDevice LogicalDevice)
{
    this->LogicalDevice = LogicalDevice;
}

void FVulkanContext::GetDeviceQueues(VkSurfaceKHR Surface)
{
    for (auto& Entry : Queues)
    {
        CheckDeviceQueueSupport(PhysicalDevice, Entry.first, Entry.second.QueueIndex);
        vkGetDeviceQueue(LogicalDevice, Entry.second.QueueIndex, 0, &Entry.second.Queue);
    }
    CheckDeviceQueuePresentSupport(PhysicalDevice, PresentQueue.QueueIndex, Surface);
    vkGetDeviceQueue(LogicalDevice, PresentQueue.QueueIndex, 0, &PresentQueue.Queue);
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
    FBuffer Buffer = Context.CreateBuffer(Size, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "Tmp_Save_Image_Buffer");
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

VkDescriptorPool FVulkanContext::CreateDescriptorPool(const std::map<VkDescriptorType, uint32_t>& DescriptorsMap, VkDevice LogicalDevice, const std::string& debug_name)
{
    /// Fill it pool sizes
    std::vector<VkDescriptorPoolSize> PoolSizes{};
    uint32_t MaxSets = 0;
    for (auto Type : DescriptorsMap)
    {
        PoolSizes.push_back({Type.first, Type.second});
        MaxSets += Type.second;
    }

    VkDescriptorPoolCreateInfo PoolInfo{};
    PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    PoolInfo.poolSizeCount = static_cast<uint32_t>(PoolSizes.size());
    PoolInfo.pPoolSizes = PoolSizes.data();
    PoolInfo.maxSets = static_cast<uint32_t>(MaxSets);

    VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;

    if (vkCreateDescriptorPool(LogicalDevice, &PoolInfo, nullptr, &DescriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool!");
    }

    V::SetName(LogicalDevice, DescriptorPool, debug_name);

    return DescriptorPool;
}

VkShaderModule FVulkanContext::CreateShaderFromFile(const std::string& FileName)
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

VkRenderPass FVulkanContext::CreateRenderpass(VkDevice LogicalDevice, FGraphicsPipelineOptions& GraphicsPipelineOptions)
{
    std::vector<VkAttachmentDescription> AttachmentDescriptions;
    std::vector<VkAttachmentReference> ColorAttachmentReferences;

    for (int i = 0; i < GraphicsPipelineOptions.ColorAttachmentDescriptions.size(); ++i)
    {
        auto ColorEntry = GraphicsPipelineOptions.ColorAttachmentDescriptions[i];
        if (VK_FORMAT_UNDEFINED != ColorEntry.format)
        {
            AttachmentDescriptions.push_back(ColorEntry);
            VkAttachmentReference AttachmentReference{};
            AttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            AttachmentReference.attachment = AttachmentDescriptions.size() - 1 ;
            ColorAttachmentReferences.push_back(AttachmentReference);
        }
    }

    std::vector<VkAttachmentReference> DepthStencilAttachmentReferences;

    {
        if (VK_FORMAT_UNDEFINED != GraphicsPipelineOptions.DepthStencilAttachmentDescriptions.format)
        {
            AttachmentDescriptions.push_back(GraphicsPipelineOptions.DepthStencilAttachmentDescriptions);
            VkAttachmentReference AttachmentReference{};
            AttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            AttachmentReference.attachment = AttachmentDescriptions.size() - 1;
            DepthStencilAttachmentReferences.push_back(AttachmentReference);
        }
    }

    std::vector<VkAttachmentReference> ResolveAttachmentReferences(ColorAttachmentReferences.size(), {VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED});

    for (int i = 0; i < GraphicsPipelineOptions.ResolveAttachmentDescriptions.size(); ++i)
    {
        auto ResolveEntry = GraphicsPipelineOptions.ResolveAttachmentDescriptions[i];

        if (VK_FORMAT_UNDEFINED != ResolveEntry.format)
        {
            AttachmentDescriptions.push_back(ResolveEntry);
            VkAttachmentReference AttachmentReference{};
            AttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            AttachmentReference.attachment = AttachmentDescriptions.size() - 1 ;
            ResolveAttachmentReferences[i] = (AttachmentReference);
        }
    }

    VkSubpassDependency Dependency{};
    Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    Dependency.dstSubpass = 0;
    Dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    Dependency.srcAccessMask = 0;
    Dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkSubpassDescription Subpass{};
    Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    Subpass.colorAttachmentCount = ColorAttachmentReferences.size();
    Subpass.pColorAttachments = (ColorAttachmentReferences.size() > 0) ? ColorAttachmentReferences.data() : nullptr;
    Subpass.pDepthStencilAttachment = (DepthStencilAttachmentReferences.size() > 0) ? DepthStencilAttachmentReferences.data() : nullptr;
    Subpass.pResolveAttachments = (ResolveAttachmentReferences.size() > 0) ? ResolveAttachmentReferences.data() : nullptr;

    VkRenderPassCreateInfo RenderPassInfo{};
    RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    RenderPassInfo.attachmentCount = static_cast<uint32_t>(AttachmentDescriptions.size());
    RenderPassInfo.pAttachments = AttachmentDescriptions.data();
    RenderPassInfo.subpassCount = 1;
    RenderPassInfo.pSubpasses = &Subpass;
    RenderPassInfo.dependencyCount = 1;
    RenderPassInfo.pDependencies  = &Dependency;

    VkRenderPass RenderPass = VK_NULL_HANDLE;

    if (vkCreateRenderPass(LogicalDevice, &RenderPassInfo, nullptr, &RenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create render pass!");
    }

    return RenderPass;
}

VkPipeline FVulkanContext::CreateGraphicsPipeline(VkShaderModule VertexShader, VkShaderModule FragmentShader, std::uint32_t Width, std::uint32_t Height, FGraphicsPipelineOptions& GraphicsPipelineOptions)
{
    GraphicsPipelineOptions.RenderPass = CreateRenderpass(LogicalDevice, GraphicsPipelineOptions);

    std::vector<VkPipelineShaderStageCreateInfo> PipelineShaderStageCreateInfoVector(2);

    PipelineShaderStageCreateInfoVector[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    PipelineShaderStageCreateInfoVector[0].module = VertexShader;
    PipelineShaderStageCreateInfoVector[0].pName = "main";
    PipelineShaderStageCreateInfoVector[0].stage = VK_SHADER_STAGE_VERTEX_BIT;

    PipelineShaderStageCreateInfoVector[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    PipelineShaderStageCreateInfoVector[1].module = FragmentShader;
    PipelineShaderStageCreateInfoVector[1].pName = "main";
    PipelineShaderStageCreateInfoVector[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkPipelineVertexInputStateCreateInfo VertexInputInfo{};
    VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VertexInputInfo.vertexBindingDescriptionCount = GraphicsPipelineOptions.VertexInputBindingDescriptionVector.size();
    VertexInputInfo.vertexAttributeDescriptionCount = GraphicsPipelineOptions.VertexInputAttributeDescriptionVector.size();
    VertexInputInfo.pVertexBindingDescriptions = (VertexInputInfo.vertexBindingDescriptionCount > 0) ? GraphicsPipelineOptions.VertexInputBindingDescriptionVector.data() : nullptr;
    VertexInputInfo.pVertexAttributeDescriptions = (VertexInputInfo.vertexAttributeDescriptionCount > 0) ? GraphicsPipelineOptions.VertexInputAttributeDescriptionVector.data() : nullptr;

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
    Scissors.extent = {Width, Height};

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
    Multisampling.rasterizationSamples = GraphicsPipelineOptions.MSAASamples;
    Multisampling.minSampleShading = 0.2f;
    Multisampling.pSampleMask = nullptr;
    Multisampling.alphaToCoverageEnable = VK_FALSE;
    Multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState ColorBlendAttachment{};
    ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          +                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    ColorBlendAttachment.blendEnable = VK_FALSE;
    ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    std::vector<VkPipelineColorBlendAttachmentState> ColorBlendingAttachments(GraphicsPipelineOptions.ColorAttachmentDescriptions.size(), ColorBlendAttachment);

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
    PipelineInfo.pStages = PipelineShaderStageCreateInfoVector.data();
    PipelineInfo.pVertexInputState = &VertexInputInfo;
    PipelineInfo.pInputAssemblyState = &InputAssembly;
    PipelineInfo.pViewportState = &ViewportState;
    PipelineInfo.pRasterizationState = &Rasterizer;
    PipelineInfo.pMultisampleState = &Multisampling;
    PipelineInfo.pDepthStencilState = nullptr;
    PipelineInfo.pColorBlendState = &ColorBlending;
    PipelineInfo.pDepthStencilState = &DepthStencil;
    PipelineInfo.layout = GraphicsPipelineOptions.PipelineLayout;
    PipelineInfo.renderPass = GraphicsPipelineOptions.RenderPass;
    PipelineInfo.subpass = 0;
    PipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    PipelineInfo.basePipelineIndex = -1;

    VkPipeline Pipeline = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(LogicalDevice, VK_NULL_HANDLE, 1, &PipelineInfo, nullptr, &Pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }

    return Pipeline;
}

VkSemaphore FVulkanContext::CreateSemaphore()
{
    VkSemaphoreCreateInfo SemaphoreInfo{};
    SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore Semaphore;

    if (vkCreateSemaphore(LogicalDevice, &SemaphoreInfo, nullptr, &Semaphore) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create synchronization objects for a frame!");
    }

    return Semaphore;
}

VkFence FVulkanContext::CreateSignalledFence()
{
    VkFenceCreateInfo FenceInfo{};
    FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence Fence;

    if (vkCreateFence(LogicalDevice, &FenceInfo, nullptr, &Fence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create VkFence!");
    }

    return Fence;
}

VkFence FVulkanContext::CreateUnsignalledFence()
{
    VkFenceCreateInfo FenceInfo{};
    FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkFence Fence;

    if (vkCreateFence(LogicalDevice, &FenceInfo, nullptr, &Fence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create VkFence!");
    }

    return Fence;
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

void FVulkanContext::CreatePipelines()
{
    uint32_t Width = Swapchain->GetWidth();
    uint32_t Height = Swapchain->GetHeight();

    auto ColorImage = CreateImage2D(Width, Height, false, MSAASamples, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_ColorImage");


    auto NormalsImage = CreateImage2D(Width, Height, false, MSAASamples, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                                   VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                   VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_NormalsImage");


    auto RenderableIndexImage = CreateImage2D(Width, Height, false, MSAASamples, VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL,
                                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                           VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_RenderableIndexImage");

    auto ResolvedColorImage = CreateImage2D(Width, Height, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                         VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_ResolvedColorImage");

    RenderTask = std::make_shared<FRenderTask>(this, int(Swapchain->Size()), LogicalDevice);
    RenderTask->RegisterOutput(0, ColorImage);
    RenderTask->RegisterOutput(1, NormalsImage);
    RenderTask->RegisterOutput(2, RenderableIndexImage);
    RenderTask->RegisterOutput(3, ResolvedColorImage);

    RenderTask->Init();
    RenderTask->UpdateDescriptorSets();
    RenderTask->RecordCommands();


    PassthroughTask = std::make_shared<FPassthroughTask>(this, int(Swapchain->Size()), LogicalDevice);
    PassthroughTask->RegisterInput(0, RenderTask->GetOutput(3));
    PassthroughTask->RegisterOutput(0, RenderTask->GetOutput(3));
    PassthroughTask->Init();
    PassthroughTask->UpdateDescriptorSets();
    PassthroughTask->RecordCommands();

    ImguiTask = std::make_shared<FImguiTask>(this, int (Swapchain->Size()), LogicalDevice);
    ImguiTask->RegisterInput(0, PassthroughTask->GetOutput(0));
    ImguiTask->Init();
}

VkSampler FVulkanContext::CreateTextureSampler(uint32_t MipLevel)
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
    SamplerInfo.maxLod = static_cast<float>(MipLevel);

    VkSampler Sampler = VK_NULL_HANDLE;

    if (vkCreateSampler(LogicalDevice, &SamplerInfo, nullptr, &Sampler) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create texture sampler!");
    }

    return Sampler;
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
        DeviceTransformBuffers[i] = CreateBuffer(TransformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "Device_Transform_Buffer_" + std::to_string(i));
        DeviceCameraBuffers[i] = CreateBuffer(CameraBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "Device_Camera_Buffer_" + std::to_string(i));
        DeviceRenderableBuffers[i] = CreateBuffer(RenderableBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "Device_Renderable_Buffer_" + std::to_string(i));
    }
}

void FVulkanContext::CreateSyncObjects()
{
    ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    ImagesInFlight.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
    ImGuiFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo SemaphoreInfo{};
    SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo FenceInfo{};
    FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (vkCreateSemaphore(LogicalDevice, &SemaphoreInfo, nullptr, &ImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(LogicalDevice, &FenceInfo, nullptr, &ImagesInFlight[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create synchronization objects for a frame!");
        }
    }
}

void FVulkanContext::Render()
{
    /// Previous rendering iteration of the frame might still be in use, so we wait for it
    vkWaitForFences(LogicalDevice, 1, &ImagesInFlight[CurrentFrame], VK_TRUE, UINT64_MAX);

    /// Acquire next image from swapchain, also it's index and provide semaphore to signal when image is ready to be used
    VkResult Result = Swapchain->GetNextImage(CurrentImage, ImageAvailableSemaphores[CurrentFrame], ImageIndex);

    /// Run some checks
    if (Result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain(Swapchain->GetWidth(), Swapchain->GetHeight());
        return;
    }
    if (Result != VK_SUCCESS && Result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    UpdateUniformBuffer(ImageIndex);

    auto RenderSignalSemaphore = RenderTask-> Submit(GetGraphicsQueue(), ImageAvailableSemaphores[CurrentFrame], ImagesInFlight[CurrentFrame], VK_NULL_HANDLE, CurrentFrame);

    auto PassthroughSignalSemaphore = PassthroughTask->Submit(GetGraphicsQueue(), RenderSignalSemaphore, VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto ImguiFinishedSemaphore = ImguiTask->Submit(GetGraphicsQueue(), PassthroughSignalSemaphore, VK_NULL_HANDLE, ImagesInFlight[ImageIndex], CurrentFrame);

    ImGuiFinishedSemaphores[CurrentFrame] = ImguiFinishedSemaphore;
}

void FVulkanContext::Present(VkSemaphore WaitSemaphore, uint32_t ImageIndex)
{
    VkSemaphore WaitSemaphores[] = {WaitSemaphore};

    VkPresentInfoKHR PresentInfo{};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pWaitSemaphores = WaitSemaphores;
    VkSwapchainKHR SwapChains[] = {Swapchain->GetSwapchain()};
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = SwapChains;
    PresentInfo.pImageIndices = &ImageIndex;
    PresentInfo.pResults = nullptr;

    VkResult Result = vkQueuePresentKHR(PresentQueue.Queue, &PresentInfo);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR || bFramebufferResized)
    {
        bFramebufferResized = false;
        RecreateSwapChain(Swapchain->GetWidth(), Swapchain->GetHeight());
        return;
    }
    else if (Result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image!");
    }
}

void FVulkanContext::Present()
{
    Present(ImGuiFinishedSemaphores[CurrentFrame], CurrentFrame);

    CurrentFrame = (CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void FVulkanContext::WaitIdle()
{
    vkDeviceWaitIdle(LogicalDevice);
}

void FVulkanContext::RecreateSwapChain(int Width, int Height)
{
    vkDeviceWaitIdle(LogicalDevice);

    CleanUpSwapChain();

    Swapchain = std::make_shared<FSwapchain>(*this, Width, Height, PhysicalDevice, LogicalDevice, Surface, GetGraphicsQueueIndex(), GetPresentIndex(), VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_PRESENT_MODE_MAILBOX_KHR);


    RenderTask = std::make_shared<FRenderTask>(this, int(Swapchain->Size()), LogicalDevice);
    RenderTask->Init();
    RenderTask->UpdateDescriptorSets();
    RenderTask->RecordCommands();

    PassthroughTask = std::make_shared<FPassthroughTask>(this, int(Swapchain->Size()), LogicalDevice);
    PassthroughTask->Init();
    PassthroughTask->UpdateDescriptorSets();
    PassthroughTask->RecordCommands();

    CurrentFrame = 0;
}

void FVulkanContext::CleanUpSwapChain()
{
    Swapchain = nullptr;

    RenderTask->Cleanup();
    RenderTask = nullptr;
    PassthroughTask->Cleanup();
    PassthroughTask = nullptr;
    ImguiTask->Cleanup();
    ImguiTask = nullptr;
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

#ifndef NDEBUG
void FVulkanContext::DestroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT& DebugUtilsMessenger)
{
    if (DebugUtilsMessenger != VK_NULL_HANDLE)
    {
        V::vkDestroyDebugUtilsMessengerEXT(Instance, DebugUtilsMessenger, nullptr);
        DebugUtilsMessenger = VK_NULL_HANDLE;
    }
}
#endif

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

    TextureImage = nullptr;

    DescriptorSetManager = nullptr;

    auto& Coordinator = ECS::GetCoordinator();

    for(std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(LogicalDevice, ImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(LogicalDevice, ImagesInFlight[i], nullptr);
    }

    CommandBufferManager = nullptr;
    ResourceAllocator = nullptr;
    vkDestroyDevice(LogicalDevice, nullptr);

    /// TODO
    DestroyDebugUtilsMessengerEXT(DebugMessenger);

    vkDestroySurfaceKHR(Instance, Surface, nullptr);
    vkDestroyInstance(Instance, nullptr);
}

FVulkanContext::~FVulkanContext()
{
}
