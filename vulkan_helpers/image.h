#pragma once

#include "vulkan/vulkan.h"

struct FImage
{
    FImage(uint32_t Width, uint32_t Height, uint32_t MipLevels, VkSampleCountFlagBits NumSamples, VkFormat Format, VkImageTiling Tiling, VkImageUsageFlags Usage, VkMemoryPropertyFlags Properties, VkImageAspectFlags AspectFlags, VkDevice Device);
    ~FImage();

    void Transition(VkImageLayout  OldLayout, VkImageLayout NewLayout);


    VkImage Image;
    VkDeviceMemory Memory;
    VkImageView View;

    VkFormat Format;
    VkDevice Device;

    uint32_t Width;
    uint32_t Height;
    uint32_t MipLevels;
};