#pragma once

#include <vulkan/vulkan.h>

#include "image.h"

#include <map>
#include <string>
#include <vector>

class FContext;
struct FBuffer;

struct NameImageEntry
{
    NameImageEntry(const std::string& Name, uint32_t Width, uint32_t Height, bool bMipMapsRequired, VkSampleCountFlagBits NumSamples, VkFormat Format, VkImageTiling Tiling, VkImageUsageFlags Usage, VkMemoryPropertyFlags Properties, VkImageAspectFlags AspectFlags,
                   VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties, VkDevice Device)
    : Image(Width, Height, bMipMapsRequired, NumSamples, Format, Tiling, Usage, Properties, AspectFlags, PhysicalDeviceMemoryProperties, Device),
      Name(Name) {};

    std::string Name;
    FImage Image;
};

class FImageManager
{
public:
    FImageManager() = default;
    ~FImageManager();

    void Init(FContext& Context);

    void LoadImageFromFile(const std::string& ImageName, const std::string& Path);
    void CreateImage(const std::string& ImageName, uint32_t Width, uint32_t Height, bool bMipMapsRequired, VkSampleCountFlagBits NumSamples, VkFormat Format, VkImageTiling Tiling, VkImageUsageFlags Usage, VkMemoryPropertyFlags Properties, VkImageAspectFlags AspectFlags);
    void RemoveImage(const std::string& ImageName);
    FImage& operator()(const std::string& ImageName);

    void CopyBufferToImage(const FBuffer& Buffer, const std::string& ImageName);
    void CopyImageToBuffer(const std::string& ImageName, const FBuffer& Buffer);
    void SaveImage(const std::string& ImageName);
    void FetchImageData(const std::string& ImageName, std::vector<char>& Data);

private:
    FContext* Context;
    VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties;

    std::map<std::string, uint32_t> NameToHashMap;
    std::map<uint32_t, uint32_t> HashToIndexMap;
    std::vector<NameImageEntry> Images;

};
