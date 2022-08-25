#pragma once

#include "image.h"

#include <vector>
#include <map>

enum class AttachmentType {Color, Resolve, DepthStencil};

class FRenderPass
{
public:
    FRenderPass() = default;
    ~FRenderPass();

    void AddImageAsAttachment(FImage& Image, AttachmentType Type, VkImageLayout InitialLayout, VkImageLayout FinalLayout);
    void Construct(VkDevice LogicalDevice);

public:
    std::vector<VkAttachmentDescription> ColorAttachmentDescriptions;
    std::map<size_t, uint32_t> ColorImageToIndexMap;

    std::vector<VkAttachmentDescription> ResolvedAttachmentDescriptions;
    std::map<size_t, uint32_t> ResolvedImageToIndexMap;

    std::vector<VkAttachmentDescription> DepthStencilAttachmentDescriptions;
    std::map<size_t, uint32_t> DepthStencilImageToIndexMap;

    VkDevice LogicalDevice;
    VkRenderPass RenderPass;
};