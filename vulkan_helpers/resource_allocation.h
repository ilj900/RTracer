#pragma once

#include "vulkan/vulkan.h"

class FContext;

#include <memory>

struct FBuffer
{
    VkBuffer Buffer = VK_NULL_HANDLE;
    VkDeviceMemory Memory = VK_NULL_HANDLE;
    VkDeviceSize Size = 0;
};

class FResourceAllocator
{
public:
    FResourceAllocator(VkPhysicalDevice PhysicalDevice, VkDevice Device, FContext* Context);

    FBuffer CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties);
    void CopyBuffer(FBuffer &SrcBuffer, FBuffer &DstBuffer, VkDeviceSize Size);
    void DestroyBuffer(FBuffer& Buffer);
    uint32_t FindMemoryType(uint32_t TypeFilter, VkMemoryPropertyFlags Properties);

private:
    VkDevice Device = VK_NULL_HANDLE;
    FContext *Context = nullptr;

    VkPhysicalDeviceMemoryProperties MemProperties{};
};