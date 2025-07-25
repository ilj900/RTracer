#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

#include "texture_manager.h"

#include "utils.h"

#include <algorithm>
#include <iostream>
#include <random>
#include <set>
#include <stdexcept>
#include <unordered_map>

#include "stb_image.h"
#include "stb_image_write.h"

#include "tinyexr.h"

FVulkanContext* VulkanContext = nullptr;
std::function<VkSurfaceKHR(VkInstance)> FVulkanContext::SurfaceCreationFunction = nullptr;

void FVulkanContext::SetSurfaceCreationFunction(std::function<VkSurfaceKHR(VkInstance)> SurfaceCreationFunctionIn)
{
	SurfaceCreationFunction = SurfaceCreationFunctionIn;
}

FVulkanContext* GetVulkanContext(const std::vector<std::string>& AdditionalDeviceExtensions)
{
	if (VulkanContext == nullptr)
	{
		VulkanContext = new FVulkanContext(AdditionalDeviceExtensions);
	}

	return VulkanContext;
}

void FreeVulkanContext()
{
	if (VulkanContext != nullptr)
	{
		delete VulkanContext;
		VulkanContext = nullptr;
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL FVulkanContext::DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT MessageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallBackData,
        void* pUserData)
{
    std::unordered_map<uint32_t, std::string> SeverityFlagToString =
            {
                    {VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT, "VERBOSE"},
                    {VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT, "INFO"},
                    {VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT, "WARNING"},
                    {VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "ERROR"},
                    {VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT, "SHOULD NEVER BE PRINTED"}
            };
    std::cerr << "Validation layer " << SeverityFlagToString[MessageSeverity] << ": " << pCallBackData->pMessage << '\n' << std::endl;

    return VK_FALSE;
}

FVulkanContext::FVulkanContext(const std::vector<std::string>& AdditionalDeviceExtensions)
{
    /// Fill in vulkan context creation options
    FVulkanContextOptions VulkanContextOptions;
    //VulkanContextOptions.AddInstanceLayer("VK_LAYER_KHRONOS_validation");

#ifndef NDEBUG
    VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo = {};
    DebugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    DebugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    DebugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    DebugCreateInfo.pfnUserCallback = DebugCallback;
    VulkanContextOptions.AddInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, &DebugCreateInfo, sizeof(VkDebugUtilsMessengerCreateInfoEXT));
#endif

    for (const auto & AdditionalDeviceExtension : AdditionalDeviceExtensions)
    {
        VulkanContextOptions.AddInstanceExtension(AdditionalDeviceExtension);
    }

    VkPhysicalDeviceAccelerationStructureFeaturesKHR PhysicalDeviceAccelerationStructureFeatures{};
    PhysicalDeviceAccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    PhysicalDeviceAccelerationStructureFeatures.accelerationStructure = VK_TRUE;
    VulkanContextOptions.AddDeviceExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, &PhysicalDeviceAccelerationStructureFeatures, sizeof(VkPhysicalDeviceAccelerationStructureFeaturesKHR));

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR PhysicalDeviceRayTracingPipelineFeatures{};
    PhysicalDeviceRayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    PhysicalDeviceRayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
	PhysicalDeviceRayTracingPipelineFeatures.rayTracingPipelineTraceRaysIndirect = VK_TRUE;
    VulkanContextOptions.AddDeviceExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, &PhysicalDeviceRayTracingPipelineFeatures, sizeof(VkPhysicalDeviceRayTracingPipelineFeaturesKHR));

    VkPhysicalDeviceBufferDeviceAddressFeatures PhysicalDeviceBufferDeviceAddressFeatures{};
    PhysicalDeviceBufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    PhysicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
    VulkanContextOptions.AddDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, &PhysicalDeviceBufferDeviceAddressFeatures, sizeof(VkPhysicalDeviceBufferDeviceAddressFeatures));

    VkPhysicalDeviceHostQueryResetFeatures PhysicalDeviceHostQueryResetFeatures{};
    PhysicalDeviceHostQueryResetFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES;
    PhysicalDeviceHostQueryResetFeatures.hostQueryReset = VK_TRUE;
    VulkanContextOptions.AddDeviceExtension(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME, &PhysicalDeviceHostQueryResetFeatures, sizeof(VkPhysicalDeviceHostQueryResetFeatures));

    VkPhysicalDeviceMaintenance4Features PhysicalDeviceMaintenance4Features{};
    PhysicalDeviceMaintenance4Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES;
    PhysicalDeviceMaintenance4Features.maintenance4 = VK_TRUE;
    VulkanContextOptions.AddDeviceExtension(VK_KHR_MAINTENANCE_4_EXTENSION_NAME, &PhysicalDeviceMaintenance4Features, sizeof(PhysicalDeviceMaintenance4Features));

    VulkanContextOptions.AddDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    VulkanContextOptions.AddDeviceExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);

	if (SurfaceCreationFunction)
	{
		VulkanContextOptions.AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}
    /// Create Vulkan instance
    Instance = CreateVkInstance("Hello Triangle", {1, 3, 0}, "No Engine", {1, 3, 0}, VK_API_VERSION_1_3, VulkanContextOptions);

    /// Load Vulkan options
    V::LoadVkFunctions(Instance);

#ifndef NDEBUG
    /// Create debug messenger
    DebugMessenger = CreateDebugMessenger(VulkanContextOptions);
#endif
    /// Create Surface
    if (SurfaceCreationFunction != nullptr)
	{
		Surface = SurfaceCreationFunction(Instance);
	}

    /// Pick Physical device
    PhysicalDevice = PickPhysicalDevice(VulkanContextOptions, Surface);

    /// Create Logical device
    LogicalDevice = CreateLogicalDevice(PhysicalDevice, VulkanContextOptions);

    GetDeviceQueues(Surface);
}

#ifndef NDEBUG
VkDebugUtilsMessengerEXT FVulkanContext::CreateDebugMessenger(FVulkanContextOptions& VulkanContextOptions) const
{
    VkDebugUtilsMessengerEXT DebugUtilsMessengerEXT;
    auto* DebugCreateInfo = VulkanContextOptions.GetInstanceExtensionStructurePtr<VkDebugUtilsMessengerCreateInfoEXT>(VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT);

    V::vkCreateDebugUtilsMessengerEXT(Instance, DebugCreateInfo, nullptr, &DebugUtilsMessengerEXT);

    return DebugUtilsMessengerEXT;
}

void FVulkanContext::SetDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT DebugUtilsMessengerEXT)
{
    this->DebugMessenger = DebugUtilsMessengerEXT;
}
#endif

void FVulkanContext::DestroySurface(VkSurfaceKHR* SurfaceIn) const
{
    vkDestroySurfaceKHR(Instance, *SurfaceIn, nullptr);
    *SurfaceIn = VK_NULL_HANDLE;
}

