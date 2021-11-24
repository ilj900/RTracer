#pragma once

#include "vulkan/vulkan.h"

struct FImage
{
    FImage(uint32_t Width, uint32_t Height, uint32_t MipLevels, VkSampleCountFlagBits NumSamples, VkFormat Format, VkImageTiling Tiling, VkImageUsageFlags Usage, VkMemoryPropertyFlags Properties, VkImageAspectFlags AspectFlags, VkDevice Device);
    ~FImage();

    void Transition(VkImageLayout NewLayout);


    VkImage Image;
    VkDeviceMemory Memory;
    VkImageView View;

    VkDevice Device;
    VkImageLayout CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout OldLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    uint32_t Width;
    uint32_t Height;
};