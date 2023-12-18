#pragma once

#include "buffer.h"
#include "image.h"

#include <memory>
#include <map>

class FVulkanContext;

class FResourceAllocator
{
public:
    FResourceAllocator(VkPhysicalDevice PhysicalDevice, VkDevice Device, FVulkanContext* Context);
    ~FResourceAllocator();

    FBuffer CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties, const std::string& DebugName = "");
    void DestroyBuffer(FBuffer& Buffer);
    FMemoryRegion AllocateMemory(VkDeviceSize Size, VkMemoryRequirements MemRequirements, VkMemoryPropertyFlags Properties, bool bDeviceAddressRequired = false);

    FBuffer RegisterBuffer(FBuffer Buffer, const std::string& Name);
    FBuffer GetBuffer(const std::string& Name);
    void UnregisterAndDestroyBuffer(const std::string& Name);

    FBuffer LoadDataToBuffer(FBuffer& Buffer, std::vector<VkDeviceSize> Sizes, std::vector<VkDeviceSize> Offsets, std::vector<void*> Datas);
    void LoadDataFromBuffer(FBuffer& Buffer, VkDeviceSize Size, VkDeviceSize Offset, void* Data);
    void LoadDataToStagingBuffer(std::vector<VkDeviceSize> Sizes, std::vector<void*> Datas);
    void LoadDataFromStagingBuffer(VkDeviceSize Size, void* Data, VkDeviceSize Offset);
    void CopyBuffer(FBuffer &SrcBuffer, FBuffer &DstBuffer, std::vector<VkDeviceSize> Sizes, std::vector<VkDeviceSize> SourceOffsets, std::vector<VkDeviceSize> DestinationOffsets);
    template <typename T>
    std::vector<T> DebugGetDataFromBuffer(FBuffer& SrcBuffer, int Size, int Offset)
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

    VkDevice Device = VK_NULL_HANDLE;
    FVulkanContext *Context = nullptr;

    VkPhysicalDeviceMemoryProperties MemProperties{};

private:
    std::map<std::string, FBuffer> Buffers;
};