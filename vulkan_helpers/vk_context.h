#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"

#include "resource_allocation.h"
#include "command_buffer_manager.h"
#include "maths.h"
#include "swapchain.h"
#include "controller.h"
#include "buffer.h"
#include "descriptors.h"
#include "image.h"
#include "renderpass.h"
#include "vk_utils.h"
#include "vk_pipeline.h"

#include "task_render.h"
#include "task_passthrough.h"
#include "task_imgui.h"

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

    void Init(GLFWwindow* Window, int Width, int Height);

#ifndef NDEBUG
    VkDebugUtilsMessengerEXT CreateDebugMessenger(FVulkanContextOptions& VulkanContextOptions);
    void SetDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT DebugUtilsMessengerEXT);
    void DestroyDebugUtilsMessengerEXT(FVulkanContextOptions& VulkanContextOptions);
#endif
    void InitManagerResources(int Width, int Height, VkSurfaceKHR Surface);
    void QueuePhysicalDeviceProperties();
    void GetDeviceQueues(VkSurfaceKHR Surface);
    void CreateDepthAndAAImages();
    void LoadModelDataToGPU();
    void CreateUniformBuffers();
    void CreatePipelines();
    void CreateSyncObjects();
    void CreateImguiContext(GLFWwindow* Window);
    void RecreateSwapChain(int Width, int Height);
    void CleanUpSwapChain();
    void CleanUp();
    void UpdateUniformBuffer(uint32_t CurrentImage);

    void Render();
    void Present();
    void WaitIdle();

    VkFormat FindSupportedFormat(const std::vector<VkFormat>& Candidates, VkImageTiling Tiling, VkFormatFeatureFlags Features);
    VkFormat FindDepthFormat();
    bool HasStensilComponent(VkFormat Format);

    bool CheckInstanceLayersSupport(const std::vector<const char*>& Layers);
    void SetInstance(VkInstance Instance);
    VkInstance GetInstance();

    bool CheckDeviceExtensionsSupport(VkPhysicalDevice Device, std::set<std::string>& RequiredExtension);
    bool CheckDeviceQueueSupport(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface);
    bool CheckDeviceQueueSupport(VkPhysicalDevice Device, VkQueueFlagBits QueueFlagBits, uint32_t& QueueFamilyIndex);
    VkPhysicalDevice PickPhysicalDevice(FVulkanContextOptions& VulkanContextOptions, VkSurfaceKHR Surface);
    void SetPhysicalDevice(VkPhysicalDevice PhysicalDevice);
    VkPhysicalDevice GetPhysicalDevice();

    VkInstance CreateVkInstance(const std::string& AppName, const FVersion3& AppVersion, const std::string& EngineName, const FVersion3& EngineVersion, uint32_t ApiVersion, FVulkanContextOptions& VulkanContextOptions);
    std::vector<VkPhysicalDevice> EnumerateAllPhysicalDevices(VkInstance Instance);
    std::vector<VkQueueFamilyProperties> EnumeratePhysicalDeviceQueueFamilyProperties(VkPhysicalDevice Device);
    bool CheckDeviceQueuePresentSupport(VkPhysicalDevice PhysicalDevice, uint32_t& QueueFamilyIndex, VkSurfaceKHR Surface);
    VkDevice CreateLogicalDevice(VkPhysicalDevice PhysicalDevice, FVulkanContextOptions& VulkanContextOptions);
    void SetLogicalDevice(VkDevice LogicalDevice);
    std::vector<VkDeviceQueueCreateInfo> GetDeviceQueueCreateInfo(VkPhysicalDevice PhysicalDevice, std::set<uint32_t> QueueIndices);

    VkQueue GetGraphicsQueue();
    uint32_t GetGraphicsQueueIndex();
    VkQueue GetComputeQueue();
    uint32_t GetComputeQueueIndex();
    VkQueue GetTransferQueue();
    uint32_t GetTransferQueueIndex();
    VkQueue GetSparseBindingQueue();
    uint32_t GetSparseBindingQueueIndex();
    VkQueue GetPresentQueue();
    uint32_t GetPresentIndex();

    VkSurfaceKHR CreateSurface(GLFWwindow* Window);
    void SetSurface(VkSurfaceKHR Surface);
    VkSurfaceKHR GetSurface();

    void SetWindow(GLFWwindow* Window);
    GLFWwindow* GetWindow();

    FBuffer CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties);
    FMemoryPtr PushDataToBuffer(FBuffer& Buffer, VkDeviceSize Size, void* Data);
    void CopyBuffer(FBuffer &SrcBuffer, FBuffer &DstBuffer, VkDeviceSize Size, VkDeviceSize SourceOffset, VkDeviceSize DestinationOffset);
    void DestroyBuffer(FBuffer& Buffer);

    ImagePtr CreateImage2D(uint32_t Width, uint32_t Height, bool bMipMapsRequired, VkSampleCountFlagBits NumSamples, VkFormat Format,
                               VkImageTiling Tiling, VkImageUsageFlags Usage, VkMemoryPropertyFlags Properties,
                               VkImageAspectFlags AspectFlags, VkDevice Device, const std::string& DebugImageName);
    ImagePtr LoadImageFromFile(const std::string& Path, const std::string& DebugImageName);
    ImagePtr Wrap(VkImage ImageToWrap, VkFormat Format, VkImageAspectFlags AspectFlags, VkDevice LogicalDevice, const std::string& DebugImageName);
    void SaveImage(const FImage& Image);
    template <typename T>
    void FetchImageData(const FImage& Image, std::vector<T>& Data);

    VkSampler CreateTextureSampler(uint32_t MipLevel);

    VkFramebuffer CreateFramebuffer(std::vector<ImagePtr> Images, VkRenderPass RenderPass, const std::string& debug_name);

    VkDescriptorPool CreateDescriptorPool(const std::map<VkDescriptorType, uint32_t>& DescriptorsMap, VkDevice LogicalDevice, const std::string& debug_name);

    VkShaderModule CreateShaderFromFile(const std::string& FileName);
    VkRenderPass CreateRenderpass(VkDevice LogicalDevice, FGraphicsPipelineOptions& GraphicsPipelineOptions);
    VkPipeline CreateGraphicsPipeline(VkShaderModule VertexShader, VkShaderModule FragmentShader, std::uint32_t Width, std::uint32_t Height, FGraphicsPipelineOptions& GraphicsPipelineOptions);

    VkSemaphore CreateSemaphore();
    VkFence CreateSignalledFence();
    VkFence CreateUnsignalledFence();

    static VKAPI_ATTR VkBool32 VKAPI_CALL FVulkanContext::DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT MessageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallBackData,
            void* pUserData);

