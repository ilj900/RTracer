#pragma once

#include "buffer.h"
#include "image.h"

#include <memory>
#include <map>

class FVulkanContext;

class FResourceAllocator
{
public:
    FResourceAllocator();
    ~FResourceAllocator();

    FBuffer CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties, const std::string& DebugName = "");
    void DestroyBuffer(FBuffer& Buffer);
    FMemoryRegion AllocateMemory(VkDeviceSize Size, VkMemoryRequirements MemRequirements, VkMemoryPropertyFlags Properties, bool bDeviceAddressRequired = false);

    FBuffer RegisterBuffer(FBuffer Buffer, const std::string& Name);
    FBuffer GetBuffer(const std::string& Name);
	bool BufferExists(const std::string& Name);
    void UnregisterAndDestroyBuffer(const std::string& Name);

    FBuffer LoadDataToBuffer(FBuffer& Buffer, std::vector<VkDeviceSize> SizesIn, std::vector<VkDeviceSize> OffsetsIn, std::vector<void*> DatasIn);
    void LoadDataFromBuffer(FBuffer& Buffer, VkDeviceSize Size, VkDeviceSize Offset, void* Data);
    void LoadDataToStagingBuffer(std::vector<VkDeviceSize> Sizes, std::vector<void*> Datas);
    void LoadDataFromStagingBuffer(VkDeviceSize Size, void* Data, VkDeviceSize Offset);
    void CopyBuffer(const FBuffer &SrcBuffer, FBuffer &DstBuffer, std::vector<VkDeviceSize> Sizes, std::vector<VkDeviceSize> SourceOffsets, std::vector<VkDeviceSize> DestinationOffsets);

    template <typename T>
    std::vector<T> DebugGetDataFromBuffer(const std::string& Name)
    {
        FBuffer Buffer = GetBuffer(Name);
        return DebugGetDataFromBuffer<T>(Buffer, Buffer.BufferSize, 0);
    }

    template <typename T>
    std::vector<T> DebugGetDataFromBuffer(const FBuffer& SrcBuffer, int Size, int Offset)
    {
        std::vector<T> Result;
        Result.resize(Size / sizeof(T));

        CopyBuffer(SrcBuffer, StagingBuffer, {(VkDeviceSize)Size}, {(VkDeviceSize)Offset}, {0});
        LoadDataFromStagingBuffer(Size, Result.data(), 0);

        return Result;
    };

    void* Map(FBuffer& Buffer);
    void Unmap(FBuffer& Buffer);

    FMemoryRegion LoadDataToImage(FImage& Image, VkDeviceSize Size, void* Data);
    void CopyBufferToImage(FBuffer& SrcBuffer, FImage& DstImage);
    void CopyImageToBuffer(const FImage& SrcImage, FBuffer& DstBuffer);
    void GetImageData(FImage& SrcImage, void* Data);

    uint32_t FindMemoryType(uint32_t TypeFilter, VkMemoryPropertyFlags Properties);

    VkDeviceSize StagingBufferSize = uint64_t(256) * 1024 * 1024;
    FBuffer StagingBuffer;

    VkPhysicalDeviceMemoryProperties MemProperties{};

private:
    std::map<std::string, FBuffer> Buffers;
};

FResourceAllocator* GetResourceAllocator();
void FreeResourceAllocator();

#define RESOURCE_ALLOCATOR() GetResourceAllocator()
#define FREE_RESOURCE_ALLOCATOR() FreeResourceAllocator()