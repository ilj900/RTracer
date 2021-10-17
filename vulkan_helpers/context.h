#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"

#include "resource_allocation.h"
#include "function_loader.h"
#include "maths.h"
#include "model.h"
#include "swapchain.h"
#include "controller.h"

#include <vector>
#include <string>
#include <array>
#include <memory>
#include <map>

class FContext
{
public:
    FContext(GLFWwindow* Window, FController* Controller);
    ~FContext();

    void Init();

    void CreateInstance();
    void SetupDebugMessenger();
    void CreateSurface();
    void PickPhysicalDevice();
    void QueuePhysicalDeviceProperties();
    void CreateLogicalDevice();
    void CreateDepthAndAAImages();
    void CreateRenderPass();
    void CreateDescriptorSetLayouts();
    void CreateGraphicsPipeline();
    void CreateCommandPool();
    void CreateFramebuffers();
    void CreateTextureImage(std::string& TexturePath);
    void CreateTextureImageView();
    void CreateTextureSampler();
    void CreateUniformBuffers();
    void CreateDescriptorPool();
    void CreateDescriptorSet();
    void CreateCommandBuffers();
    void CreateSyncObjects();
    void RecreateSwapChain();
    void CleanUpSwapChain();
    void CleanUp();
    void DestroyDebugUtilsMessengerEXT();
    void UpdateUniformBuffer(uint32_t CurrentImage);

    void Render();
    void Present();
    void WaitIdle();

    void CreateImage(uint32_t Width, uint32_t Height, uint32_t MipLevels, VkSampleCountFlagBits NumSamples, VkFormat Format, VkImageTiling Tiling, VkImageUsageFlags Usage, VkMemoryPropertyFlags Properties, VkImage& Image, VkDeviceMemory& ImageMemory);
    VkImageView CreateImageView(VkImage Image, VkFormat Format, VkImageAspectFlags AspectFlags, uint32_t MipLevels);
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& Candidates, VkImageTiling Tiling, VkFormatFeatureFlags Features);
    VkShaderModule CreateShaderFromFile(const std::string& FileName);
    VkFormat FindDepthFormat();
    void TransitionImageLayout(VkImage Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout, uint32_t MipLevels);
    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommand(VkCommandBuffer CommandBuffer);
    bool HasStensilComponent(VkFormat Format);
    void CopyBufferToImage(FBuffer &Buffer, VkImage Image, uint32_t Width, uint32_t Height);
    void GenerateMipmaps(VkImage Image, VkFormat ImageFormat, int32_t TexWidth, int32_t TexHeight, uint32_t mipLevels);
    void LoadDataIntoBuffer(FBuffer &Buffer, void* Data, size_t Size);
    void AddDescriptor(VkDescriptorType Type, uint32_t Count);

    bool CheckDeviceExtensionsSupport(VkPhysicalDevice Device);
    bool CheckDeviceQueueSupport(VkPhysicalDevice Device);

public:
    GLFWwindow* Window = nullptr;
    FController* Controller = nullptr;

    std::shared_ptr<FResourceAllocator> ResourceAllocator = nullptr;
    std::shared_ptr<FVulkanFunctionLoader> FunctionLoader = nullptr;
    std::vector<FModel> Models;

    std::vector<std::string> InstanceExtensions;
    std::vector<std::string> DeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    std::vector<std::string> ValidationLayers = {"VK_LAYER_KHRONOS_validation"};

    VkSurfaceCapabilitiesKHR Capabilities;
    std::vector<VkSurfaceFormatKHR> Formats;
    std::vector<VkPresentModeKHR> PresentModes;

    VkSampleCountFlagBits MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    VkInstance Instance;
    VkSurfaceKHR Surface;
    VkPhysicalDevice PhysicalDevice;
    VkDevice LogicalDevice;

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

    VkRenderPass RenderPass;

    VkDescriptorSetLayout FrameDescriptorSetLayout;
    VkDescriptorSetLayout RenderableDescriptorSetLayout;

    VkPipelineLayout PipelineLayout;
    VkPipeline GraphicsPipeline;

    VkCommandPool CommandPool;

    VkDebugUtilsMessengerEXT DebugMessenger;

    VkImage ColorImage;
    VkDeviceMemory ColorImageMemory;
    VkImageView ColorImageView;

    VkImage DepthImage;
    VkDeviceMemory DepthImageMemory;
    VkImageView DepthImageView;

    VkImage TextureImage;
    VkDeviceMemory TextureImageMemory;
    VkImageView TextureImageView;

    VkSampler TextureSampler;

    uint32_t  MipLevels;

    std::map<VkDescriptorType, uint32_t> Descriptors;
    VkDescriptorPool DescriptorPool;
    std::vector<VkDescriptorSet> RenderebleDescriptorSet;
    std::vector<VkDescriptorSet> FrameDescriptorSet;

    std::vector<VkCommandBuffer> CommandBuffers;

    std::vector<FBuffer> DeviceTransformBuffers;
    std::vector<FBuffer> DeviceCameraBuffers;

    std::vector<VkSemaphore> ImageAvailableSemaphores;
    std::vector<VkSemaphore> RenderFinishedSemaphores;
    std::vector<VkFence> RenderingFinishedFences;
    std::vector<VkFence> ImagesInFlight;
    size_t CurrentFrame = 0;
    uint32_t ImageIndex = 0;

    VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo = {};

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

std::vector<char> ReadFile(const std::string& FileName);