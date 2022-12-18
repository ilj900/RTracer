#pragma once

#include "vulkan/vulkan.h"

#include <vector>

struct FBuffer;

struct FMemoryPtr
{
    VkDeviceSize Offset = 0;
    VkDeviceSize Size = 0;
};

struct FMemoryRegion
{
    VkDeviceMemory Memory = VK_NULL_HANDLE;
    std::vector<FMemoryPtr> MemoryPtrs;
};

struct FBuffer
{
    VkBuffer Buffer = VK_NULL_HANDLE;
    VkDeviceSize BufferSize = 0;
    VkDeviceSize CurrentOffset = 0;
    FMemoryRegion MemoryRegion;
};