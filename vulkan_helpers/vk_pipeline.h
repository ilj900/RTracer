#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

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

    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkRenderPass RenderPass = VK_NULL_HANDLE;
};