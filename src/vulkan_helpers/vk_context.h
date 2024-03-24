#pragma once

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

#include "resource_allocation.h"
#include "command_buffer_manager.h"
#include "timing_manager.h"
#include "maths.h"
#include "swapchain.h"
#include "buffer.h"
#include "descriptors.h"
#include "image.h"
#include "vk_acceleration_structure.h"
#include "vk_utils.h"
#include "vk_pipeline.h"

#include <vector>
#include <string>
#include <array>
#include <memory>
#include <map>

class FVulkanContext
{
public:
    FVulkanContext() = default;
    FVulkanContext operator=(const FVulkanContext* Other) = delete;
    FVulkanContext(const FVulkanContext& Other) = delete;
    ~FVulkanContext();

#ifndef NDEBUG
    VkDebugUtilsMessengerEXT CreateDebugMessenger(FVulkanContextOptions& VulkanContextOptions) const;
    void SetDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT DebugUtilsMessengerEXT);
    void DestroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT& DebugUtilsMessenger) const;
#endif
    void InitManagerResources();
    void QueuePhysicalDeviceProperties();
    void GetDeviceQueues(VkSurfaceKHR Surface);
    void CleanUp();

    void WaitIdle() const;

    VkFormat FindSupportedFormat(const std::vector<VkFormat>& Candidates, VkImageTiling Tiling, VkFormatFeatureFlags Features) const;
    VkFormat FindDepthFormat();
    static bool HasStensilComponent(VkFormat Format);

    bool CheckInstanceLayersSupport(const std::vector<const char*>& Layers);
    void SetInstance(VkInstance InstanceIn);
    VkInstance GetInstance() const;

    bool CheckDeviceExtensionsSupport(VkPhysicalDevice Device, std::set<std::string>& RequiredExtension);
    bool CheckDeviceQueueSupport(VkPhysicalDevice PhysicalDeviceIn, VkSurfaceKHR SurfaceIn);
    bool CheckDeviceQueueSupport(VkPhysicalDevice PhysicalDeviceIn, VkQueueFlagBits QueueFlagBits, uint32_t& QueueFamilyIndex);
    VkPhysicalDevice PickPhysicalDevice(FVulkanContextOptions& VulkanContextOptions, VkSurfaceKHR SurfaceIn);
    void SetPhysicalDevice(VkPhysicalDevice PhysicalDeviceIn);
    VkPhysicalDevice GetPhysicalDevice() const;
    VkPhysicalDeviceProperties2 GetPhysicalDeviceProperties2(VkPhysicalDevice PhysicalDevice, void* pNextStructure);
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR GetRTProperties();

    VkInstance CreateVkInstance(const std::string& AppName, const FVersion3& AppVersion, const std::string& EngineName, const FVersion3& EngineVersion, uint32_t ApiVersion, FVulkanContextOptions& VulkanContextOptions);
    std::vector<VkPhysicalDevice> EnumerateAllPhysicalDevices(VkInstance InstanceIn);
    std::vector<VkQueueFamilyProperties> EnumeratePhysicalDeviceQueueFamilyProperties(VkPhysicalDevice Device);
    bool CheckDeviceQueuePresentSupport(VkPhysicalDevice PhysicalDeviceIn, uint32_t& QueueFamilyIndex, VkSurfaceKHR SurfaceIn);
    VkDevice CreateLogicalDevice(VkPhysicalDevice PhysicalDeviceIn, FVulkanContextOptions& VulkanContextOptions);
    void SetLogicalDevice(VkDevice LogicalDeviceIn);
    VkDevice GetLogicalDevice() const;
    std::vector<VkDeviceQueueCreateInfo> GetDeviceQueueCreateInfo(VkPhysicalDevice PhysicalDevice, std::set<uint32_t> QueueIndices);

    VkResult Present(VkSwapchainKHR Swapchain, VkSemaphore WaitSemaphore, uint32_t ImageIndex);

    VkQueue GetGraphicsQueue();
    uint32_t GetGraphicsQueueIndex();
    VkQueue GetComputeQueue();
    uint32_t GetComputeQueueIndex();
    VkQueue GetTransferQueue();
    uint32_t GetTransferQueueIndex();
    VkQueue GetSparseBindingQueue();
    uint32_t GetSparseBindingQueueIndex();
    VkQueue GetPresentQueue() const;
    uint32_t GetPresentIndex() const;

    VkSurfaceKHR CreateSurface(GLFWwindow* WindowIn) const;
    void DestroySurface(VkSurfaceKHR* SurfaceIn) const;
    void SetSurface(VkSurfaceKHR SurfaceIn);
    VkSurfaceKHR GetSurface() const;

    void SaveBufferFloat(FBuffer& Buffer, uint32_t WidthIn, uint32_t HeightIn, const std::string& Name);
    void SaveBufferFloat3(FBuffer& Buffer, uint32_t WidthIn, uint32_t HeightIn, const std::string& Name);
    void SaveBufferUint(FBuffer& Buffer, uint32_t WidthIn, uint32_t HeightIn, const std::string& Name);
    void SaveBufferUint3(FBuffer& Buffer, uint32_t WidthIn, uint32_t HeightIn, const std::string& Name);

    FAccelerationStructure CreateAccelerationStructure(VkDeviceSize Size, VkAccelerationStructureTypeKHR Type, const std::string& DebugName = "");
    void DestroyAccelerationStructure(FAccelerationStructure &AccelerationStructure);
    VkDeviceAddress GetBufferDeviceAddressInfo(const FBuffer& Buffer);
    VkDeviceAddress GetASDeviceAddressInfo(FAccelerationStructure& AS);
    VkAccelerationStructureGeometryTrianglesDataKHR GetAccelerationStructureGeometryTrianglesData(FBuffer& VertexBuffer, FBuffer& IndexBuffer, VkDeviceSize VertexStride, uint32_t MaxVertices, FMemoryPtr& VertexBufferPtr, FMemoryPtr& IndexBufferPtr);
    VkAccelerationStructureGeometryKHR GetAccelerationStructureGeometry(VkAccelerationStructureGeometryTrianglesDataKHR& AccelerationStructureGeometryTrianglesData);
    VkAccelerationStructureBuildRangeInfoKHR GetAccelerationStructureBuildRangeInfo(uint32_t PrimitiveCount);
    VkAccelerationStructureBuildGeometryInfoKHR GetAccelerationStructureBuildGeometryInfo(VkAccelerationStructureGeometryKHR& AccelerationStructureGeometry, VkBuildAccelerationStructureFlagsKHR Flags);
    FAccelerationStructure GenerateBlas(FBuffer& VertexBuffer, FBuffer& IndexBuffer, VkDeviceSize VertexStride, uint32_t MaxVertices, FMemoryPtr& VertexBufferPtr, FMemoryPtr& IndexBufferPtr);
    FAccelerationStructure GenerateTlas(const FBuffer& BlasInstanceBuffer, uint32_t BLASCount);

    ImagePtr CreateImage2D(uint32_t Width, uint32_t Height, bool bMipMapsRequired, VkSampleCountFlagBits NumSamples, VkFormat Format,
                               VkImageTiling Tiling, VkImageUsageFlags Usage, VkMemoryPropertyFlags Properties,
                               VkImageAspectFlags AspectFlags, VkDevice Device, const std::string& DebugImageName);
    ImagePtr CreateEXRImageFromFile(const std::string& Path, const std::string& DebugImageName);
    ImagePtr LoadImageFromFile(const std::string& Path, const std::string& DebugImageName);
    ImagePtr Wrap(VkImage ImageToWrap, VkFormat Format, VkImageAspectFlags AspectFlags, VkDevice LogicalDevice, const std::string& DebugImageName);
    void SaveImage(const FImage& Image);
    template <typename T>
    void FetchImageData(const FImage& Image, std::vector<T>& Data);

    VkSampler CreateTextureSampler(uint32_t MipLevel);

    VkFramebuffer CreateFramebuffer(int Width, int Height, std::vector<ImagePtr> Images, VkRenderPass RenderPass, const std::string& debug_name) const;

    VkDescriptorPool CreateDescriptorPool(const std::map<VkDescriptorType, uint32_t>& DescriptorsMap, VkDevice LogicalDevice, const std::string& debug_name);

    VkRenderPass CreateRenderpass(VkDevice LogicalDevice, FGraphicsPipelineOptions& GraphicsPipelineOptions);
    VkPipeline CreateGraphicsPipeline(VkShaderModule VertexShader, VkShaderModule FragmentShader, std::uint32_t Width, std::uint32_t Height, FGraphicsPipelineOptions& GraphicsPipelineOptions);
    VkPipeline CreateRayTracingPipeline(VkShaderModule RayGenShader, VkShaderModule RayMissShader, VkShaderModule VertexShader, std::uint32_t Width, std::uint32_t Height, VkPipelineLayout PipelineLayout);
    VkPipeline CreateComputePipeline(VkShaderModule ComputeShader, VkPipelineLayout PipelineLayout);

    VkSemaphore CreateSemaphore() const;
    VkFence CreateSignalledFence() const;
    VkFence CreateUnsignalledFence() const;

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT MessageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallBackData,
            void* pUserData);

