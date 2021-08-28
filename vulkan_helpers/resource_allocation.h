#pragma once

#include "vulkan/vulkan.h"

struct FBuffer
{
    VkBuffer Buffer = VK_NULL_HANDLE;
    VkDeviceMemory Memory = VK_NULL_HANDLE;
};

class FResourceAllocator
{
public:
    FResourceAllocator(VkPhysicalDevice PhysicalDevice, VkDevice Device);
    FBuffer CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties);
    void DestroyBuffer(FBuffer& Buffer);
    uint32_t FindMemoryType(uint32_t TypeFilter, VkMemoryPropertyFlags Properties);

private:
    VkDevice Device = VK_NULL_HANDLE;

    VkPhysicalDeviceMemoryProperties MemProperties{};
};