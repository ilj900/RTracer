#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"
#include "vulkan_wrappers.h"

#include "resource_allocation.h"
#include "function_loader.h"
#include "command_buffer_manager.h"
#include "maths.h"
#include "swapchain.h"
#include "controller.h"
#include "buffer.h"
#include "descriptors.h"
#include "image.h"
#include "image_manager.h"
#include "renderpass.h"

#include <vector>
#include <string>
#include <array>
#include <memory>
#include <map>

class FContext
{
public:
    FContext() = default;
    FContext operator=(const FContext* Other) = delete;
    FContext(const FContext& Other) = delete;
    ~FContext();

    void Init(GLFWwindow* Window, FController* Controller);

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

public:
    V::FInstanceCreationOptions InstanceCreationOptions{};

    GLFWwindow* Window = nullptr;
    FController* Controller = nullptr;

    std::shared_ptr<FResourceAllocator> ResourceAllocator = nullptr;
    std::shared_ptr<FVulkanFunctionLoader> FunctionLoader = nullptr;
    std::shared_ptr<FCommandBufferManager> CommandBufferManager = nullptr;
    std::shared_ptr<FImageManager> ImageManager = nullptr;

    std::vector<std::string> DeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkSampleCountFlagBits MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    VkInstance Instance;
    VkSurfaceKHR Surface;
    VkPhysicalDevice PhysicalDevice;
    VkDevice LogicalDevice;

    std::shared_ptr<FRenderPass> RenderPass = nullptr;

    // Queues
    VkQueue GraphicsQueue;
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

    VkDebugUtilsMessengerCreateInfoEXT DebugUtilsMessengerCreateInfoEXT{};
    VkDebugUtilsMessengerEXT DebugMessenger;

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

    uint32_t GraphicsQueueIndex = UINT32_MAX;
    uint32_t PresentQueueIndex = UINT32_MAX;

    std::string ModelPath = "../models/viking_room/viking_room.obj";
    std::string TexturePath = "../models/viking_room/viking_room.png";
};

FContext& GetContext();

std::vector<char> ReadFile(const std::string& FileName);