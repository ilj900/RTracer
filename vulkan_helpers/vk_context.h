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
#include "image_manager.h"
#include "renderpass.h"
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

    void Init(GLFWwindow* Window, FController* Controller);

    void CreateInstance();
    void LoadFunctionPointers();
    void SetupDebugMessenger();
    void CreateSurface();
    void PickPhysicalDevice();
    void QueuePhysicalDeviceProperties();
    void CreateLogicalDevice();
    void CreateDepthAndAAImages();
    void CreateRenderPass();
    void CreatePassthroughRenderPass();
    void CreateDescriptorSetLayouts();
    void CreateGraphicsPipeline();
    void CreatePassthroughPipeline();
    void CreateRenderFramebuffers();
    void CreatePassthroughFramebuffers();
    void LoadModelDataToGPU();
    void CreateTextureSampler();
    void CreateUniformBuffers();
    void CreateDescriptorPool();
    void CreateDescriptorSet();
    void CreateCommandBuffers();
    void CreateSyncObjects();
    void CreateImguiContext();
    void RecreateSwapChain();
    void CleanUpSwapChain();
    void CleanUp();
    void DestroyDebugUtilsMessengerEXT();
    void UpdateUniformBuffer(uint32_t CurrentImage);

    void RenderImGui();
    void Render();
    void Present();
    void WaitIdle();

    VkFormat FindSupportedFormat(const std::vector<VkFormat>& Candidates, VkImageTiling Tiling, VkFormatFeatureFlags Features);
    VkFormat FindDepthFormat();
    bool HasStensilComponent(VkFormat Format);
    void LoadDataIntoBuffer(FBuffer &Buffer, void* Data, size_t Size);
    void FreeData(FBuffer Buffer);

    bool CheckInstanceLayersSupport(const std::vector<const char*>& Layers);
    bool CheckDeviceExtensionsSupport(VkPhysicalDevice Device);
    bool CheckDeviceQueueSupport(VkPhysicalDevice Device);

    VkInstance CreateVkInstance(const std::string& AppName, const FVersion3& AppVersion, const std::string& EngineName, const FVersion3& EngineVersion, uint32_t ApiVersion, FVulkanContextOptions& Options);

    static PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
    static PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
    static PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
    static PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR ;
    static PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
    static PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
    static PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
    static PFN_vkCmdWriteAccelerationStructuresPropertiesKHR vkCmdWriteAccelerationStructuresPropertiesKHR;
    static PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
    static PFN_vkCmdCopyAccelerationStructureKHR vkCmdCopyAccelerationStructureKHR;
    static PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
    static PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;
    static PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;

public:
    FVulkanContextOptions VulkanContextOptions;
    GLFWwindow* Window = nullptr;
    FController* Controller = nullptr;

    std::shared_ptr<FResourceAllocator> ResourceAllocator = nullptr;
    std::shared_ptr<FCommandBufferManager> CommandBufferManager = nullptr;
    std::shared_ptr<FImageManager> ImageManager = nullptr;

    std::vector<std::string> InstanceExtensions;
    std::vector<std::string> DeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    std::vector<std::string> ValidationLayers = {"VK_LAYER_KHRONOS_validation"};

    VkSampleCountFlagBits MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    VkInstance Instance;
    VkSurfaceKHR Surface;
    VkPhysicalDevice PhysicalDevice;
    VkDevice LogicalDevice;

    std::shared_ptr<FRenderPass> RenderPass = nullptr;
    std::shared_ptr<FRenderPass> PassthroughRenderPass = nullptr;

    // Queues
    VkQueue GraphicsQueue;
    VkQueue ComputeQueue;
    VkQueue TransferQueue;
    VkQueue SparseBindingQueue;
    VkQueue PresentQueue;

    // SwapChain
    std::shared_ptr<FSwapchain> Swapchain = nullptr;
    VkImage CurrentImage = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> SwapChainFramebuffers;
    std::vector<VkFramebuffer> PassthroughFramebuffers;

    VkDescriptorPool ImGuiDescriptorPool;
    VkRenderPass ImGuiRenderPass;
    std::vector<VkFramebuffer> ImGuiFramebuffers;

    FPipeline GraphicsPipeline;
    FPipeline PassthroughPipeline;

#ifndef NDEBUG
    VkDebugUtilsMessengerEXT DebugMessenger;
#endif
    /// Images for drawing
    std::string ColorImage = "ColorImage";
    std::string ResolvedColorImage = "ResolvedColorImage";
    std::string NormalsImage = "NormalsImage";
    std::string RenderableIndexImage = "RenderableIndexImage";
    std::string DepthImage = "DepthImage";
    std::string UtilityImageR32 = "UtilityImageR32";
    std::string UtilityImageR8G8B8A8_SRGB = "UtilityImageR8G8B8A8_SRGB";

    /// Texture used to pain the model
    std::string TextureImage = "TextureImage";

    VkSampler TextureSampler;

    uint32_t  MipLevels;
    std::shared_ptr<FDescriptorSetManager>DescriptorSetManager = nullptr;

    std::vector<VkCommandBuffer> GraphicsCommandBuffers;
    std::vector<VkCommandBuffer> PassthroughCommandBuffers;

    std::vector<FBuffer> DeviceTransformBuffers;
    std::vector<FBuffer> DeviceCameraBuffers;
    std::vector<FBuffer> DeviceRenderableBuffers;

    std::vector<VkSemaphore> ImageAvailableSemaphores;
    std::vector<VkSemaphore> RenderFinishedSemaphores;
    std::vector<VkSemaphore> PassthroughFinishedSemaphore;
    std::vector<VkSemaphore> ImGuiFinishedSemaphores;
    std::vector<VkFence> RenderingFinishedFences;
    std::vector<VkFence> ImGuiFinishedFences;
    std::vector<VkFence> ImagesInFlight;
    size_t CurrentFrame = 0;
    uint32_t ImageIndex = 0;

    const int MAX_FRAMES_IN_FLIGHT = 2;
    bool bFramebufferResized = false;

    bool bGraphicsCapabilityRequired = true;
    uint32_t GraphicsQueueIndex = UINT32_MAX;
    bool bComputeCapabilityRequired = false;
    uint32_t ComputeQueueIndex = UINT32_MAX;
    bool bTransferCapabilityRequired = false;
    uint32_t TransferQueueIndex = UINT32_MAX;
    bool bSparseBindingCapabilityRequired = false;
    uint32_t SparseBindingQueueIndex = UINT32_MAX;
    bool bPresentCapabilityRequired = true;
    uint32_t PresentQueueIndex = UINT32_MAX;

    std::string ModelPath = "../models/viking_room/viking_room.obj";
    std::string TexturePath = "../models/viking_room/viking_room.png";
};

FVulkanContext& GetContext();