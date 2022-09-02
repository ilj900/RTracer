#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

#include "systems/camera_system.h"
#include "systems/mesh_system.h"
#include "components/device_camera_component.h"
#include "components/device_transform_component.h"
#include "components/device_renderable_component.h"
#include "components/mesh_component.h"
#include "components/device_mesh_component.h"
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
    const std::string TLAS_LAYOUT_NAME = "TLAS";
    const std::string RT_OUT_IMAGE_LAYOUT_NAME = "RT out image layout";
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

        //CreateRenderPass();
        CreatePassthroughRenderPass();
        CreateImguiRenderpasss();

        //CreateDescriptorSetLayouts();
        CreateRTDescriptorSetLayouts();

        //CreateGraphicsPipeline();
        CreateRTPipeline();
        CreatePassthroughPipeline();

        //ImageManager->LoadImageFromFile(TextureImage, TexturePath);

        CreateRenderFramebuffers();
        CreatePassthroughFramebuffers();
        CreateImguiFramebuffers();

        CreateTextureSampler();

        CreateBLAS();
        CreateTLAS();

        LoadModelDataToGPU();

        CreateUniformBuffers();

        //CreateDescriptorPool();
        CreateRTDescriptorPool();
        CreateImguiDescriptorPool();

        //CreateDescriptorSet();
        CreateRTDescriptorSet();

        CreateSBT();

        //CreateCommandBuffers();
        CreateRTCommandBuffers();

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

    VkPhysicalDeviceAccelerationStructureFeaturesKHR PhysicalDeviceAccelerationStructureFeatures{};
    PhysicalDeviceAccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    PhysicalDeviceAccelerationStructureFeatures.accelerationStructure = VK_TRUE;
    VulkanContextOptions.AddDeviceExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, &PhysicalDeviceAccelerationStructureFeatures, sizeof(VkPhysicalDeviceAccelerationStructureFeaturesKHR));

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR PhysicalDeviceRayTracingPipelineFeatures{};
    PhysicalDeviceRayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    PhysicalDeviceRayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
    VulkanContextOptions.AddDeviceExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, &PhysicalDeviceRayTracingPipelineFeatures, sizeof(VkPhysicalDeviceRayTracingPipelineFeaturesKHR));

    VkPhysicalDeviceBufferDeviceAddressFeatures PhysicalDeviceBufferDeviceAddressFeatures{};
    PhysicalDeviceBufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    PhysicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
    VulkanContextOptions.AddDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, &PhysicalDeviceBufferDeviceAddressFeatures, sizeof(VkPhysicalDeviceBufferDeviceAddressFeatures));

    VkPhysicalDeviceHostQueryResetFeatures PhysicalDeviceHostQueryResetFeatures{};
    PhysicalDeviceHostQueryResetFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES;
    PhysicalDeviceHostQueryResetFeatures.hostQueryReset = VK_TRUE;
    VulkanContextOptions.AddDeviceExtension(VK_KHR_RAY_QUERY_EXTENSION_NAME, &PhysicalDeviceHostQueryResetFeatures, sizeof(VkPhysicalDeviceHostQueryResetFeatures));

    VulkanContextOptions.AddDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    VulkanContextOptions.AddDeviceExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    VulkanContextOptions.AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // Resolve and add extensions and layers
    uint32_t Counter = 0;
    auto ExtensionsRequiredByGLFW = glfwGetRequiredInstanceExtensions(&Counter);
    for (uint32_t i = 0; i < Counter; ++i)
    {
        VulkanContextOptions.AddInstanceExtension(ExtensionsRequiredByGLFW[i]);
    }

    Instance = CreateVkInstance("Hello Triangle", {1, 3, 0}, "No Engine", {1, 3, 0}, VK_API_VERSION_1_3, VulkanContextOptions);
}

void FVulkanContext::LoadFunctionPointers()
{
    V::LoadVkFunctions(Instance);
}

void FVulkanContext::SetupDebugMessenger()
{
#ifndef NDEBUG
    auto* DebugCreateInfo = VulkanContextOptions.GetInstanceExtensionStructurePtr<VkDebugUtilsMessengerCreateInfoEXT>(VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT);

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
    std::vector<VkPhysicalDevice> Devices = GetAllPhysicalDevices();

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

    auto DeviceExtensions = VulkanContextOptions.GetDeviceExtensionsList();

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
    CreateInfo.ppEnabledLayerNames = (CreateInfo.enabledLayerCount == 0) ? nullptr : CharLayers.data();

    VkInstance ResultingInstance;
    VkResult Result = vkCreateInstance(&CreateInfo, nullptr, &ResultingInstance);
    assert((Result == VK_SUCCESS) && "Failed to create instance!");
    return ResultingInstance;
}

