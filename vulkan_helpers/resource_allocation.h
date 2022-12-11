#pragma once

#include "buffer.h"

#include <memory>

class FVulkanContext;

class FResourceAllocator
{
public:
    FResourceAllocator(VkPhysicalDevice PhysicalDevice, VkDevice Device, FVulkanContext* Context);
    ~FResourceAllocator();

    FBuffer CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties);
    FBuffer CreateBufferWidthData(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties, void* Data);
    FBuffer LoadDataToBuffer(FBuffer& Buffer, VkDeviceSize Size, VkDeviceSize Offset, void* Data);
    void CopyBuffer(FBuffer &SrcBuffer, FBuffer &DstBuffer, VkDeviceSize Size, VkDeviceSize SourceOffset, VkDeviceSize DestinationOffset);
    void DestroyBuffer(FBuffer& Buffer);
    uint32_t FindMemoryType(uint32_t TypeFilter, VkMemoryPropertyFlags Properties);

private:
    VkDeviceSize StagingBufferSize = 256 * 1024 * 1024;
    FBuffer StagingBuffer;

    VkDevice Device = VK_NULL_HANDLE;
    FVulkanContext *Context = nullptr;

    VkPhysicalDeviceMemoryProperties MemProperties{};
};