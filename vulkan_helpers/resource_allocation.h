#pragma once

#include "buffer.h"

#include <memory>

class FVulkanContext;

class FResourceAllocator
{
public:
    FResourceAllocator(VkPhysicalDevice PhysicalDevice, VkDevice Device, FVulkanContext* Context);
    ~FResourceAllocator();

    FMemoryRegion AllocateMemory(VkDeviceSize Size, VkMemoryRequirements MemRequirements, VkMemoryPropertyFlags Properties);
    FBuffer CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties);
    FMemoryPtr PushDataToBuffer(FBuffer& Buffer, VkDeviceSize Size, void* Data);
    FBuffer LoadDataToBuffer(FBuffer& Buffer, VkDeviceSize Size, VkDeviceSize Offset, void* Data);
    void LoadDataFromBuffer(FBuffer& Buffer, VkDeviceSize Size, VkDeviceSize Offset, void* Data);
    void CopyBuffer(FBuffer &SrcBuffer, FBuffer &DstBuffer, VkDeviceSize Size, VkDeviceSize SourceOffset, VkDeviceSize DestinationOffset);
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