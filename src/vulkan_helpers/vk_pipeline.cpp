#include "vk_context.h"
#include "vk_pipeline.h"
#include "vk_functions.h"

#include <map>

void FGraphicsPipelineOptions::RegisterColorAttachment(uint32_t Index, ImagePtr& Image, VkImageLayout InitialLayout, VkImageLayout FinalLayout, VkAttachmentLoadOp AttachmentLoadOp)
{
    if (Index >= ColorAttachmentDescriptions.size())
    {
        ColorAttachmentDescriptions.resize(Index + 1);
    }

    VkAttachmentDescription AttachmentDescription{};
    AttachmentDescription.format = Image->Format;
    AttachmentDescription.samples = Image->Samples;
    AttachmentDescription.loadOp = AttachmentLoadOp;
    AttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    AttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    AttachmentDescription.initialLayout = InitialLayout;
    AttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    AttachmentDescription.finalLayout = FinalLayout;

    ColorAttachmentDescriptions[Index] = AttachmentDescription;
}

void FGraphicsPipelineOptions::RegisterDepthStencilAttachment(ImagePtr& Image, VkImageLayout InitialLayout, VkImageLayout FinalLayout, VkAttachmentLoadOp AttachmentLoadOp)
{
    DepthStencilAttachmentDescriptions = VkAttachmentDescription{};
    DepthStencilAttachmentDescriptions.format = Image->Format;
    DepthStencilAttachmentDescriptions.samples = Image->Samples;
    DepthStencilAttachmentDescriptions.loadOp = AttachmentLoadOp;
    DepthStencilAttachmentDescriptions.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    DepthStencilAttachmentDescriptions.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    DepthStencilAttachmentDescriptions.initialLayout = InitialLayout;
    DepthStencilAttachmentDescriptions.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    DepthStencilAttachmentDescriptions.finalLayout = FinalLayout;
}

void FGraphicsPipelineOptions::RegisterResolveAttachment(uint32_t Index, ImagePtr& Image, VkImageLayout InitialLayout, VkImageLayout FinalLayout, VkAttachmentLoadOp AttachmentLoadOp)
{
    if (Index >= ResolveAttachmentDescriptions.size())
    {
        ResolveAttachmentDescriptions.resize(Index + 1);
    }

    VkAttachmentDescription ResolveAttachmentDescription{};
    ResolveAttachmentDescription.format = Image->Format;
    ResolveAttachmentDescription.samples = Image->Samples;
    ResolveAttachmentDescription.loadOp = AttachmentLoadOp;
    ResolveAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    ResolveAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    ResolveAttachmentDescription.initialLayout = InitialLayout;
    ResolveAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ResolveAttachmentDescription.finalLayout = FinalLayout;

    ResolveAttachmentDescriptions[Index] = ResolveAttachmentDescription;
}

void FGraphicsPipelineOptions::AddVertexInputBindingDescription(const VkVertexInputBindingDescription& VertexInputBindingDescription)
{
    VertexInputBindingDescriptionVector.push_back(VertexInputBindingDescription);
}

void FGraphicsPipelineOptions::AddVertexInputAttributeDescription(const VkVertexInputAttributeDescription& VertexInputAttributeDescription)
{
    VertexInputAttributeDescriptionVector.push_back(VertexInputAttributeDescription);
}

void FGraphicsPipelineOptions::SetPipelineLayout(VkPipelineLayout PipelineLayout)
{
    this->PipelineLayout = PipelineLayout;
}

void FGraphicsPipelineOptions::SetMSAA(VkSampleCountFlagBits SampleCountFlagBits)
{
    MSAASamples = SampleCountFlagBits;
}
