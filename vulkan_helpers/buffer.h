#pragma once

#include "vulkan/vulkan.h"

#include <unordered_set>
#include <memory>

struct FBuffer;

struct FMemoryPtr
{
    VkDeviceSize Offset = 0;
    VkDeviceSize Size = 0;
};

bool operator==(const FMemoryPtr& A, const FMemoryPtr& B);

template<>
struct std::hash<FMemoryPtr>
{
    size_t operator()(FMemoryPtr const& MemoryPtr) const;
};

struct FMemoryRegion
{
    VkDeviceMemory Memory = VK_NULL_HANDLE;
    std::unordered_set<FMemoryPtr> MemoryPtrs;
};

struct FBuffer
{
    VkBuffer Buffer = VK_NULL_HANDLE;
    VkDeviceSize BufferSize = 0;
    VkDeviceSize CurrentOffset = 0;
    FMemoryRegion MemoryRegion;

    FMemoryPtr CheckAvailableMemory(VkDeviceSize Size);
    FMemoryPtr ReserveMemory(FMemoryPtr MemoryChunk);
};
