#pragma once

#include "buffer.h"
#include "image.h"

#include <memory>

class FVulkanContext;

class FResourceAllocator
{
public:
    FResourceAllocator(VkPhysicalDevice PhysicalDevice, VkDevice Device, FVulkanContext* Context);
    ~FResourceAllocator();

    FMemoryRegion AllocateMemory(VkDeviceSize Size, VkMemoryRequirements MemRequirements, VkMemoryPropertyFlags Properties, bool bDeviceAddressRequired = false);

    FBuffer CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties, const std::string& DebugName = "");
    FMemoryPtr PushDataToBuffer(FBuffer& Buffer, VkDeviceSize Size, void* Data);
    FBuffer LoadDataToBuffer(FBuffer& Buffer, VkDeviceSize Size, VkDeviceSize Offset, void* Data);
    void LoadDataFromBuffer(FBuffer& Buffer, VkDeviceSize Size, VkDeviceSize Offset, void* Data);
    void LoadDataToStagingBuffer(VkDeviceSize Size, void* Data);
    void LoadDataFromStaginBuffer(VkDeviceSize Size, void* Data);
    void CopyBuffer(FBuffer &SrcBuffer, FBuffer &DstBuffer, VkDeviceSize Size, VkDeviceSize SourceOffset, VkDeviceSize DestinationOffset);
    void* Map(FBuffer& Buffer);
    void Unmap(FBuffer& Buffer);

    FMemoryRegion LoadDataToImage(FImage& Image, VkDeviceSize Size, void* Data);
    void CopyBufferToImage(FBuffer& SrcBuffer, FImage& DstImage);
    void CopyImageToBuffer(const FImage& SrcImage, FBuffer& DstBuffer);
    void GetImageData(FImage& SrcImage, void* Data);
    void DestroyBuffer(FBuffer& Buffer);

    uint32_t FindMemoryType(uint32_t TypeFilter, VkMemoryPropertyFlags Properties);

    VkDeviceSize StagingBufferSize = uint64_t(256) * 1024 * 1024;
    VkDeviceSize MeshBufferSize = uint64_t(2) * 1024 * 1024 * 1024;

    FBuffer StagingBuffer;
    FBuffer MeshBuffer;

    VkDevice Device = VK_NULL_HANDLE;
    FVulkanContext *Context = nullptr;

    VkPhysicalDeviceMemoryProperties MemProperties{};
};