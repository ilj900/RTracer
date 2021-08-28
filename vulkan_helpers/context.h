#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"

#include "resource_allocation.h"
#include "maths.h"

#include <vector>
#include <string>
#include <array>
#include <memory>

struct FVertex {
    FVector3 Pos;
    FVector3 Color;
    FVector2 TexCoord;

    static VkVertexInputBindingDescription GetBindingDescription()
    {
        VkVertexInputBindingDescription BindingDescription{};
        BindingDescription.binding = 0;
        BindingDescription.stride = sizeof(FVertex);
        BindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return BindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> AttributeDescription{};
        AttributeDescription[0].binding = 0;
        AttributeDescription[0].location = 0;
        AttributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        AttributeDescription[0].offset = offsetof(FVertex, Pos);

        AttributeDescription[1].binding = 0;
        AttributeDescription[1].location = 1;
        AttributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        AttributeDescription[1].offset = offsetof(FVertex, Color);

        AttributeDescription[2].binding = 0;
        AttributeDescription[2].location = 2;
        AttributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
        AttributeDescription[2].offset = offsetof(FVertex, TexCoord);

        return AttributeDescription;
    }

    bool operator==(const FVertex& Other) const
    {
        return Pos == Other.Pos && Color == Other.Color && TexCoord == Other.TexCoord;
    }

};

template<>
struct std::hash<FVertex>
{
    size_t operator()(FVertex const& Vertex) const;
};

struct UniformBufferObject
{
    alignas(16) FMatrix4 Model;
    alignas(16) FMatrix4 View;
    alignas(16) FMatrix4 Projection;
};

namespace V
{
    class FContext
    {
    public:
        FContext(GLFWwindow* Window);
        ~FContext();

        void Init();

        void CreateInstance();
        void SetupDebugMessenger();
        void CreateSurface();
        void PickPhysicalDevice();
        void QueuePhysicalDeviceProperties();
        void CreateLogicalDevice();
        void CreateSwapChain();
        void CreateRenderPass();
        void CreateDescriptorSetLayout();
        void CreateGraphicsPipeline();
        void CreateCommandPool();
        void CreateFramebuffers();
        void CreateTextureImage(std::string& TexturePath);
        void CreateTextureImageView();
        void CreateTextureSampler();
        void LoadModel(const std::string& ModelPath);
        void CreateVertexBuffer();
        void CreateIndexBuffer();
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

        void DrawFrame();

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
        void CopyBuffer(FBuffer &SrcBuffer, FBuffer &DstBuffer, VkDeviceSize Size);

        bool CheckDeviceExtensionsSupport(VkPhysicalDevice Device);
        bool CheckDeviceQueueSupport(VkPhysicalDevice Device);

    public:
        GLFWwindow* Window = nullptr;

        std::shared_ptr<FResourceAllocator> ResourceAllocator = nullptr;

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
        VkSwapchainKHR SwapChain;
        std::vector<VkImage> SwapChainImages;
        VkFormat SwapChainImageFormat;
        VkExtent2D SwapChainExtent;
        std::vector<VkImageView> SwapChainImageViews;
        std::vector<VkFramebuffer> SwapChainFramebuffers;

        VkRenderPass RenderPass;

        VkDescriptorSetLayout DescriptorSetLayout;

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

        FBuffer StagingBuffer;

        uint32_t  MipLevels;

        FBuffer VertexBuffer;

        FBuffer IndexBuffer;

        VkDescriptorPool DescriptorPool;
        std::vector<VkDescriptorSet> DescriptorSets;

        std::vector<VkCommandBuffer> CommandBuffers;

        std::vector<FVertex> Vertices;
        std::vector<uint32_t> Indices;

        std::vector<FBuffer> UniformBuffers;

        std::vector<VkSemaphore> ImageAvailableSemaphores;
        std::vector<VkSemaphore> RenderFinishedSemaphores;
        std::vector<VkFence> InFlightFences;
        std::vector<VkFence> ImagesInFlight;
        size_t CurrentFrame = 0;

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
}

std::vector<char> ReadFile(const std::string& FileName);