#pragma once

#include "vulkan/vulkan.h"

#include <vector>

struct FBuffer;

struct FMemoryRegion
{
    FBuffer* Buffer;
    VkDeviceSize Offset = 0;
    VkDeviceSize Size = 0;
};

struct FBuffer
{
    VkBuffer Buffer = VK_NULL_HANDLE;
    VkDeviceMemory Memory = VK_NULL_HANDLE;
    VkDeviceSize BufferSize = 0;
    VkDeviceSize CurrentOffset = 0;
    std::vector<FMemoryRegion> MemoryRegions;
};