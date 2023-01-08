#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

enum class eShaderType {VERTEX, FRAGMENT, COMPUTE, RAYGEN, RAYMISS, RAYCLOSEHIT};

class FPipeline
{
public:
    FPipeline() = default;

    void AddVertexInputBindingDescription(const VkVertexInputBindingDescription& VertexInputBindingDescription);
    void AddVertexInputAttributeDescription(const VkVertexInputAttributeDescription& VertexInputAttributeDescription);
    void SetMSAA(VkSampleCountFlagBits SampleCountFlagBits);
    void SetExtent2D(VkExtent2D Extent2D);
    void AddDescriptorSetLayout(VkDescriptorSetLayout DescriptorSetLayout);
    void SetWidth(uint32_t Wdth);
    void SetHeight(uint32_t Hght);
    void SetBlendAttachmentsCount(uint32_t Count);
    VkPipelineLayout GetPipelineLayout();
    VkPipeline GetPipeline();

    VkPipeline CreateGraphicsPipeline(VkDevice Device, VkRenderPass RenderPass);
    VkPipeline CreateRayTracingPipeline(VkDevice LogicalDevice);

    void Delete();

    ~FPipeline();

private:
    VkPipelineLayout PipelineLayout;
    VkPipeline Pipeline;
    VkDevice LogicalDevice;

    struct FShaderModule
    {
        VkShaderModule ShaderModule;
        eShaderType ShaderType;
    };

    std::vector<FShaderModule> ShaderModules;

    std::vector<VkVertexInputBindingDescription> VertexInputBindingDescriptionVector{};
    std::vector<VkVertexInputAttributeDescription> VertexInputAttributeDescriptionVector{};

    uint32_t Width = 1920;
    uint32_t Height = 1080;

    VkSampleCountFlagBits MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    VkExtent2D Extent = {1920, 1080};

    std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;

    uint32_t BlendCount = 0;
};

struct FGraphicsPipelineOptions
{
    void RegisterColorAttachment(uint32_t Index, ImagePtr& Image, VkImageLayout InitialLayout, VkImageLayout FinalLayout, VkAttachmentLoadOp AttachmentLoadOp);
    void RegisterDepthStencilAttachment(ImagePtr& Image, VkImageLayout InitialLayout, VkImageLayout FinalLayout, VkAttachmentLoadOp AttachmentLoadOp);
    void RegisterResolveAttachment(uint32_t Index, ImagePtr& Image, VkImageLayout InitialLayout, VkImageLayout FinalLayout, VkAttachmentLoadOp AttachmentLoadOp);

    void AddVertexInputBindingDescription(const VkVertexInputBindingDescription& VertexInputBindingDescription);
    void AddVertexInputAttributeDescription(const VkVertexInputAttributeDescription& VertexInputAttributeDescription);
    void SetPipelineLayout(VkPipelineLayout PipelineLayout);
    void SetMSAA(VkSampleCountFlagBits SampleCountFlagBits);

    std::vector<VkVertexInputBindingDescription> VertexInputBindingDescriptionVector{};
    std::vector<VkVertexInputAttributeDescription> VertexInputAttributeDescriptionVector{};

    std::vector<VkAttachmentDescription> ColorAttachmentDescriptions;
    std::vector<VkAttachmentDescription> ResolveAttachmentDescriptions;
    VkAttachmentDescription DepthStencilAttachmentDescriptions{};

    VkSampleCountFlagBits MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    VkExtent2D Extent = {1920, 1080};

    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkRenderPass RenderPass = VK_NULL_HANDLE;
};