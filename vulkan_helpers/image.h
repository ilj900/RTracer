#pragma once

#include "buffer.h"

#include <functional>
#include <memory>
#include <string>

struct FImage
{
    FImage(uint32_t Width, uint32_t Height, bool bMipMapsRequired, VkSampleCountFlagBits NumSamples, VkFormat Format,
           VkImageTiling Tiling, VkImageUsageFlags Usage, VkMemoryPropertyFlags Properties,
           VkImageAspectFlags AspectFlags, VkDevice Device, const std::string& DebugImageName);
    FImage(VkImage ImageToWrap, VkFormat Format, VkImageAspectFlags AspectFlags, VkDevice LogicalDevice, const std::string& DebugImageName);
    ~FImage();

    FImage(const FImage&) = delete;
    void operator=(const FImage&) = delete;

    void Transition(VkImageLayout  OldLayout, VkImageLayout NewLayout);
    void SwapData(FImage& Other);

    VkImage Image = VK_NULL_HANDLE;
    FMemoryRegion MemoryRegion;
    VkImageView View = VK_NULL_HANDLE;

    VkFormat Format = VK_FORMAT_R8G8B8A8_SRGB;
    VkSampleCountFlagBits Samples= VK_SAMPLE_COUNT_1_BIT;
    VkDevice Device = VK_NULL_HANDLE;
    VkImageTiling Tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags Usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkMemoryPropertyFlags Properties;
    VkImageAspectFlags AspectFlags;
    VkDeviceSize Size;
    bool bInitialized = false;
    std::string DebugName;

    uint32_t Width = 0;
    uint32_t Height = 0;
    uint32_t MipLevels = 1;

    /// We might need to wrap images provided to us
    /// For example swapchain images
    bool bIsWrappedImage = false;

    void GenerateMipMaps();
    void Resolve(FImage& Image);

    /// In case we need to put an image into a map or set
    size_t GetHash();
    friend bool operator<(const FImage& A, const FImage& B);

private:
    void CreateImage();
    void AllocateMemory();
    void BindMemoryToImage();
    void CreateImageView();
};

/// FImage hash function, to we could use them in maps/sets
template<>
struct std::hash<FImage>
{
    size_t operator()(const FImage& Image)
    {
        return std::hash<VkImage>()(Image.Image);
    }
};

typedef std::shared_ptr<FImage> ImagePtr;