public:
    VkQueue GetQueue(VkQueueFlagBits QueueFlagBits);
    uint32_t GetQueueIndex(VkQueueFlagBits QueueFlagBits);

    std::shared_ptr<FResourceAllocator> ResourceAllocator = nullptr;
    std::shared_ptr<FCommandBufferManager> CommandBufferManager = nullptr;

    VkSampleCountFlagBits MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    VkInstance Instance = VK_NULL_HANDLE;
    VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
    VkDevice LogicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR Surface = VK_NULL_HANDLE;
    GLFWwindow* Window;

    /// Queues
    struct IndexQueue
    {
        uint32_t QueueIndex = UINT32_MAX;
        VkQueue Queue = VK_NULL_HANDLE;
    };
    std::map <VkQueueFlagBits, IndexQueue> Queues = {{VK_QUEUE_GRAPHICS_BIT, {UINT32_MAX, VK_NULL_HANDLE}},
                                                  {VK_QUEUE_COMPUTE_BIT,  {UINT32_MAX, VK_NULL_HANDLE}},};
    IndexQueue PresentQueue;

    // SwapChain
    std::shared_ptr<FSwapchain> Swapchain = nullptr;
    VkImage CurrentImage = VK_NULL_HANDLE;

    VkDescriptorPool ImGuiDescriptorPool;
    std::vector<VkFramebuffer> ImGuiFramebuffers;

    std::shared_ptr<FRenderTask> RenderTask = nullptr;
    std::shared_ptr<FPassthroughTask> PassthroughTask = nullptr;
    std::shared_ptr<FImguiTask> ImguiTask = nullptr;

#ifndef NDEBUG
    VkDebugUtilsMessengerEXT DebugMessenger;
#endif
    /// Images for drawing
    ImagePtr UtilityImageR32;
    ImagePtr UtilityImageR8G8B8A8_SRGB;

    /// Texture used to pain the model
    ImagePtr TextureImage;

    uint32_t MipLevels;
    std::shared_ptr<FDescriptorSetManager>DescriptorSetManager = nullptr;

    std::vector<FBuffer> DeviceTransformBuffers;
    std::vector<FBuffer> DeviceCameraBuffers;
    std::vector<FBuffer> DeviceRenderableBuffers;

    std::vector<VkSemaphore> ImageAvailableSemaphores;
    std::vector<VkSemaphore> ImGuiFinishedSemaphores;
    std::vector<VkFence> ImagesInFlight;
    size_t CurrentFrame = 0;
    uint32_t ImageIndex = 0;

    const int MAX_FRAMES_IN_FLIGHT = 2;
    bool bFramebufferResized = false;

    std::string ModelPath = "../models/viking_room/viking_room.obj";
    std::string TexturePath = "../models/viking_room/viking_room.png";
};

FVulkanContext& GetContext();