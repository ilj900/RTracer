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
    void CreateDescriptorSetLayouts();
    void CreateGraphicsPipeline();
    void CreateFramebuffers();
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
    VkShaderModule CreateShaderFromFile(const std::string& FileName);
    VkFormat FindDepthFormat();
    bool HasStensilComponent(VkFormat Format);
    void LoadDataIntoBuffer(FBuffer &Buffer, void* Data, size_t Size);
    void FreeData(FBuffer Buffer);

    bool CheckInstanceLayersSupport(const std::vector<const char*>& Layers);
    bool CheckDeviceExtensionsSupport(VkPhysicalDevice Device);
    bool CheckDeviceQueueSupport(VkPhysicalDevice Device);

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = nullptr;
    PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR = nullptr;
    PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR = nullptr;
    PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR = nullptr;
    PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR = nullptr;
    PFN_vkCmdWriteAccelerationStructuresPropertiesKHR vkCmdWriteAccelerationStructuresPropertiesKHR = nullptr;
    PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR = nullptr;
    PFN_vkCmdCopyAccelerationStructureKHR vkCmdCopyAccelerationStructureKHR = nullptr;
    PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR = nullptr;
    PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR = nullptr;
    PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR = nullptr;

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

    VkDescriptorPool ImGuiDescriptorPool;
    VkRenderPass ImGuiRenderPass;
    std::vector<VkFramebuffer> ImGuiFramebuffers;

    VkPipelineLayout PipelineLayout;
    VkPipeline GraphicsPipeline;

#ifndef NDEBUG
    VkDebugUtilsMessengerEXT DebugMessenger;
#endif
    /// Images for drawing
    std::string ColorImage = "ColorImage";
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

    std::vector<VkCommandBuffer> CommandBuffers;

    std::vector<FBuffer> DeviceTransformBuffers;
    std::vector<FBuffer> DeviceCameraBuffers;
    std::vector<FBuffer> DeviceRenderableBuffers;

    std::vector<VkSemaphore> ImageAvailableSemaphores;
    std::vector<VkSemaphore> RenderFinishedSemaphores;
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

std::vector<char> ReadFile(const std::string& FileName);