std::vector<VkPhysicalDevice> FVulkanContext::GetAllPhysicalDevices()
{
    uint32_t DeviceCount = 0;
    vkEnumeratePhysicalDevices(Instance, &DeviceCount, nullptr);

    if (DeviceCount == 0)
    {
        assert(0 &&"Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> Devices(DeviceCount);
    vkEnumeratePhysicalDevices(Instance, &DeviceCount, Devices.data());

    return Devices;
}

VkPhysicalDeviceProperties2 FVulkanContext::GetPhysicalDeviceProperties2(VkPhysicalDevice PhysicalDevice, void* pNextStructure)
{
    VkPhysicalDeviceProperties2 PhysicalDeviceProperties2{};
    PhysicalDeviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    PhysicalDeviceProperties2.pNext = pNextStructure;

    vkGetPhysicalDeviceProperties2(PhysicalDevice, &PhysicalDeviceProperties2);

    return PhysicalDeviceProperties2;
}

FAccelerationStructure FVulkanContext::CreateAccelerationStructure(VkDeviceSize Size, VkAccelerationStructureTypeKHR Type)
{
    FAccelerationStructure AccelerationStructure;
    AccelerationStructure.Type = Type;

    AccelerationStructure.Buffer = ResourceAllocator->CreateBuffer(Size, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkAccelerationStructureCreateInfoKHR AccelerationStructureCreateInfoKHR{};
    AccelerationStructureCreateInfoKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    AccelerationStructureCreateInfoKHR.type = Type;
    AccelerationStructureCreateInfoKHR.size = Size;
    AccelerationStructureCreateInfoKHR.buffer = AccelerationStructure.Buffer.Buffer;

    VkResult Result = vkCreateAccelerationStructureKHR(LogicalDevice, &AccelerationStructureCreateInfoKHR, nullptr, &AccelerationStructure.AccelerationStructure);
    assert((Result == VK_SUCCESS) && "Failed to create acceleration structure");
    return AccelerationStructure;
}

void FVulkanContext::DestroyAccelerationStructure(FAccelerationStructure &AccelerationStructure)
{
    vkDestroyAccelerationStructureKHR(LogicalDevice, AccelerationStructure.AccelerationStructure, nullptr);
    ResourceAllocator->DestroyBuffer(AccelerationStructure.Buffer);
}

VkDeviceAddress FVulkanContext::GetBufferDeviceAddressInfo(FBuffer& Buffer)
{
    VkBufferDeviceAddressInfo BufferDeviceAddressInfo{};
    BufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    BufferDeviceAddressInfo.buffer = Buffer.Buffer;

    auto Result = vkGetBufferDeviceAddress(LogicalDevice, &BufferDeviceAddressInfo);

    return Result;
}

VkDeviceAddress FVulkanContext::GetASDeviceAddressInfo(FAccelerationStructure& AS)
{
    VkAccelerationStructureDeviceAddressInfoKHR AccelerationStructureDeviceAddressInfo{};
    AccelerationStructureDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    AccelerationStructureDeviceAddressInfo.accelerationStructure = AS.AccelerationStructure;
    return vkGetAccelerationStructureDeviceAddressKHR(LogicalDevice, &AccelerationStructureDeviceAddressInfo);
}

VkAccelerationStructureGeometryTrianglesDataKHR FVulkanContext::GetAccelerationStructureGeometryTrianglesData(FBuffer& VertexBuffer, FBuffer& IndexBuffer, uint32_t MaxVertices)
{
    VkAccelerationStructureGeometryTrianglesDataKHR AccelerationStructureGeometryTrianglesData{};
    AccelerationStructureGeometryTrianglesData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    AccelerationStructureGeometryTrianglesData.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    AccelerationStructureGeometryTrianglesData.vertexData.deviceAddress = GetBufferDeviceAddressInfo(VertexBuffer);
    AccelerationStructureGeometryTrianglesData.vertexStride = sizeof (FVertex);
    if (IndexBuffer.Buffer)
    {
        AccelerationStructureGeometryTrianglesData.indexType = VK_INDEX_TYPE_UINT32;
        AccelerationStructureGeometryTrianglesData.indexData.deviceAddress = GetBufferDeviceAddressInfo(IndexBuffer);
    }
    AccelerationStructureGeometryTrianglesData.maxVertex = MaxVertices;

    return AccelerationStructureGeometryTrianglesData;
}

VkAccelerationStructureGeometryKHR FVulkanContext::GetAccelerationStructureGeometry(VkAccelerationStructureGeometryTrianglesDataKHR& AccelerationStructureGeometryTrianglesData)
{
    VkAccelerationStructureGeometryKHR AccelerationStructureGeometry{};
    AccelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    AccelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    AccelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
    AccelerationStructureGeometry.geometry.triangles = AccelerationStructureGeometryTrianglesData;

    return AccelerationStructureGeometry;
}

VkAccelerationStructureBuildRangeInfoKHR FVulkanContext::GetAccelerationStructureBuildRangeInfo(uint32_t PrimitiveCount)
{
    VkAccelerationStructureBuildRangeInfoKHR AccelerationStructureBuildRangeInfo{};
    AccelerationStructureBuildRangeInfo.firstVertex = 0;
    AccelerationStructureBuildRangeInfo.primitiveCount = PrimitiveCount;
    AccelerationStructureBuildRangeInfo.primitiveOffset = 0;
    AccelerationStructureBuildRangeInfo.transformOffset = 0;

    return AccelerationStructureBuildRangeInfo;
}

VkAccelerationStructureBuildGeometryInfoKHR FVulkanContext::GetAccelerationStructureBuildGeometryInfo(VkAccelerationStructureGeometryKHR& AccelerationStructureGeometry, VkBuildAccelerationStructureFlagsKHR Flags)
{
    VkAccelerationStructureBuildGeometryInfoKHR AccelerationStructureBuildGeometryInfo{};
    AccelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    AccelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    AccelerationStructureBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    AccelerationStructureBuildGeometryInfo.flags = Flags;
    AccelerationStructureBuildGeometryInfo.geometryCount = 1;
    AccelerationStructureBuildGeometryInfo.pGeometries = &AccelerationStructureGeometry;

    return AccelerationStructureBuildGeometryInfo;
}

FAccelerationStructure FVulkanContext::GenerateBlas(FBuffer& VertexBuffer, FBuffer& IndexBuffer, uint32_t MaxVertices)
{
    VkAccelerationStructureGeometryTrianglesDataKHR AccelerationStructureGeometryTrianglesData = GetAccelerationStructureGeometryTrianglesData(VertexBuffer, IndexBuffer, MaxVertices);
    VkAccelerationStructureGeometryKHR AccelerationStructureGeometry = GetAccelerationStructureGeometry(AccelerationStructureGeometryTrianglesData);
    VkAccelerationStructureBuildGeometryInfoKHR AccelerationStructureBuildGeometryInfo = GetAccelerationStructureBuildGeometryInfo(AccelerationStructureGeometry, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);
    VkAccelerationStructureBuildRangeInfoKHR AccelerationStructureBuildRangeInfo = GetAccelerationStructureBuildRangeInfo(MaxVertices/3);

    const VkAccelerationStructureBuildRangeInfoKHR*  VkAccelerationStructureBuildRangeInfoKHRPtr= &AccelerationStructureBuildRangeInfo;

    uint32_t MaxPrimitiveCount = AccelerationStructureBuildRangeInfo.primitiveCount;

    VkAccelerationStructureBuildSizesInfoKHR AccelerationStructureBuildSizesInfo{};
    AccelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

    vkGetAccelerationStructureBuildSizesKHR(LogicalDevice, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &AccelerationStructureBuildGeometryInfo, &MaxPrimitiveCount, &AccelerationStructureBuildSizesInfo);

    FAccelerationStructure NotCompactedBLAS = CreateAccelerationStructure(AccelerationStructureBuildSizesInfo.accelerationStructureSize, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);

    FBuffer ScratchBuffer = ResourceAllocator->CreateBuffer(AccelerationStructureBuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    auto ScratchAddress = GetBufferDeviceAddressInfo(ScratchBuffer);

    AccelerationStructureBuildGeometryInfo.dstAccelerationStructure = NotCompactedBLAS.AccelerationStructure;
    AccelerationStructureBuildGeometryInfo.scratchData.deviceAddress = ScratchAddress;

    VkQueryPool QueryPool{};
    VkQueryPoolCreateInfo QueryPoolCreateInfo{};
    QueryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    QueryPoolCreateInfo.queryCount = 1;
    QueryPoolCreateInfo.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR;
    vkCreateQueryPool(LogicalDevice, &QueryPoolCreateInfo, nullptr, &QueryPool);

    vkResetQueryPool(LogicalDevice, QueryPool, 0, 1);

    CommandBufferManager->RunSingletimeCommand([&, this](VkCommandBuffer CommandBuffer)
                                               {
                                                   vkCmdBuildAccelerationStructuresKHR(CommandBuffer, 1, &AccelerationStructureBuildGeometryInfo, &VkAccelerationStructureBuildRangeInfoKHRPtr);

                                                   VkMemoryBarrier Barrier{};
                                                   Barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                                                   Barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
                                                   Barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
                                                   vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                                                                        0, 1, &Barrier, 0, nullptr, 0, nullptr);

                                                   vkCmdWriteAccelerationStructuresPropertiesKHR(CommandBuffer, 1, &AccelerationStructureBuildGeometryInfo.dstAccelerationStructure, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, QueryPool, 0);
                                               });

    FAccelerationStructure CompactedBLAS = CreateAccelerationStructure(AccelerationStructureBuildSizesInfo.accelerationStructureSize, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);

    VkDeviceSize CompactedSize = 0;
    vkGetQueryPoolResults(LogicalDevice, QueryPool, 0, 1, sizeof(VkDeviceSize), &CompactedSize, sizeof(VkDeviceSize), VK_QUERY_RESULT_WAIT_BIT);

    CommandBufferManager->RunSingletimeCommand([&, this](VkCommandBuffer CommandBuffer)
                                               {
                                                   VkCopyAccelerationStructureInfoKHR CopyAccelerationStructureInfo{};
                                                   CopyAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR;
                                                   CopyAccelerationStructureInfo.src = NotCompactedBLAS.AccelerationStructure;
                                                   CopyAccelerationStructureInfo.dst = CompactedBLAS.AccelerationStructure;
                                                   CopyAccelerationStructureInfo.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR;

                                                   vkCmdCopyAccelerationStructureKHR(CommandBuffer, &CopyAccelerationStructureInfo);
                                               });

    DestroyAccelerationStructure(NotCompactedBLAS);

    std::cout << "Delta in size: " << AccelerationStructureBuildSizesInfo.accelerationStructureSize - CompactedSize
              << ", not compacted size is: "<< AccelerationStructureBuildSizesInfo.accelerationStructureSize<< ", compacted: " << CompactedSize << "." << std::endl;

    vkDestroyQueryPool(LogicalDevice, QueryPool, nullptr);

    return CompactedBLAS;
}

FAccelerationStructure FVulkanContext::GenerateTlas(std::vector<FAccelerationStructure> BLASes, std::vector<FMatrix4> TransformMatrices, std::vector<uint32_t> BlasIndices)
{
    uint32_t BLASCount = BLASes.size();

    std::vector<VkAccelerationStructureInstanceKHR> AccelerationStructureInstanceVector;
    AccelerationStructureInstanceVector.reserve(BLASes.size());
    for (int i = 0; i < BLASCount; ++i)
    {
        VkAccelerationStructureInstanceKHR BlasInstance{};
        auto& T = TransformMatrices[i].Data;
        BlasInstance.transform = {T[0].X, T[0].Y, T[0].Z, T[0].W,
                                  T[1].X, T[1].Y, T[1].Z, T[1].W,
                                  T[2].X, T[2].Y, T[2].Z, T[2].W,};
        BlasInstance.instanceCustomIndex = BlasIndices[i];
        BlasInstance.accelerationStructureReference = GetASDeviceAddressInfo(BLASes[i]);
        BlasInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        BlasInstance.mask = 0xFF;
        BlasInstance.instanceShaderBindingTableRecordOffset = 0;
        AccelerationStructureInstanceVector.emplace_back(BlasInstance);
    }

    auto BlasInstanceBuffer = ResourceAllocator->CreateBuffer(AccelerationStructureInstanceVector.size() * sizeof(VkAccelerationStructureInstanceKHR), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    auto BlasInstanceBufferAddress = GetBufferDeviceAddressInfo(BlasInstanceBuffer);

    FAccelerationStructure TLAS;
    FBuffer ScratchBuffer;

    CommandBufferManager->RunSingletimeCommand([&, this](VkCommandBuffer CommandBuffer)
    {
        VkMemoryBarrier MemoryBarrier{};
        MemoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        MemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        MemoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1,
                             &MemoryBarrier, 0, nullptr, 0, nullptr);
        VkAccelerationStructureGeometryInstancesDataKHR AccelerationStructureGeometryInstancesData{};
        AccelerationStructureGeometryInstancesData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
        AccelerationStructureGeometryInstancesData.data.deviceAddress = BlasInstanceBufferAddress;
        VkAccelerationStructureGeometryKHR AccelerationStructureGeometry{};
        AccelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        AccelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        AccelerationStructureGeometry.geometry.instances = AccelerationStructureGeometryInstancesData;
        VkAccelerationStructureBuildGeometryInfoKHR AccelerationStructureBuildGeometry{};
        AccelerationStructureBuildGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        AccelerationStructureBuildGeometry.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        AccelerationStructureBuildGeometry.geometryCount = 1;
        AccelerationStructureBuildGeometry.pGeometries = &AccelerationStructureGeometry;
        AccelerationStructureBuildGeometry.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        AccelerationStructureBuildGeometry.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        AccelerationStructureBuildGeometry.srcAccelerationStructure = VK_NULL_HANDLE;
        uint32_t CountInstances = BLASCount;
        VkAccelerationStructureBuildSizesInfoKHR AccelerationStructureBuildSizesInfo{};
        AccelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
        vkGetAccelerationStructureBuildSizesKHR(LogicalDevice, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &AccelerationStructureBuildGeometry, &CountInstances, &AccelerationStructureBuildSizesInfo);
        TLAS = CreateAccelerationStructure(AccelerationStructureBuildSizesInfo.accelerationStructureSize, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);
        ScratchBuffer = ResourceAllocator->CreateBuffer(AccelerationStructureBuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        AccelerationStructureBuildGeometry.srcAccelerationStructure = TLAS.AccelerationStructure;
        AccelerationStructureBuildGeometry.dstAccelerationStructure = TLAS.AccelerationStructure;
        AccelerationStructureBuildGeometry.scratchData.deviceAddress = GetBufferDeviceAddressInfo(ScratchBuffer);
        VkAccelerationStructureBuildRangeInfoKHR AccelerationStructureBuildRangeInfo{};
        AccelerationStructureBuildRangeInfo.primitiveCount = CountInstances;
        const VkAccelerationStructureBuildRangeInfoKHR* AccelerationStructureBuildRangeInfoPtr = &AccelerationStructureBuildRangeInfo;
        vkCmdBuildAccelerationStructuresKHR(CommandBuffer, 1, &AccelerationStructureBuildGeometry, &AccelerationStructureBuildRangeInfoPtr);
    });

    ResourceAllocator->DestroyBuffer(BlasInstanceBuffer);
    ResourceAllocator->DestroyBuffer(ScratchBuffer);

    return TLAS;
}

VkPhysicalDeviceRayTracingPipelinePropertiesKHR FVulkanContext::GetRTProperties()
{
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR PhysicalDeviceRayTracingPipelinePropertiesKHR{};
    PhysicalDeviceRayTracingPipelinePropertiesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
    VkPhysicalDeviceProperties2 PhysicalDeviceProperties2 = GetPhysicalDeviceProperties2(PhysicalDevice, &PhysicalDeviceRayTracingPipelinePropertiesKHR);
    return PhysicalDeviceRayTracingPipelinePropertiesKHR;
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

    VkPhysicalDeviceFeatures2 DeviceFeatures2{};
    DeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    DeviceFeatures2.features.samplerAnisotropy = VK_TRUE;
    DeviceFeatures2.features.sampleRateShading = VK_TRUE;

    VulkanContextOptions.BuildDevicePNextChain(reinterpret_cast<BaseVulkanStructure*>(&DeviceFeatures2));

    VkDeviceCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    CreateInfo.pNext = &DeviceFeatures2;
    CreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
    CreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());

    auto DeviceExtensions = VulkanContextOptions.GetDeviceExtensionsList();
    CreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
    CreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();

    auto DeviceLayers = VulkanContextOptions.GetDeviceLayers();
    CreateInfo.enabledLayerCount = static_cast<uint32_t>(DeviceLayers.size());
    CreateInfo.ppEnabledLayerNames = (CreateInfo.enabledLayerCount == 0) ? nullptr : DeviceLayers.data();

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

//    ImageManager->CreateImage(ColorImage, Width, Height, false, MSAASamples, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
//                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//                              VK_IMAGE_ASPECT_COLOR_BIT);
//    V::SetName(LogicalDevice, (*ImageManager)(ColorImage).Image, "V_ColorImage");
//    V::SetName(LogicalDevice, (*ImageManager)(ColorImage).View, "V_ColorImagView");

//    ImageManager->CreateImage(NormalsImage, Width, Height, false, MSAASamples, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
//                                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//                                            VK_IMAGE_ASPECT_COLOR_BIT);
//    V::SetName(LogicalDevice, (*ImageManager)(NormalsImage).Image, "V_NormalsImage");
//    V::SetName(LogicalDevice, (*ImageManager)(NormalsImage).View, "V_NormalsImageView");

//    ImageManager->CreateImage(RenderableIndexImage, Width, Height, false, MSAASamples, VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL,
//                                                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//                                                    VK_IMAGE_ASPECT_COLOR_BIT);
//    V::SetName(LogicalDevice, (*ImageManager)(RenderableIndexImage).Image, "V_RenderableIndexImage");
//    V::SetName(LogicalDevice, (*ImageManager)(RenderableIndexImage).View, "V_RenderableIndexImageView");

//    ImageManager->CreateImage(UtilityImageR32, Width, Height, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL,
//                                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//                                               VK_IMAGE_ASPECT_COLOR_BIT);
//    V::SetName(LogicalDevice, (*ImageManager)(UtilityImageR32).Image, "V_UtilityImageR32");
//    V::SetName(LogicalDevice, (*ImageManager)(UtilityImageR32).View, "V_UtilityImageR32View");
//
//    auto& UtilityImgR32 = (*ImageManager)(UtilityImageR32);
//    UtilityImgR32.Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    ImageManager->CreateImage(ResolvedColorImage, Width, Height, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              VK_IMAGE_ASPECT_COLOR_BIT);
    V::SetName(LogicalDevice, (*ImageManager)(ResolvedColorImage).Image, "V_ResolvedColorImage");
    V::SetName(LogicalDevice, (*ImageManager)(ResolvedColorImage).View, "V_ResolvedColorImageView");

//    ImageManager->CreateImage(UtilityImageR8G8B8A8_SRGB, Width, Height, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
//                                                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//                                                         VK_IMAGE_ASPECT_COLOR_BIT);
//    V::SetName(LogicalDevice, (*ImageManager)(UtilityImageR8G8B8A8_SRGB).Image, "V_UtilityImageR8G8B8A8_SRGB");
//    V::SetName(LogicalDevice, (*ImageManager)(UtilityImageR8G8B8A8_SRGB).View, "V_UtilityImageR8G8B8A8_SRGBView");

//    /// Create Image and ImageView for Depth
//    VkFormat DepthFormat = FindDepthFormat();
//    ImageManager->CreateImage(DepthImage, Width, Height, false, MSAASamples, DepthFormat, VK_IMAGE_TILING_OPTIMAL,
//                                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//                                          VK_IMAGE_ASPECT_DEPTH_BIT);
//
//
//    V::SetName(LogicalDevice, (*ImageManager)(DepthImage).Image, "V_DepthImage");
//    V::SetName(LogicalDevice, (*ImageManager)(DepthImage).View, "V_DepthImageView");
//    auto& DepthImg = (*ImageManager)(DepthImage);
//    DepthImg.Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
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
//    RenderPass = std::make_shared<FRenderPass>();
//    RenderPass->AddImageAsAttachment((*ImageManager)(ColorImage), AttachmentType::Color, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
//    RenderPass->AddImageAsAttachment((*ImageManager)(NormalsImage), AttachmentType::Color, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
//    RenderPass->AddImageAsAttachment((*ImageManager)(RenderableIndexImage), AttachmentType::Color, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
//    RenderPass->AddImageAsAttachment((*ImageManager)(DepthImage), AttachmentType::DepthStencil, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
//    RenderPass->AddImageAsAttachment((*ImageManager)(ResolvedColorImage), AttachmentType::Resolve, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
//
//    RenderPass->Construct(LogicalDevice);
//    V::SetName(LogicalDevice, RenderPass->RenderPass, "V_RenderRenderpass");
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

void FVulkanContext::CreateRTDescriptorSetLayouts()
{
    DescriptorSetManager->AddDescriptorLayout(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME, 0, LAYOUTS::TLAS_LAYOUT_NAME,
                                              {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME, 0, LAYOUTS::RT_OUT_IMAGE_LAYOUT_NAME,
                                              {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR});

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
//    GraphicsPipeline.AddShader("../shaders/triangle_vert.spv", eShaderType::VERTEX);
//    GraphicsPipeline.AddShader("../shaders/triangle_frag.spv", eShaderType::FRAGMENT);
//
//    auto AttributeDescriptions = FVertex::GetAttributeDescriptions();
//    for (auto& Entry : AttributeDescriptions)
//    {
//        GraphicsPipeline.AddVertexInputAttributeDescription(Entry);
//    }
//    GraphicsPipeline.AddVertexInputBindingDescription(FVertex::GetBindingDescription());
//
//    GraphicsPipeline.SetMSAA(Context.MSAASamples);
//    GraphicsPipeline.SetExtent2D(Swapchain->GetExtent2D());
//    GraphicsPipeline.SetWidth(Swapchain->GetWidth());
//    GraphicsPipeline.SetHeight(Swapchain->GetHeight());
//    GraphicsPipeline.SetBlendAttachmentsCount(3);
//    GraphicsPipeline.AddDescriptorSetLayout(DescriptorSetManager->GetVkDescriptorSetLayout(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME));
//    GraphicsPipeline.AddDescriptorSetLayout(DescriptorSetManager->GetVkDescriptorSetLayout(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME));
//    GraphicsPipeline.CreateGraphicsPipeline(LogicalDevice, RenderPass->RenderPass);
}

void FVulkanContext::CreateRTPipeline()
{
    RTPipeline.AddShader("../shaders/ray_gen.spv", eShaderType::RAYGEN);
    RTPipeline.AddShader("../shaders/ray_closest_hit.spv", eShaderType::RAYCLOSEHIT);
    RTPipeline.AddShader("../shaders/ray_miss.spv", eShaderType::RAYMISS);

    RTPipeline.SetExtent2D(Swapchain->GetExtent2D());
    RTPipeline.SetWidth(Swapchain->GetWidth());
    RTPipeline.SetHeight(Swapchain->GetHeight());

    RTPipeline.AddDescriptorSetLayout(DescriptorSetManager->GetVkDescriptorSetLayout(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME));

    RTPipeline.CreateRayTracingPipeline(LogicalDevice);
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
//    SwapChainFramebuffers.resize(Swapchain->Size());
//    for (std::size_t i = 0; i < Swapchain->Size(); ++i) {
//        std::vector<VkImageView> Attachments = {(*ImageManager)(ColorImage).View, (*ImageManager)(NormalsImage).View, (*ImageManager)(RenderableIndexImage).View,
//                                                (*ImageManager)(DepthImage).View, (*ImageManager)(ResolvedColorImage).View};
//
//        VkFramebufferCreateInfo FramebufferCreateInfo{};
//        FramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//        FramebufferCreateInfo.renderPass = RenderPass->RenderPass;
//        FramebufferCreateInfo.attachmentCount = static_cast<uint32_t>(Attachments.size());
//        FramebufferCreateInfo.pAttachments = Attachments.data();
//        FramebufferCreateInfo.width = Swapchain->GetWidth();
//        FramebufferCreateInfo.height = Swapchain->GetHeight();
//        FramebufferCreateInfo.layers = 1;
//
//        if (vkCreateFramebuffer(LogicalDevice, &FramebufferCreateInfo, nullptr, &SwapChainFramebuffers[i]) != VK_SUCCESS)
//        {
//            throw std::runtime_error("Failed to create framebuffer!");
//        }
//
//        V::SetName(LogicalDevice, SwapChainFramebuffers[i], "V_Render_fb_" + std::to_string(i));
//    }
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
        FramebufferCreateInfo.renderPass = ImGuiRenderPass->RenderPass;
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

void FVulkanContext::CreateRTDescriptorPool()
{
    auto ModelsCount = ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMeshSystem>()->Size();
    auto NumberOfSwapChainImages = Swapchain->Size();

    DescriptorSetManager->AddDescriptorSet(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME, NumberOfSwapChainImages);

    DescriptorSetManager->ReserveDescriptorPool();
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
    V::SetName(LogicalDevice, ImGuiDescriptorPool, "V_ImGuiDescriptorPool");
}

void FVulkanContext::CreateDescriptorSet()
{
//    auto& Coordinator = ECS::GetCoordinator();
//    auto MeshSystem = Coordinator.GetSystem<ECS::SYSTEMS::FMeshSystem>();
//
//    /// Create descriptor sets
//    DescriptorSetManager->CreateAllDescriptorSets();
//
//    for (size_t i = 0; i < Swapchain->Size(); ++i)
//    {
//        uint32_t j = 0;
//        for (auto Mesh : *MeshSystem)
//        {
//            VkDescriptorBufferInfo TransformBufferInfo{};
//            TransformBufferInfo.buffer = DeviceTransformBuffers[i].Buffer;
//            TransformBufferInfo.offset = sizeof(ECS::COMPONENTS::FDeviceTransformComponent) * j;
//            TransformBufferInfo.range = sizeof(ECS::COMPONENTS::FDeviceTransformComponent);
//            DescriptorSetManager->UpdateDescriptorSetInfo(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME, LAYOUTS::TRANSFORM_LAYOUT_NAME, j * Swapchain->Size() + i, TransformBufferInfo);
//
//            VkDescriptorBufferInfo RenderableBufferInfo{};
//            RenderableBufferInfo.buffer = DeviceRenderableBuffers[i].Buffer;
//            RenderableBufferInfo.offset = sizeof(ECS::COMPONENTS::FDeviceRenderableComponent) * j;
//            RenderableBufferInfo.range = sizeof(ECS::COMPONENTS::FDeviceRenderableComponent);
//            DescriptorSetManager->UpdateDescriptorSetInfo(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME, LAYOUTS::RENDERABLE_LAYOUT_NAME, j * Swapchain->Size() + i, RenderableBufferInfo);
//
//            ++j;
//        }
//    }
//
//    for (size_t i = 0; i < Swapchain->Size(); ++i)
//    {
//
//        VkDescriptorBufferInfo CameraBufferInfo{};
//        CameraBufferInfo.buffer = DeviceCameraBuffers[i].Buffer;
//        CameraBufferInfo.offset = 0;
//        CameraBufferInfo.range = sizeof(ECS::COMPONENTS::FDeviceCameraComponent);
//        DescriptorSetManager->UpdateDescriptorSetInfo(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME, LAYOUTS::CAMERA_LAYOUT_NAME, i, CameraBufferInfo);
//
//        VkDescriptorImageInfo ImageBufferInfo{};
//        ImageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//        ImageBufferInfo.imageView = (*ImageManager)(TextureImage).View;
//        ImageBufferInfo.sampler = TextureSampler;
//        DescriptorSetManager->UpdateDescriptorSetInfo(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME, LAYOUTS::TEXTURE_SAMPLER_LAYOUT_NAME, i, ImageBufferInfo);
//    }
//
//    for (size_t i = 0; i < Swapchain->Size(); ++i)
//    {
//        VkDescriptorImageInfo ImageBufferInfo{};
//        ImageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//        ImageBufferInfo.imageView = (*ImageManager)(ResolvedColorImage).View;
//        ImageBufferInfo.sampler = TextureSampler;
//        DescriptorSetManager->UpdateDescriptorSetInfo(LAYOUT_SETS::PASSTHROUGH_LAYOUT_NAME, LAYOUTS::TEXTURE_SAMPLER_LAYOUT_NAME, i, ImageBufferInfo);
//    }
}

void FVulkanContext::CreateRTDescriptorSet()
{
    auto& Coordinator = ECS::GetCoordinator();
    auto MeshSystem = Coordinator.GetSystem<ECS::SYSTEMS::FMeshSystem>();

    /// Create descriptor sets
    DescriptorSetManager->CreateAllDescriptorSets();

    for (size_t i = 0; i < Swapchain->Size(); ++i)
    {

        DescriptorSetManager->UpdateDescriptorSetInfo(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME, LAYOUTS::TLAS_LAYOUT_NAME, i, TLAS.AccelerationStructure);

        VkDescriptorImageInfo ImageBufferInfo{};
        ImageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        ImageBufferInfo.imageView = (*ImageManager)(ResolvedColorImage).View;
        DescriptorSetManager->UpdateDescriptorSetInfo(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME, LAYOUTS::RT_OUT_IMAGE_LAYOUT_NAME, i, ImageBufferInfo);
    }
}

void FVulkanContext::CreateCommandBuffers()
{
//    GraphicsCommandBuffers.resize(SwapChainFramebuffers.size());
//
//    for (std::size_t i = 0; i < GraphicsCommandBuffers.size(); ++i)
//    {
//        GraphicsCommandBuffers[i] = CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
//        {
//            VkRenderPassBeginInfo RenderPassInfo{};
//            RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//            RenderPassInfo.renderPass = RenderPass->RenderPass;
//            RenderPassInfo.framebuffer = SwapChainFramebuffers[i];
//            RenderPassInfo.renderArea.offset = {0, 0};
//            RenderPassInfo.renderArea.extent = Swapchain->GetExtent2D();
//
//            std::vector<VkClearValue> ClearValues{7};
//            ClearValues[0].color = {0.f, 0.f, 0.f, 1.f};
//            ClearValues[1].color = {0.0f, 0.f, 0.f, 1.f};
//            ClearValues[2].color = {0, 0, 0, 0};
//            ClearValues[3].depthStencil = {1.f, 0};
//            ClearValues[4].color = {0.f, 0.f, 0.f, 0.f};
//            RenderPassInfo.clearValueCount = static_cast<uint32_t>(ClearValues.size());
//            RenderPassInfo.pClearValues = ClearValues.data();
//
//            vkCmdBeginRenderPass(CommandBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
//            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline.GetPipeline());
//
//            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline.GetPipelineLayout(), 0, 1, &DescriptorSetManager->GetSet(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME, i), 0,
//                                    nullptr);
//            auto& Coordinator = ECS::GetCoordinator();
//            auto MeshSystem = Coordinator.GetSystem<ECS::SYSTEMS::FMeshSystem>();
//
//            uint32_t j = 0;
//            for (auto Entity : *MeshSystem)
//            {
//                vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline.GetPipelineLayout(), 1, 1,
//                                        &DescriptorSetManager->GetSet(LAYOUT_SETS::PER_RENDERABLE_LAYOUT_NAME, j * Swapchain->Size() + i), 0,
//                                        nullptr);
//
//                MeshSystem->Bind(Entity, CommandBuffer);
//                MeshSystem->Draw(Entity, CommandBuffer);
//                ++j;
//            }
//            vkCmdEndRenderPass(CommandBuffer);
//        });
//
//        V::SetName(LogicalDevice, GraphicsCommandBuffers[i], "V_GraphicsCommandBuffers" + std::to_string(i));
//    }

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

        V::SetName(LogicalDevice, PassthroughCommandBuffers[i], "V_PassthroughCommandBuffers" + std::to_string(i));
    }
}

void FVulkanContext::CreateRTCommandBuffers()
{
    RTCommandBuffers.resize(Swapchain->Size());

    for (std::size_t i = 0; i <RTCommandBuffers.size(); ++i)
    {
        RTCommandBuffers[i] = CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, RTPipeline.GetPipeline());
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, RTPipeline.GetPipelineLayout(), 0, 1, &DescriptorSetManager->GetSet(LAYOUT_SETS::PER_FRAME_LAYOUT_NAME, i), 0, nullptr);
            vkCmdTraceRaysKHR(CommandBuffer, &RGenRegion, &RMissRegion, &RHitRegion, &RCallRegion, 1920, 1080, 1);
        });
    }
}

void FVulkanContext::CreateSyncObjects()
{
    ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    RTFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
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
            vkCreateSemaphore(LogicalDevice, &SemaphoreInfo, nullptr, &RTFinishedSemaphores[i]) != VK_SUCCESS ||
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
    SubmitInfo.pCommandBuffers = &RTCommandBuffers[ImageIndex];

    VkSemaphore SignalSemaphores[] = {RTFinishedSemaphores[CurrentFrame]};
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

    VkSemaphore PassthroughWaitSemaphores[] = {RTFinishedSemaphores[CurrentFrame]};
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

    Swapchain = std::make_shared<FSwapchain>(*this, PhysicalDevice, LogicalDevice, Surface, Window, GraphicsQueueIndex, PresentQueueIndex, VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_PRESENT_MODE_MAILBOX_KHR);
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
//    ImageManager->RemoveImage(ColorImage);
    ImageManager->RemoveImage(ResolvedColorImage);
//    ImageManager->RemoveImage(UtilityImageR8G8B8A8_SRGB);
//    ImageManager->RemoveImage(NormalsImage);
//    ImageManager->RemoveImage(RenderableIndexImage);
//    ImageManager->RemoveImage(UtilityImageR32);
//    ImageManager->RemoveImage(DepthImage);

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
    for (auto& CommandBuffer : RTCommandBuffers)
    {
        CommandBufferManager->FreeCommandBuffer(CommandBuffer);
    }

    for (auto& CommandBuffer : PassthroughCommandBuffers)
    {
        CommandBufferManager->FreeCommandBuffer(CommandBuffer);
    }

    /// Remove pipelines
    //GraphicsPipeline.Delete();
    PassthroughPipeline.Delete();

    /// Remove renderpasses
    //RenderPass = nullptr;
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
    /// Free all device buffers
    for (size_t i = 0; i < Swapchain->Size(); ++i)
    {
        ResourceAllocator->DestroyBuffer(DeviceTransformBuffers[i]);
        ResourceAllocator->DestroyBuffer(DeviceCameraBuffers[i]);
        ResourceAllocator->DestroyBuffer(DeviceRenderableBuffers[i]);
    }

    CleanUpSwapChain();

    vkDestroyDescriptorPool(LogicalDevice, ImGuiDescriptorPool, nullptr);

    vkDestroySampler(LogicalDevice, TextureSampler, nullptr);
//    ImageManager->RemoveImage(TextureImage);

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
        vkDestroySemaphore(LogicalDevice, RTFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(LogicalDevice, ImageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(LogicalDevice, PassthroughFinishedSemaphore[i], nullptr);
        vkDestroySemaphore(LogicalDevice, ImGuiFinishedSemaphores[i], nullptr);
        vkDestroyFence(LogicalDevice, RenderingFinishedFences[i], nullptr);
        vkDestroyFence(LogicalDevice, ImGuiFinishedFences[i], nullptr);
    }

    CommandBufferManager = nullptr;
    vkDestroyDevice(LogicalDevice, nullptr);


    DestroyDebugUtilsMessengerEXT();

    vkDestroySurfaceKHR(Instance, Surface, nullptr);
    vkDestroyInstance(Instance, nullptr);
}

void FVulkanContext::CreateBLAS()
{
    auto MeshSystem = ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMeshSystem>();

    for(auto Mesh : *MeshSystem)
    {
        auto MeshComponent = ECS::GetCoordinator().GetComponent<ECS::COMPONENTS::FMeshComponent>(Mesh);
        auto DeviceMeshComponent = ECS::GetCoordinator().GetComponent<ECS::COMPONENTS::FDeviceMeshComponent>(Mesh);
        BLASVector.emplace_back(GenerateBlas(DeviceMeshComponent.VertexBuffer, DeviceMeshComponent.IndexBuffer, MeshComponent.Vertices.size()));
    }
}

void FVulkanContext::CreateTLAS()
{
    auto MeshSystem = ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMeshSystem>();

    std::vector<FMatrix4> Transforms;
    std::vector<uint32_t> BlasIndices;
    int i = 0;

    for(auto Mesh : *MeshSystem)
    {
        Transforms.push_back(ECS::GetCoordinator().GetComponent<ECS::COMPONENTS::FDeviceTransformComponent>(Mesh).ModelMatrix);
        BlasIndices.push_back(i++);
    }

    TLAS = GenerateTlas(BLASVector, Transforms, BlasIndices);

}

void FVulkanContext::CreateSBT()
{
    auto RTProperties = GetRTProperties();

    uint32_t MissCount = 1;
    uint32_t HitCount = 1;
    auto HandleCount = 1 + MissCount + HitCount;
    uint32_t HandleSize = RTProperties.shaderGroupHandleSize;

    auto AlignUp = [](uint32_t X, uint32_t A)-> uint32_t
    {
        return (X + (A - 1)) & ~(A - 1);
    };

    uint32_t HandleSizeAligned = AlignUp(HandleSize, RTProperties.shaderGroupHandleAlignment);

    RGenRegion.stride = AlignUp(HandleSize, RTProperties.shaderGroupBaseAlignment);
    RGenRegion.size = RGenRegion.stride;

    RMissRegion.stride = HandleSizeAligned;
    RMissRegion.size = AlignUp(MissCount * HandleSizeAligned, RTProperties.shaderGroupBaseAlignment);

    RHitRegion.stride = HandleSizeAligned;
    RHitRegion.size = AlignUp(HitCount * HandleSizeAligned, RTProperties.shaderGroupBaseAlignment);

    uint32_t DataSize = HandleCount * HandleSize;
    std::vector<uint8_t> Handles(DataSize);

    auto Result = vkGetRayTracingShaderGroupHandlesKHR(LogicalDevice, RTPipeline.GetPipeline(), 0, HandleCount, DataSize, Handles.data());
    assert(Result == VK_SUCCESS && "Failed to get handles for SBT");

    VkDeviceSize SBTSize = RGenRegion.size + RMissRegion.size + RHitRegion.size;
    auto SBTBuffer = ResourceAllocator->CreateBuffer(SBTSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    auto SBTBufferAddress = GetBufferDeviceAddressInfo(SBTBuffer);
    RGenRegion.deviceAddress = SBTBufferAddress;
    RMissRegion.deviceAddress = RGenRegion.deviceAddress + RGenRegion.size;
    RHitRegion.deviceAddress = RMissRegion.deviceAddress + RMissRegion.size;

    auto GetHandle = [&](int i)
    {
        return Handles.data() + i * HandleSize;
    };

    auto* SBTBufferPtr = reinterpret_cast<uint8_t*>(ResourceAllocator->Map(SBTBuffer));
    uint8_t* DataPtr{nullptr};
    uint32_t HandleIndex{0};

    DataPtr = SBTBufferPtr;
    memcpy(DataPtr, GetHandle(HandleIndex++), HandleSize);

    DataPtr = SBTBufferPtr + RGenRegion.size;
    memcpy(DataPtr, GetHandle(HandleIndex++), HandleSize);

    DataPtr = SBTBufferPtr + RGenRegion.size + RMissRegion.size;
    memcpy(DataPtr, GetHandle(HandleIndex++), HandleSize);

    ResourceAllocator->Unmap(SBTBuffer);
}


FVulkanContext::~FVulkanContext()
{
}