public:
    VkQueue GetQueue(VkQueueFlagBits QueueFlagBits);
    uint32_t GetQueueIndex(VkQueueFlagBits QueueFlagBits);

    std::shared_ptr<FResourceAllocator> ResourceAllocator = nullptr;
    std::shared_ptr<FCommandBufferManager> CommandBufferManager = nullptr;
    std::shared_ptr<FTimingManager> TimingManager = nullptr;

    VkSampleCountFlagBits MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    int TimestampPeriod = 1;

    VkInstance Instance = VK_NULL_HANDLE;
    VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
    VkDevice LogicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR Surface = VK_NULL_HANDLE;

    /// Queues
    struct IndexQueue
    {
        uint32_t QueueIndex = UINT32_MAX;
        VkQueue Queue = VK_NULL_HANDLE;
    };
    std::map <VkQueueFlagBits, IndexQueue> Queues = {{VK_QUEUE_GRAPHICS_BIT, {UINT32_MAX, VK_NULL_HANDLE}},
                                                  {VK_QUEUE_COMPUTE_BIT,  {UINT32_MAX, VK_NULL_HANDLE}},};
    IndexQueue PresentQueue;

#ifndef NDEBUG
    VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;
#endif

    uint32_t MipLevels = 0;
    std::shared_ptr<FDescriptorSetManager>DescriptorSetManager = nullptr;

    bool bFramebufferResized = false;
};

FVulkanContext& GetContext();
std::shared_ptr<FResourceAllocator> GetResourceAllocator();