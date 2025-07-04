#pragma once

#include "buffer.h"

#include <cassert>
#include <cstdint>

bool operator==(const FMemoryPtr& A, const FMemoryPtr& B)
{
    return (A.Offset == B.Offset);
}

size_t std::hash<FMemoryPtr>::operator()(FMemoryPtr const& MemoryPtr) const
{
    return std::hash<std::uint32_t>{}(MemoryPtr.Offset);
}

FMemoryPtr FBuffer::CheckAvailableMemory(VkDeviceSize Size)
{
    FMemoryPtr Result;

    for (auto& Entry : MemoryRegion.MemoryPtrs)
    {
        if (Entry.Size >= Size)
        {
            Result.Size = Size;
            Result.Offset = Entry.Offset;
            return Result;
        }
    }

    assert("Failed to find big enough memory chunk in buffer");

    return Result;
}

FMemoryPtr FBuffer::ReserveMemory(FMemoryPtr MemoryChunk)
{
    auto Entry = MemoryRegion.MemoryPtrs.find(MemoryChunk);
    if (Entry != MemoryRegion.MemoryPtrs.end())
    {
        FMemoryPtr NewValue;
        NewValue.Size = Entry->Size - MemoryChunk.Size;
        NewValue.Offset = Entry->Offset + MemoryChunk.Size;
        MemoryRegion.MemoryPtrs.erase(Entry);
        MemoryRegion.MemoryPtrs.insert(NewValue);
        return MemoryChunk;
    }

    assert("Failed to find reserve memory chunk in buffer");

    return FMemoryPtr{};
}