#pragma once

#include "vulkan/vulkan.h"

#include <functional>

struct FImage
{
    FImage() = default;
    FImage(uint32_t Width, uint32_t Height, uint32_t MipLevels, VkSampleCountFlagBits NumSamples, VkFormat Format, VkImageTiling Tiling, VkImageUsageFlags Usage, VkMemoryPropertyFlags Properties, VkImageAspectFlags AspectFlags, VkDevice Device);
    ~FImage();

    void Transition(VkImageLayout  OldLayout, VkImageLayout NewLayout);


    VkImage Image;
    VkDeviceMemory Memory;
    VkImageView View;

    VkFormat Format;
    VkSampleCountFlagBits Samples= VK_SAMPLE_COUNT_1_BIT;
    VkDevice Device;

    uint32_t Width;
    uint32_t Height;
    uint32_t MipLevels;

    bool bIsWrappedImage = false;

    size_t GetHash();

    static void Wrap(VkImage ImageToWrap, VkFormat Format, VkImageAspectFlags AspectFlags, uint32_t MipLevels, VkDevice LogicalDevice, FImage& Image);

    friend bool operator<(const FImage& A, const FImage& B);
};

/// FVertex hash function, to we could use them in maps/sets
template<>
struct std::hash<FImage>
{
    size_t operator()(const FImage& Image)
    {
        return std::hash<VkImage>()(Image.Image);
    }
};