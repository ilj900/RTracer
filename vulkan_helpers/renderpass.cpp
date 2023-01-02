#include "renderpass.h"
#include "vk_context.h"

#include <stdexcept>

void FRenderPass::AddImageAsAttachment(ImagePtr& Image, AttachmentType Type, VkImageLayout InitialLayout, VkImageLayout FinalLayout, VkAttachmentLoadOp AttachmentLoadOp)
{
    std::vector<VkAttachmentDescription>* AttachmentDescriptions;
    std::map<size_t , uint32_t>* ImageToIndexMap;

    switch (Type)
    {
        case AttachmentType::Color:
        {
            AttachmentDescriptions = &ColorAttachmentDescriptions;
            ImageToIndexMap = &ColorImageToIndexMap;
            break;
        }
        case AttachmentType::Resolve:
        {
            AttachmentDescriptions = &ResolvedAttachmentDescriptions;
            ImageToIndexMap = &ResolvedImageToIndexMap;
            break;
        }
        case AttachmentType::DepthStencil:
        {
            AttachmentDescriptions = &DepthStencilAttachmentDescriptions;
            ImageToIndexMap = &DepthStencilImageToIndexMap;
        }
    }

    if (ImageToIndexMap->find(Image->GetHash()) != ImageToIndexMap->end())
    {
        throw std::runtime_error("The image you are trying to add to Renderpass is already added as attachment.");
    }

    VkAttachmentDescription AttachmentDescription{};
    AttachmentDescription.format = Image->Format;
    AttachmentDescription.samples = Image->Samples;
    AttachmentDescription.loadOp = AttachmentLoadOp;
    AttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    AttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    AttachmentDescription.initialLayout = InitialLayout;
    if(Type == AttachmentType::DepthStencil)
    {
        AttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        AttachmentDescription.finalLayout = FinalLayout;
    }
    else
    {
        AttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        AttachmentDescription.finalLayout = FinalLayout;
    }

    AttachmentDescriptions->emplace_back(AttachmentDescription);

    (*ImageToIndexMap)[Image->GetHash()] = static_cast<uint32_t>(AttachmentDescriptions->size() - 1);
}

void FRenderPass::Construct(VkDevice LogicalDevice)
{
    this->LogicalDevice = LogicalDevice;

    std::vector<VkAttachmentDescription> AllAttachments;

    AllAttachments.insert(AllAttachments.end(), ColorAttachmentDescriptions.begin(), ColorAttachmentDescriptions.end());

    uint32_t DepthAttachmentOffset = AllAttachments.size();
    AllAttachments.insert(AllAttachments.end(), DepthStencilAttachmentDescriptions.begin(), DepthStencilAttachmentDescriptions.end());

    uint32_t  ResolvedAttachmentOffset = AllAttachments.size();
    AllAttachments.insert(AllAttachments.end(), ResolvedAttachmentDescriptions.begin(), ResolvedAttachmentDescriptions.end());

    std::vector<VkAttachmentReference> ColorAttachmentReferences(ColorAttachmentDescriptions.size());
    for (uint32_t i = 0; i < ColorAttachmentDescriptions.size(); ++i)
    {
        ColorAttachmentReferences[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        ColorAttachmentReferences[i].attachment = i;
    }

    VkAttachmentReference DepthAttachmentReference{};
    DepthAttachmentReference.attachment = DepthAttachmentOffset;
    DepthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    auto ColorAttachmentsCount = ColorAttachmentDescriptions.size();
    std::vector<VkAttachmentReference> ResolvedAttachmentReferences(ColorAttachmentsCount, {VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    for (uint32_t i = 0; i < ResolvedAttachmentDescriptions.size(); ++i)
    {
        ResolvedAttachmentReferences[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        ResolvedAttachmentReferences[i].attachment = i + ResolvedAttachmentOffset;
    }

    VkSubpassDependency Dependency{};
    Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    Dependency.dstSubpass = 0;
    Dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    Dependency.srcAccessMask = 0;
    Dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkSubpassDescription Subpass{};
    Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    Subpass.colorAttachmentCount = ColorAttachmentReferences.size();
    Subpass.pColorAttachments = (ColorAttachmentReferences.size() > 0) ? ColorAttachmentReferences.data() : nullptr;
    Subpass.pDepthStencilAttachment = (DepthStencilAttachmentDescriptions.size() > 0) ? &DepthAttachmentReference : nullptr;
    Subpass.pResolveAttachments = (ResolvedAttachmentReferences.size() > 0) ? ResolvedAttachmentReferences.data() : nullptr;

    VkRenderPassCreateInfo RenderPassInfo{};
    RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    RenderPassInfo.attachmentCount = static_cast<uint32_t>(AllAttachments.size());
    RenderPassInfo.pAttachments = AllAttachments.data();
    RenderPassInfo.subpassCount = 1;
    RenderPassInfo.pSubpasses = &Subpass;
    RenderPassInfo.dependencyCount = 1;
    RenderPassInfo.pDependencies  = &Dependency;

    if (vkCreateRenderPass(LogicalDevice, &RenderPassInfo, nullptr, &RenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create render pass!");
    }
}


FRenderPass::~FRenderPass()
{
    vkDestroyRenderPass(LogicalDevice, RenderPass, nullptr);
}