void FVulkanContext::SetSurface(VkSurfaceKHR SurfaceIn)
{
    this->Surface = SurfaceIn;
}

VkSurfaceKHR FVulkanContext::GetSurface() const
{
    return Surface;
}

std::vector<VkPhysicalDevice> FVulkanContext::EnumerateAllPhysicalDevices(VkInstance InstanceIn)
{
    uint32_t DeviceCount = 0;
    vkEnumeratePhysicalDevices(InstanceIn, &DeviceCount, nullptr);

    if (DeviceCount == 0)
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> Devices(DeviceCount);
    vkEnumeratePhysicalDevices(InstanceIn, &DeviceCount, Devices.data());

    return Devices;
}

VkPhysicalDevice FVulkanContext::PickPhysicalDevice(FVulkanContextOptions& VulkanContextOptions, VkSurfaceKHR SurfaceIn)
{
    auto Devices = EnumerateAllPhysicalDevices(Instance);

    auto DeviceExtensionList = VulkanContextOptions.GetDeviceExtensionsList();

    std::set<std::string> RequiredExtensions (DeviceExtensionList.begin(), DeviceExtensionList.end());

    for (const auto& Device : Devices)
    {
        if (CheckDeviceExtensionsSupport(Device, RequiredExtensions) && (CheckDeviceQueueSupport(Device, SurfaceIn)))
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

void FVulkanContext::SetPhysicalDevice(VkPhysicalDevice PhysicalDeviceIn)
{
    this->PhysicalDevice = PhysicalDeviceIn;
}

VkPhysicalDevice FVulkanContext::GetPhysicalDevice() const
{
    return PhysicalDevice;
}

VkPhysicalDeviceProperties2 FVulkanContext::GetPhysicalDeviceProperties2(VkPhysicalDevice PhysicalDevice, void* pNextStructure)
{
    VkPhysicalDeviceProperties2 PhysicalDeviceProperties2{};
    PhysicalDeviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    PhysicalDeviceProperties2.pNext = pNextStructure;

    vkGetPhysicalDeviceProperties2(PhysicalDevice, &PhysicalDeviceProperties2);

    return PhysicalDeviceProperties2;
}

VkPhysicalDeviceRayTracingPipelinePropertiesKHR FVulkanContext::GetRTProperties()
{
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR PhysicalDeviceRayTracingPipelinePropertiesKHR{};
    PhysicalDeviceRayTracingPipelinePropertiesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
    VkPhysicalDeviceProperties2 PhysicalDeviceProperties2 = GetPhysicalDeviceProperties2(PhysicalDevice, &PhysicalDeviceRayTracingPipelinePropertiesKHR);
    return PhysicalDeviceRayTracingPipelinePropertiesKHR;
}

void FVulkanContext::InitManagerResources()
{
    RESOURCE_ALLOCATOR();

    DescriptorSetManager = std::make_shared<FDescriptorSetManager>(LogicalDevice);

    COMMAND_BUFFER_MANAGER();
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

    TimestampPeriod = PhysicalDeviceProperties.limits.timestampPeriod;
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

void FVulkanContext::SetInstance(VkInstance InstanceIn)
{
    this->Instance = InstanceIn;
}

VkInstance FVulkanContext::GetInstance() const
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

bool FVulkanContext::CheckDeviceQueueSupport(VkPhysicalDevice PhysicalDeviceIn, VkQueueFlagBits QueueFlagBits, uint32_t& QueueFamilyIndex)
{
    auto Properties = EnumeratePhysicalDeviceQueueFamilyProperties(PhysicalDeviceIn);

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

bool FVulkanContext::CheckDeviceQueuePresentSupport(VkPhysicalDevice PhysicalDeviceIn, uint32_t& QueueFamilyIndex, VkSurfaceKHR SurfaceIn)
{
    auto Properties = EnumeratePhysicalDeviceQueueFamilyProperties(PhysicalDeviceIn);

    VkBool32 PresentSupported = false;

    for (uint32_t i = 0; i < Properties.size(); ++i)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDeviceIn, i, SurfaceIn, &PresentSupported);
        if (PresentSupported)
        {
            QueueFamilyIndex = i;
            return true;
        }
    }

    return false;
}

bool FVulkanContext::CheckDeviceQueueSupport(VkPhysicalDevice PhysicalDeviceIn, VkSurfaceKHR SurfaceIn)
{
    bool EverythingIsOK = true;

    for (auto& Entry : Queues)
    {
        EverythingIsOK = CheckDeviceQueueSupport(PhysicalDeviceIn, Entry.first, Entry.second.QueueIndex);
        if (!EverythingIsOK)
        {
            return false;
        }
    }

    if (SurfaceIn != VK_NULL_HANDLE)
	{
		if (!CheckDeviceQueuePresentSupport(PhysicalDeviceIn, PresentQueue.QueueIndex, SurfaceIn))
		{
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

VkQueue FVulkanContext::GetPresentQueue() const
{
    return PresentQueue.Queue;
}
uint32_t FVulkanContext::GetPresentIndex() const
{
    return PresentQueue.QueueIndex;
}

int FVulkanContext::SaveEXRWrapper(const float *Data, int Width, int Height, int Components, const int SaveAsFp16, const std::string& Name)
{
	const char* Err = NULL;
	return SaveEXR(Data, Width, Height, Components, SaveAsFp16, Name.data(), &Err);
}

void FVulkanContext::SaveBufferFloat(FBuffer& Buffer, uint32_t WidthIn, uint32_t HeightIn, const std::string& Name)
{
    if (WidthIn * HeightIn != (Buffer.BufferSize / sizeof(float)))
    {
        throw std::runtime_error("You are trying to fetch data of the wrong size from buffer");
    }

    auto Data = RESOURCE_ALLOCATOR()->DebugGetDataFromBuffer<float>(Buffer, Buffer.BufferSize, 0);

	SaveEXRWrapper(Data.data(), WidthIn, HeightIn, 1, false, Name);
}

void FVulkanContext::SaveBufferFloat3(FBuffer& Buffer, uint32_t WidthIn, uint32_t HeightIn, const std::string& Name)
{
    if (WidthIn * HeightIn != (Buffer.BufferSize / sizeof(FVector3)))
    {
        throw std::runtime_error("You are trying to fetch data of the wrong size from buffer");
    }

    auto Data = RESOURCE_ALLOCATOR()->DebugGetDataFromBuffer<float>(Buffer, Buffer.BufferSize, 0);

	SaveEXRWrapper(Data.data(), WidthIn, HeightIn, 3, false, Name.c_str());
}

void FVulkanContext::SaveBufferFloat4(FBuffer& Buffer, uint32_t WidthIn, uint32_t HeightIn, const std::string& Name)
{
	if (WidthIn * HeightIn != (Buffer.BufferSize / sizeof(FVector4)))
	{
		throw std::runtime_error("You are trying to fetch data of the wrong size from buffer");
	}

	auto Data = RESOURCE_ALLOCATOR()->DebugGetDataFromBuffer<float>(Buffer, Buffer.BufferSize, 0);

	SaveEXRWrapper(Data.data(), WidthIn, HeightIn, 4, false, Name.c_str());
}

void FVulkanContext::SaveBufferUint(FBuffer& Buffer, uint32_t WidthIn, uint32_t HeightIn, const std::string& Name)
{
    if (WidthIn * HeightIn > (Buffer.BufferSize / sizeof(uint32_t)))
    {
        throw std::runtime_error("You are trying to fetch data of the wrong size from buffer");
    }

    auto Data = RESOURCE_ALLOCATOR()->DebugGetDataFromBuffer<uint32_t>(Buffer, WidthIn * HeightIn * sizeof(uint32_t), 0);
    std::vector<float> NewData(Data.size());
    for (int i = 0; i < Data.size(); ++i)
    {
        NewData[i] = float(Data[i]);
    }

	SaveEXRWrapper(NewData.data(), WidthIn, HeightIn, 1, false, Name.c_str());
}

void FVulkanContext::SaveBufferUint3(FBuffer& Buffer, uint32_t WidthIn, uint32_t HeightIn, const std::string& Name)
{
    if (WidthIn * HeightIn != (Buffer.BufferSize / (sizeof(uint32_t) * 3)))
    {
        throw std::runtime_error("You are trying to fetch data of the wrong size from buffer");
    }

    auto Data = RESOURCE_ALLOCATOR()->DebugGetDataFromBuffer<uint32_t>(Buffer, Buffer.BufferSize, 0);
    std::vector<float> NewData(Data.size());
    for (int i = 0; i < Data.size(); ++i)
    {
        NewData[i] = float(Data[i]);
    }

	SaveEXRWrapper(NewData.data(), WidthIn, HeightIn, 3, false, Name.c_str());
}

FAccelerationStructure FVulkanContext::CreateAccelerationStructure(VkDeviceSize Size, VkAccelerationStructureTypeKHR Type, const std::string& DebugName)
{
    FAccelerationStructure AccelerationStructure;
    AccelerationStructure.Type = Type;

    AccelerationStructure.Buffer = RESOURCE_ALLOCATOR()->CreateBuffer(Size, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, DebugName + "_Buffer");

    VkAccelerationStructureCreateInfoKHR AccelerationStructureCreateInfoKHR{};
    AccelerationStructureCreateInfoKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    AccelerationStructureCreateInfoKHR.type = Type;
    AccelerationStructureCreateInfoKHR.size = Size;
    AccelerationStructureCreateInfoKHR.buffer = AccelerationStructure.Buffer.Buffer;

    VkResult Result = V::vkCreateAccelerationStructureKHR(LogicalDevice, &AccelerationStructureCreateInfoKHR, nullptr, &AccelerationStructure.AccelerationStructure);
    assert((Result == VK_SUCCESS) && "Failed to create acceleration structure");
    V::SetName(LogicalDevice, AccelerationStructure.AccelerationStructure, DebugName);
    return AccelerationStructure;
}

void FVulkanContext::DestroyAccelerationStructure(FAccelerationStructure &AccelerationStructure)
{
    V::vkDestroyAccelerationStructureKHR(LogicalDevice, AccelerationStructure.AccelerationStructure, nullptr);
    RESOURCE_ALLOCATOR()->DestroyBuffer(AccelerationStructure.Buffer);
}

VkDeviceAddress FVulkanContext::GetBufferDeviceAddressInfo(const FBuffer& Buffer)
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
    return V::vkGetAccelerationStructureDeviceAddressKHR(LogicalDevice, &AccelerationStructureDeviceAddressInfo);
}

VkAccelerationStructureGeometryTrianglesDataKHR FVulkanContext::GetAccelerationStructureGeometryTrianglesData(FBuffer& VertexBuffer, FBuffer& IndexBuffer, VkDeviceSize VertexStride, uint32_t MaxVertices, FMemoryPtr& VertexBufferPtr, FMemoryPtr& IndexBufferPtr)
{
    VkAccelerationStructureGeometryTrianglesDataKHR AccelerationStructureGeometryTrianglesData{};
    AccelerationStructureGeometryTrianglesData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    AccelerationStructureGeometryTrianglesData.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    AccelerationStructureGeometryTrianglesData.vertexData.deviceAddress = GetBufferDeviceAddressInfo(VertexBuffer) + VertexBufferPtr.Offset;
    AccelerationStructureGeometryTrianglesData.vertexStride = VertexStride;

    if (IndexBufferPtr.Size != 0)
    {
        AccelerationStructureGeometryTrianglesData.indexType = VK_INDEX_TYPE_UINT32;
        AccelerationStructureGeometryTrianglesData.indexData.deviceAddress = GetBufferDeviceAddressInfo(IndexBuffer) + IndexBufferPtr.Offset;
    }
	else
	{
		AccelerationStructureGeometryTrianglesData.indexType = VK_INDEX_TYPE_NONE_KHR;
	}

    AccelerationStructureGeometryTrianglesData.maxVertex = MaxVertices;

    return AccelerationStructureGeometryTrianglesData;
}
VkAccelerationStructureGeometryInstancesDataKHR FVulkanContext::GetAccelerationStructureGeometryInstancesData(const FBuffer& InstanceBuffer)
{
	VkAccelerationStructureGeometryInstancesDataKHR AccelerationStructureGeometryInstancesData{};
	AccelerationStructureGeometryInstancesData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	AccelerationStructureGeometryInstancesData.data.deviceAddress = GetBufferDeviceAddressInfo(InstanceBuffer);

	return AccelerationStructureGeometryInstancesData;
}

VkAccelerationStructureGeometryKHR FVulkanContext::GetAccelerationStructureGeometry(VkAccelerationStructureGeometryTrianglesDataKHR& AccelerationStructureGeometryTrianglesData)
{
    VkAccelerationStructureGeometryKHR AccelerationStructureGeometry{};
	AccelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	AccelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    AccelerationStructureGeometry.geometry.triangles = AccelerationStructureGeometryTrianglesData;

    return AccelerationStructureGeometry;
}

VkAccelerationStructureGeometryKHR FVulkanContext::GetAccelerationStructureGeometry(VkAccelerationStructureGeometryInstancesDataKHR& AccelerationStructureGeometryInstancesData)
{
	VkAccelerationStructureGeometryKHR AccelerationStructureGeometry{};
	AccelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	AccelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	AccelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	AccelerationStructureGeometry.geometry.instances = AccelerationStructureGeometryInstancesData;

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

VkAccelerationStructureBuildGeometryInfoKHR FVulkanContext::GetAccelerationStructureBuildGeometryInfo(VkAccelerationStructureGeometryKHR& AccelerationStructureGeometry, VkBuildAccelerationStructureFlagsKHR Flags, VkAccelerationStructureTypeKHR AccelerationStructureType, VkBuildAccelerationStructureModeKHR BuildAccelerationStructureMode)
{
    VkAccelerationStructureBuildGeometryInfoKHR AccelerationStructureBuildGeometryInfo{};
    AccelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    AccelerationStructureBuildGeometryInfo.type = AccelerationStructureType;
    AccelerationStructureBuildGeometryInfo.mode = BuildAccelerationStructureMode;
    AccelerationStructureBuildGeometryInfo.flags = Flags;
    AccelerationStructureBuildGeometryInfo.geometryCount = 1;
    AccelerationStructureBuildGeometryInfo.pGeometries = &AccelerationStructureGeometry;

    return AccelerationStructureBuildGeometryInfo;
}

VkAccelerationStructureBuildSizesInfoKHR FVulkanContext::GetAccelerationStructureBuildSizesInfo(const VkAccelerationStructureBuildGeometryInfoKHR& AccelerationStructureBuildGeometryInfo, uint32_t& PrimitivesCount)
{
	VkAccelerationStructureBuildSizesInfoKHR AccelerationStructureBuildSizesInfo{};
	AccelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	V::vkGetAccelerationStructureBuildSizesKHR(LogicalDevice, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &AccelerationStructureBuildGeometryInfo, &PrimitivesCount, &AccelerationStructureBuildSizesInfo);

	return AccelerationStructureBuildSizesInfo;
}

FAccelerationStructure FVulkanContext::GenerateBlas(FBuffer& VertexBuffer, FBuffer& IndexBuffer, VkDeviceSize VertexStride, uint32_t MaxVertices, FMemoryPtr& VertexBufferPtr, FMemoryPtr& IndexBufferPtr)
{
    VkAccelerationStructureGeometryTrianglesDataKHR AccelerationStructureGeometryTrianglesData = GetAccelerationStructureGeometryTrianglesData(VertexBuffer, IndexBuffer, VertexStride, MaxVertices, VertexBufferPtr, IndexBufferPtr);
    VkAccelerationStructureGeometryKHR AccelerationStructureGeometry = GetAccelerationStructureGeometry(AccelerationStructureGeometryTrianglesData);
    VkAccelerationStructureBuildGeometryInfoKHR AccelerationStructureBuildGeometryInfo = GetAccelerationStructureBuildGeometryInfo(AccelerationStructureGeometry, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR);
    VkAccelerationStructureBuildRangeInfoKHR AccelerationStructureBuildRangeInfo = GetAccelerationStructureBuildRangeInfo(MaxVertices/3);

    const VkAccelerationStructureBuildRangeInfoKHR* VkAccelerationStructureBuildRangeInfoKHRPtr = &AccelerationStructureBuildRangeInfo;

    VkAccelerationStructureBuildSizesInfoKHR AccelerationStructureBuildSizesInfo = GetAccelerationStructureBuildSizesInfo(AccelerationStructureBuildGeometryInfo, AccelerationStructureBuildRangeInfo.primitiveCount);

    FAccelerationStructure NotCompactedBLAS = CreateAccelerationStructure(AccelerationStructureBuildSizesInfo.accelerationStructureSize, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, "NotCompactedBLAS");

    FBuffer ScratchBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(AccelerationStructureBuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    V::SetName(LogicalDevice, ScratchBuffer.Buffer, "ScratchBuffer");
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

    COMMAND_BUFFER_MANAGER()->RunSingletimeCommand([&, this](VkCommandBuffer CommandBuffer)
    {
        V::vkCmdBuildAccelerationStructuresKHR(CommandBuffer, 1, &AccelerationStructureBuildGeometryInfo, &VkAccelerationStructureBuildRangeInfoKHRPtr);

        VkMemoryBarrier Barrier{};
        Barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        Barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
        Barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                             0, 1, &Barrier, 0, nullptr, 0, nullptr);

        V::vkCmdWriteAccelerationStructuresPropertiesKHR(CommandBuffer, 1, &AccelerationStructureBuildGeometryInfo.dstAccelerationStructure, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, QueryPool, 0);
    }, VK_QUEUE_COMPUTE_BIT);

    FAccelerationStructure CompactedBLAS = CreateAccelerationStructure(AccelerationStructureBuildSizesInfo.accelerationStructureSize, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, "CompactedBLAS");

    VkDeviceSize CompactedSize = 0;
    vkGetQueryPoolResults(LogicalDevice, QueryPool, 0, 1, sizeof(VkDeviceSize), &CompactedSize, sizeof(VkDeviceSize), VK_QUERY_RESULT_WAIT_BIT);

    COMMAND_BUFFER_MANAGER()->RunSingletimeCommand([&, this](VkCommandBuffer CommandBuffer)
    {
        VkCopyAccelerationStructureInfoKHR CopyAccelerationStructureInfo{};
        CopyAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR;
        CopyAccelerationStructureInfo.src = NotCompactedBLAS.AccelerationStructure;
        CopyAccelerationStructureInfo.dst = CompactedBLAS.AccelerationStructure;
        CopyAccelerationStructureInfo.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR;

        V::vkCmdCopyAccelerationStructureKHR(CommandBuffer, &CopyAccelerationStructureInfo);
    }, VK_QUEUE_COMPUTE_BIT);

    DestroyAccelerationStructure(NotCompactedBLAS);
    GetResourceAllocator()->DestroyBuffer(ScratchBuffer);

    std::cout << "Delta in size: " << AccelerationStructureBuildSizesInfo.accelerationStructureSize - CompactedSize
              << ", not compacted size is: "<< AccelerationStructureBuildSizesInfo.accelerationStructureSize<< ", compacted: " << CompactedSize << "." << std::endl;

    vkDestroyQueryPool(LogicalDevice, QueryPool, nullptr);

    return CompactedBLAS;
}

FAccelerationStructure FVulkanContext::GenerateTlas(const FBuffer& BlasInstanceBuffer, uint32_t BLASCount)
{
	FAccelerationStructure TLAS;
	FBuffer ScratchBuffer;

	VkAccelerationStructureGeometryInstancesDataKHR AccelerationStructureGeometryInstancesData = GetAccelerationStructureGeometryInstancesData(BlasInstanceBuffer);
	VkAccelerationStructureGeometryKHR AccelerationStructureGeometry = GetAccelerationStructureGeometry(AccelerationStructureGeometryInstancesData);
	VkAccelerationStructureBuildGeometryInfoKHR AccelerationStructureBuildGeometry = GetAccelerationStructureBuildGeometryInfo(AccelerationStructureGeometry, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR);
	VkAccelerationStructureBuildSizesInfoKHR AccelerationStructureBuildSizesInfo = GetAccelerationStructureBuildSizesInfo(AccelerationStructureBuildGeometry, BLASCount);

	TLAS = CreateAccelerationStructure(AccelerationStructureBuildSizesInfo.accelerationStructureSize, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, "V::TLAS");

	ScratchBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(AccelerationStructureBuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "V::TLAS_Scratch_Buffer");

	AccelerationStructureBuildGeometry.dstAccelerationStructure = TLAS.AccelerationStructure;
	AccelerationStructureBuildGeometry.scratchData.deviceAddress = GetBufferDeviceAddressInfo(ScratchBuffer);

	VkAccelerationStructureBuildRangeInfoKHR AccelerationStructureBuildRangeInfo{};
	AccelerationStructureBuildRangeInfo.primitiveCount = BLASCount;
	const VkAccelerationStructureBuildRangeInfoKHR* AccelerationStructureBuildRangeInfoPtr = &AccelerationStructureBuildRangeInfo;

	COMMAND_BUFFER_MANAGER()->RunSingletimeCommand([&, this](VkCommandBuffer CommandBuffer)
		{
			VkMemoryBarrier MemoryBarrier{};
			MemoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			MemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			MemoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
			vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1,
				&MemoryBarrier, 0, nullptr, 0, nullptr);


			V::vkCmdBuildAccelerationStructuresKHR(CommandBuffer, 1, &AccelerationStructureBuildGeometry, &AccelerationStructureBuildRangeInfoPtr);
		}, VK_QUEUE_COMPUTE_BIT);

	RESOURCE_ALLOCATOR()->DestroyBuffer(ScratchBuffer);

	return TLAS;
}

FBuffer FVulkanContext::GenerateSBT(VkPipeline Pipeline, VkStridedDeviceAddressRegionKHR& RayMissRegion, VkStridedDeviceAddressRegionKHR& RayHitRegion, VkStridedDeviceAddressRegionKHR& RayGenRegion)
{
	static const auto RTProperties = VK_CONTEXT()->GetRTProperties();
	static const uint32_t HandleSize = RTProperties.shaderGroupHandleSize;

	uint32_t MissCount = 1;
	uint32_t HitCount = 1;
	auto HandleCount = 1 + MissCount + HitCount;

	auto AlignUp = [](uint32_t X, uint32_t A)-> uint32_t
	{
		return (X + (A - 1)) & ~(A - 1);
	};

	uint32_t HandleSizeAligned = AlignUp(HandleSize, RTProperties.shaderGroupHandleAlignment);

	RayGenRegion.stride = AlignUp(HandleSize, RTProperties.shaderGroupBaseAlignment);
	RayGenRegion.size = RayGenRegion.stride;

	RayMissRegion.stride = HandleSizeAligned;
	RayMissRegion.size = AlignUp(MissCount * HandleSizeAligned, RTProperties.shaderGroupBaseAlignment);

	RayHitRegion.stride = HandleSizeAligned;
	RayHitRegion.size = AlignUp(HitCount * HandleSizeAligned, RTProperties.shaderGroupBaseAlignment);

	uint32_t DataSize = HandleCount * HandleSize;
	std::vector<uint8_t> Handles(DataSize);

	auto Result = V::vkGetRayTracingShaderGroupHandlesKHR(LogicalDevice, Pipeline, 0, HandleCount, DataSize, Handles.data());
	assert(Result == VK_SUCCESS && "Failed to get handles for SBT");

	VkDeviceSize SBTSize = RayGenRegion.size + RayMissRegion.size + RayHitRegion.size;
	FBuffer SBTBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(SBTSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "V::Raytrace_SBT_Buffer");

	auto SBTBufferAddress = VK_CONTEXT()->GetBufferDeviceAddressInfo(SBTBuffer);
	RayGenRegion.deviceAddress = SBTBufferAddress;
	RayMissRegion.deviceAddress = RayGenRegion.deviceAddress + RayGenRegion.size;
	RayHitRegion.deviceAddress = RayMissRegion.deviceAddress + RayMissRegion.size;

	auto GetHandle = [&](int i)
	{
		return Handles.data() + i * HandleSize;
	};

	uint32_t HandleIndex{0};
	std::vector<uint8_t> SBTData(SBTSize);
	memcpy(SBTData.data(), GetHandle(HandleIndex++), HandleSize);
	memcpy(SBTData.data() + RayGenRegion.size, GetHandle(HandleIndex++), HandleSize);
	memcpy(SBTData.data() + RayGenRegion.size + RayMissRegion.size, GetHandle(HandleIndex++), HandleSize);
	RESOURCE_ALLOCATOR()->LoadDataToBuffer(SBTBuffer, {SBTSize}, {0}, {SBTData.data()});

	return SBTBuffer;
}

VkDevice FVulkanContext::CreateLogicalDevice(VkPhysicalDevice PhysicalDeviceIn, FVulkanContextOptions& VulkanContextOptions)
{
    std::set<uint32_t> QueueIndices;

	if (SurfaceCreationFunction)
	{
		QueueIndices.insert(PresentQueue.QueueIndex);
	}

    for (auto& Entry : Queues)
    {
        QueueIndices.insert(Entry.second.QueueIndex);
    }
    auto DeviceQueueCreateInfo = GetDeviceQueueCreateInfo(PhysicalDeviceIn, QueueIndices);
    VkDeviceCreateInfo CreateInfo{};

    VkPhysicalDeviceFeatures2 DeviceFeatures2{};
    DeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    DeviceFeatures2.features.samplerAnisotropy = VK_TRUE;
    DeviceFeatures2.features.sampleRateShading = VK_TRUE;
    DeviceFeatures2.features.shaderInt64 = VK_TRUE;

    VulkanContextOptions.BuildDevicePNextChain(reinterpret_cast<BaseVulkanStructure*>(&DeviceFeatures2));

    CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    CreateInfo.pQueueCreateInfos = DeviceQueueCreateInfo.data();
    CreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueIndices.size());
    CreateInfo.pNext = &DeviceFeatures2;

    auto DeviceExtensions = VulkanContextOptions.GetDeviceExtensionsList();
    CreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
    CreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();

    auto DeviceLayers = VulkanContextOptions.GetDeviceLayers();
    CreateInfo.enabledLayerCount = static_cast<uint32_t>(DeviceLayers.size());
    CreateInfo.ppEnabledLayerNames = DeviceLayers.data();

    VkDevice LogicalDeviceIn;

    if (vkCreateDevice(PhysicalDeviceIn, &CreateInfo, nullptr, &LogicalDeviceIn) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device!");
    }

    return LogicalDeviceIn;
}

void FVulkanContext::SetLogicalDevice(VkDevice LogicalDeviceIn)
{
    this->LogicalDevice = LogicalDeviceIn;
}

VkDevice FVulkanContext::GetLogicalDevice() const
{
    return LogicalDevice;
}

void FVulkanContext::GetDeviceQueues(VkSurfaceKHR Surface)
{
    static std::unordered_map<VkQueueFlagBits, std::string> QueueTypeToStringNameMap =
            {
                    {VK_QUEUE_GRAPHICS_BIT, "V::Graphics_Queue"},
                    {VK_QUEUE_COMPUTE_BIT, "V::Compute_Queue"},
                    {VK_QUEUE_TRANSFER_BIT, "V::Transfer_Queue"},
                    {VK_QUEUE_SPARSE_BINDING_BIT, "V::Sparse_Binding_Queue"}
            };
    for (auto& Entry : Queues)
    {
        CheckDeviceQueueSupport(PhysicalDevice, Entry.first, Entry.second.QueueIndex);
        vkGetDeviceQueue(LogicalDevice, Entry.second.QueueIndex, 0, &Entry.second.Queue);
        V::SetName(LogicalDevice, Entry.second.Queue, QueueTypeToStringNameMap[Entry.first]);
    }

	if (SurfaceCreationFunction)
	{
		CheckDeviceQueuePresentSupport(PhysicalDevice, PresentQueue.QueueIndex, Surface);
		vkGetDeviceQueue(LogicalDevice, PresentQueue.QueueIndex, 0, &PresentQueue.Queue);
		V::SetName(LogicalDevice, PresentQueue.Queue, "Present");
	}
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

void FVulkanContext::SaveImagePng(const std::string& ImageName, const std::string& FileName)
{
	auto Image = TEXTURE_MANAGER()->GetFramebufferImage(ImageName);
	std::string FileNameToUse = (FileName == "") ? (Image->DebugName) : (FileName);
	std::vector<uint8_t> ImageData;
	int NumberOfComponents = 0;
	const uint32_t ImageSize = Image->Width * Image->Height;

	if (Image->Format == VK_FORMAT_B8G8R8A8_SRGB)
	{
		NumberOfComponents = 4;
		std::vector<uint8_t> Data;
		FetchImageData(*Image, Data);
		ImageData = std::move(Data);
	}
	if (Image->Format == VK_FORMAT_R32G32B32A32_SFLOAT)
	{
		NumberOfComponents = 4;
		ImageData.resize(ImageSize * NumberOfComponents);
		std::vector<float> Data;
		FetchImageData(*Image, Data);

		for (int i = 0; i < ImageSize; ++i)
		{
			ImageData[i * NumberOfComponents] = static_cast<uint8_t>(std::clamp(LinearToSRGB(Data[i * NumberOfComponents]) * 255., 0., 255.));
			ImageData[i * NumberOfComponents + 1] = static_cast<uint8_t>(std::clamp(LinearToSRGB(Data[i * NumberOfComponents + 1]) * 255., 0., 255.));
			ImageData[i * NumberOfComponents + 2] = static_cast<uint8_t>(std::clamp(LinearToSRGB(Data[i * NumberOfComponents + 2]) * 255., 0., 255.));
			ImageData[i * NumberOfComponents + 3] = static_cast<uint8_t>(std::clamp(Data[i * NumberOfComponents + 3] * 255., 0., 255.));
		}
	}

	stbi_write_png((FileNameToUse + ".png").c_str(), Image->Width, Image->Height, NumberOfComponents, ImageData.data(), Image->Width * NumberOfComponents);
}

void FVulkanContext::SaveImageExr(const std::string& ImageName, const std::string& FileName)
{
	auto Image = TEXTURE_MANAGER()->GetFramebufferImage(ImageName);
	std::string FileNameToUse = (FileName.empty()) ? (Image->DebugName) : (FileName);
	std::vector<float> ImageData;
	int NumberOfComponents = 0;
	const uint32_t ImageSize = Image->Width * Image->Height;

	if (Image->Format == VK_FORMAT_B8G8R8A8_SRGB)
	{
		NumberOfComponents = 4;
		std::vector<char> Data;
		FetchImageData(*Image, Data);
		ImageData.resize(Data.size());

		for (int i = 0; i < Data.size() / 4; ++i)
		{
			/// We do components switchy switchy
			ImageData[i * NumberOfComponents] = 	float(Data[i * NumberOfComponents + 2]) / 255;
			ImageData[i * NumberOfComponents + 1] = float(Data[i * NumberOfComponents + 1]) / 255;
			ImageData[i * NumberOfComponents + 2] = float(Data[i * NumberOfComponents]) / 255;
			ImageData[i * NumberOfComponents + 3] = float(Data[i * NumberOfComponents + 3]) / 255;
		}
	}
	if (Image->Format == VK_FORMAT_R32G32B32A32_SFLOAT)
	{
		NumberOfComponents = 4;
		std::vector<float> Data;
		FetchImageData(*Image, Data);
		ImageData = std::move(Data);
	}
	if (Image->Format == VK_FORMAT_R32G32_UINT)
	{
		NumberOfComponents = 3;
		std::vector<float> Data;
		FetchImageData(*Image, Data);

		ImageData.resize(ImageSize * 3);
		for (int i = 0; i < ImageSize; ++i)
		{
			ImageData[i * NumberOfComponents] = Data[i * 2];
			ImageData[i * NumberOfComponents + 1] = Data[i * 2 + 1];
			ImageData[i * NumberOfComponents + 2] = 0;
		};

		return;
	}

	if (NumberOfComponents == 0)
	{
		throw std::runtime_error("Number of components can not be 0");
	}

	SaveEXRWrapper(ImageData.data(), Image->Width, Image->Height, NumberOfComponents, false, FileName + ".exr");
}

template <typename T>
void FVulkanContext::FetchImageData(const FImage& Image, std::vector<T>& Data)
{
    uint32_t NumberOfComponents = 0;
	uint32_t ComponentSize = 0;

    switch (Image.Format) {
        case VK_FORMAT_B8G8R8A8_SRGB:
        {
            NumberOfComponents = 4;
			ComponentSize = 1;
            break;
        }
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        {
            NumberOfComponents = 4;
			ComponentSize = 4;
            break;
        }
        case VK_FORMAT_R32_UINT:
        {
            NumberOfComponents = 1;
			ComponentSize = 4;
            break;
        }
		case VK_FORMAT_R32G32_UINT:
		{
			NumberOfComponents = 2;
			ComponentSize = 4;
			break;
		}
    }

    RESOURCE_ALLOCATOR()->CopyImageToBuffer(Image, RESOURCE_ALLOCATOR()->StagingBuffer);

    Data.resize(Image.Height * Image.Width * NumberOfComponents * ComponentSize / sizeof(T));

    RESOURCE_ALLOCATOR()->LoadDataFromStagingBuffer(Data.size() * sizeof(T), Data.data(), 0);
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

ImagePtr FVulkanContext::CreateEXRImageFromFile(const std::string& Path, const std::string& DebugImageName)
{
    float* Out;
    int Width;
    int Height;
    const char* Err = nullptr;

    int Result = LoadEXR(&Out, &Width, &Height, Path.c_str(), &Err);

    assert(TINYEXR_SUCCESS == Result && "Failed to load .exr image.");

    ImagePtr Image = std::make_shared<FImage>(Width, Height, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                              VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, DebugImageName);

    V::SetName(LogicalDevice, Image->Image, DebugImageName);
    V::SetName(LogicalDevice, Image->View, DebugImageName);

    Image->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    RESOURCE_ALLOCATOR()->LoadDataToImage(*Image, Width * Height * 4 * sizeof(float), Out);

	auto [ImportanceBuffer, IBLPDF] = GenerateImportanceMapFast<FVector4>((void*)Out, Width, Height, [](FVector4 Val){return 0.2126 * Val.x+ 0.7152 * Val.y + 0.0722 * Val.z;});

	FBuffer IBLImportanceBuffer;

	if (RESOURCE_ALLOCATOR()->BufferExists("IBLImportanceBuffer"))
	{
		IBLImportanceBuffer = RESOURCE_ALLOCATOR()->GetBuffer("IBLImportanceBuffer");
	}
	else
	{
		IBLImportanceBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(sizeof(FAliasTableEntry) * ImportanceBuffer.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "IBLImportanceBuffer");
		RESOURCE_ALLOCATOR()->RegisterBuffer(IBLImportanceBuffer, "IBLImportanceBuffer");
	}

	if (IBLImportanceBuffer.BufferSize != ImportanceBuffer.size() * sizeof(FAliasTableEntry))
	{
		RESOURCE_ALLOCATOR()->DestroyBuffer(IBLImportanceBuffer);
		IBLImportanceBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(sizeof(FAliasTableEntry) * ImportanceBuffer.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "IBLImportanceBuffer");
	}

	RESOURCE_ALLOCATOR()->LoadDataToBuffer(IBLImportanceBuffer, {sizeof(FAliasTableEntry) * ImportanceBuffer.size()}, {0}, {ImportanceBuffer.data()});

	FBuffer IBLPDFBuffer;

	if (RESOURCE_ALLOCATOR()->BufferExists("IBLPDFBuffer"))
	{
		IBLPDFBuffer = RESOURCE_ALLOCATOR()->GetBuffer("IBLPDFBuffer");
	}
	else
	{
		IBLPDFBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(sizeof(float) * IBLPDF.size(),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "IBLPDFBuffer");
		RESOURCE_ALLOCATOR()->RegisterBuffer(IBLPDFBuffer, "IBLPDFBuffer");
	}

	if (IBLPDFBuffer.BufferSize != IBLPDF.size() * sizeof(float))
	{
		RESOURCE_ALLOCATOR()->DestroyBuffer(IBLPDFBuffer);
		IBLPDFBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(sizeof(float) * IBLPDF.size(),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "IBLPDFBuffer");
	}

	RESOURCE_ALLOCATOR()->LoadDataToBuffer(IBLPDFBuffer, {sizeof(float) * IBLPDF.size()}, {0}, {IBLPDF.data()});

    return Image;
}

ImagePtr FVulkanContext::LoadImageFromFile(const std::string& Path, const std::string& DebugImageName)
{
    /// Load data from image file
    int TexWidth, TexHeight, TexChannels;
	/// STBI_rgb_alpha
    stbi_uc* Pixels = stbi_load(Path.c_str(), &TexWidth, &TexHeight, &TexChannels, STBI_rgb_alpha);
    VkDeviceSize ImageSize = TexWidth * TexHeight * STBI_rgb_alpha;

    if (!Pixels)
    {
        throw std::runtime_error("Failed to load texture image!");
    }

	/// For 1 and 2 channels we repack the loaded data from 4 channels
	if (TexChannels == 2)
	{
		for (int i = 0; i < TexHeight * TexWidth; i++)
		{
			Pixels[2 * i] = Pixels[4 * i];
			Pixels[2 * i + 1] = Pixels[4 * i + 1];
		}
		ImageSize = TexWidth * TexHeight * TexChannels;
	}

	if (TexChannels == 1)
	{
		for (int i = 0; i < TexHeight * TexWidth; i++)
		{
			Pixels[i] = Pixels[4 * i];
		}
		ImageSize = TexWidth * TexHeight * TexChannels;
	}

    /// Load data into staging buffer
	std::unordered_map<int, VkFormat> FormatMap{{1, VK_FORMAT_R8_UNORM}, {2, VK_FORMAT_R8G8_UNORM}, {3, VK_FORMAT_R8G8B8A8_UNORM}, {4, VK_FORMAT_R8G8B8A8_UNORM} };

    ImagePtr Image = std::make_shared<FImage>(TexWidth, TexHeight, true, VK_SAMPLE_COUNT_1_BIT, FormatMap[TexChannels], VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, DebugImageName);

    V::SetName(LogicalDevice, Image->Image, DebugImageName);
    V::SetName(LogicalDevice, Image->View, DebugImageName);

    Image->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    RESOURCE_ALLOCATOR()->LoadDataToImage(*Image, ImageSize, Pixels);
    /// Generate MipMaps only after we loaded image data
    Image->GenerateMipMaps();

    return Image;
}

ImagePtr FVulkanContext::Wrap(VkImage ImageToWrap, uint32_t WidthIn, uint32_t HeightIn, VkFormat Format, VkImageAspectFlags AspectFlags, VkDevice LogicalDevice, const std::string& DebugImageName)
{
    ImagePtr Image = std::make_shared<FImage>(ImageToWrap, WidthIn, HeightIn, Format, AspectFlags, LogicalDevice, DebugImageName);

    return Image;
}

VkFramebuffer FVulkanContext::CreateFramebuffer(uint32_t Width, uint32_t Height, std::vector<ImagePtr> Images, VkRenderPass RenderPass, const std::string& debug_name) const
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
    FramebufferCreateInfo.width = Width;
    FramebufferCreateInfo.height = Height;
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

VkDescriptorPool FVulkanContext::CreateFreeableDescriptorPool(const std::map<VkDescriptorType, uint32_t>& DescriptorsMap, VkDevice LogicalDevice, const std::string& debug_name)
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
    PoolInfo.flags |= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;

    if (vkCreateDescriptorPool(LogicalDevice, &PoolInfo, nullptr, &DescriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool!");
    }

    V::SetName(LogicalDevice, DescriptorPool, debug_name);

    return DescriptorPool;
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

VkPipeline FVulkanContext::CreateRayTracingPipeline(VkShaderModule RayGenShader, VkShaderModule RayMissShader, VkShaderModule RayClosestHitShader, VkPipelineLayout PipelineLayout)
{
    std::vector<VkPipelineShaderStageCreateInfo> Stages(3);

    Stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    Stages[0].pName = "main";
    Stages[0].module = RayGenShader;
    Stages[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    Stages[2].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    Stages[2].pName = "main";
    Stages[2].module = RayClosestHitShader;
    Stages[2].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

    Stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    Stages[1].pName = "main";
    Stages[1].module = RayMissShader;
    Stages[1].stage = VK_SHADER_STAGE_MISS_BIT_KHR;

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
            case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
            {
                RayTracingShaderGroupCreateInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
                RayTracingShaderGroupCreateInfo.closestHitShader = i;
                break;
            }
            case VK_SHADER_STAGE_MISS_BIT_KHR:
            {
                RayTracingShaderGroupCreateInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
                RayTracingShaderGroupCreateInfo.generalShader = i;
                break;
            }
            default:
            {
                throw std::runtime_error("Failed to create ray tracing pipeline! Incorrect group info.");
            }
        }

        RayTracingShaderGroupCreateInfoVector.push_back(RayTracingShaderGroupCreateInfo);
    }

    VkRayTracingPipelineCreateInfoKHR RayTracingPipelineCreateInfo{};
    RayTracingPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    RayTracingPipelineCreateInfo.stageCount = static_cast<uint32_t>(Stages.size());
    RayTracingPipelineCreateInfo.pStages = Stages.data();
    RayTracingPipelineCreateInfo.groupCount = static_cast<uint32_t>(RayTracingShaderGroupCreateInfoVector.size());
    RayTracingPipelineCreateInfo.pGroups = RayTracingShaderGroupCreateInfoVector.data();
    RayTracingPipelineCreateInfo.maxPipelineRayRecursionDepth = 1;
    RayTracingPipelineCreateInfo.layout = PipelineLayout;

    VkPipeline Pipeline = VK_NULL_HANDLE;

    V::vkCreateRayTracingPipelinesKHR(LogicalDevice, {}, {}, 1, &RayTracingPipelineCreateInfo, nullptr, &Pipeline);

    return Pipeline;
}

VkPipeline FVulkanContext::CreateComputePipeline(VkShaderModule ComputeShader, VkPipelineLayout PipelineLayout)
{
    VkPipelineShaderStageCreateInfo PipelineShaderStageCreateinfo{};
    PipelineShaderStageCreateinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    PipelineShaderStageCreateinfo.module = ComputeShader;
    PipelineShaderStageCreateinfo.pName = "main";
    PipelineShaderStageCreateinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;

    VkComputePipelineCreateInfo ComputePipelineCreateInfo{};
    ComputePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    ComputePipelineCreateInfo.layout = PipelineLayout;
    ComputePipelineCreateInfo.stage = PipelineShaderStageCreateinfo;

    VkPipeline Pipeline = VK_NULL_HANDLE;

    if (vkCreateComputePipelines(LogicalDevice, nullptr, 1, &ComputePipelineCreateInfo, nullptr, &Pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create compute pipeline!");
    }

    return Pipeline;
}

VkSemaphore FVulkanContext::CreateSemaphore() const
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

VkFence FVulkanContext::CreateSignalledFence() const
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

VkFence FVulkanContext::CreateUnsignalledFence() const
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

VkQueryPool FVulkanContext::CreateQueryPool(uint32_t QueryCount, VkQueryType QueryType)
{
	VkQueryPool QueryPool = VK_NULL_HANDLE;

	VkQueryPoolCreateInfo QueryPoolCreateInfo{};
	QueryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	QueryPoolCreateInfo.queryCount = QueryCount;
	QueryPoolCreateInfo.queryType = QueryType;

	if (vkCreateQueryPool(VK_CONTEXT()->LogicalDevice, &QueryPoolCreateInfo, nullptr, &QueryPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create query pool!");
	}

	return QueryPool;
}

VkFormat FVulkanContext::FindSupportedFormat(const std::vector<VkFormat>& Candidates, VkImageTiling Tiling, VkFormatFeatureFlags Features) const
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

VkSampler FVulkanContext::CreateTextureSampler(uint32_t MipLevel, VkFilter Filter)
{
	VkPhysicalDeviceProperties Properties{};
	vkGetPhysicalDeviceProperties(PhysicalDevice, &Properties);

    VkSamplerCreateInfo SamplerInfo{};
    SamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    SamplerInfo.magFilter = Filter;
    SamplerInfo.minFilter = Filter;
    SamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    SamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    SamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    SamplerInfo.anisotropyEnable = VK_TRUE;
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

VkResult FVulkanContext::Present(VkSwapchainKHR Swapchain, VkSemaphore WaitSemaphore, uint32_t ImageIndex)
{
    VkSemaphore WaitSemaphores[] = {WaitSemaphore};

    VkPresentInfoKHR PresentInfo{};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pWaitSemaphores = WaitSemaphores;
    VkSwapchainKHR SwapChains[] = {Swapchain};
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = SwapChains;
    PresentInfo.pImageIndices = &ImageIndex;
    PresentInfo.pResults = nullptr;

    VkResult Result = vkQueuePresentKHR(PresentQueue.Queue, &PresentInfo);

    if (Result != VK_ERROR_OUT_OF_DATE_KHR && Result != VK_SUBOPTIMAL_KHR && Result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    return Result;
}

void FVulkanContext::WaitIdle() const
{
    vkDeviceWaitIdle(LogicalDevice);
}

#ifndef NDEBUG
void FVulkanContext::DestroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT& DebugUtilsMessenger) const
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
    FREE_TEXTURE_MANAGER();

    DescriptorSetManager = nullptr;
    FREE_COMMAND_BUFFER_MANAGER();
    FREE_RESOURCE_ALLOCATOR();

    vkDestroyDevice(LogicalDevice, nullptr);

#ifndef NDEBUG
    DestroyDebugUtilsMessengerEXT(DebugMessenger);
#endif

	if (Surface)
	{
		vkDestroySurfaceKHR(Instance, Surface, nullptr);
	}

    vkDestroyInstance(Instance, nullptr);
}

FVulkanContext::~FVulkanContext()
{
}
