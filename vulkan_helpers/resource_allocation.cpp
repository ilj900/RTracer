#include "resource_allocation.h"
#include "buffer.h"
#include "vk_context.h"

#include <stdexcept>

FResourceAllocator::FResourceAllocator(VkPhysicalDevice PhysicalDevice, VkDevice Device, FVulkanContext* Context)
    :Device(Device),
     Context(Context)
{
    vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &MemProperties);

    /// Create staging buffer
    StagingBuffer = CreateBuffer(StagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    MeshBuffer = CreateBuffer(MeshBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

FResourceAllocator::~FResourceAllocator()
{
    DestroyBuffer(StagingBuffer);
    DestroyBuffer(MeshBuffer);
}

FBuffer FResourceAllocator::CreateBufferWidthData(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties, void* Data)
{
    VkDeviceSize BufferSize = Size;
    FBuffer Buffer = CreateBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | Usage, Properties);

    LoadDataToBuffer(Buffer, Size, 0, Data);

    return Buffer;
}

FMemoryRegion FResourceAllocator::PushDataToBuffer(FBuffer& Buffer, VkDeviceSize Size, void* Data)
{
    auto RemainingSize = Buffer.BufferSize - Buffer.CurrentOffset;
    if (Size <= RemainingSize)
    {
        FMemoryRegion MemoryRegion;
        MemoryRegion.Size = Size;
        MemoryRegion.Offset = Buffer.CurrentOffset;
        MemoryRegion.Buffer = &Buffer;
        LoadDataToBuffer(Buffer, Size, Buffer.CurrentOffset, Data);
        return MemoryRegion;
    }
    /// TODO: Try to compact the data in buffer
    assert(false && "Not enough space in Buffer");
    return FMemoryRegion();
}

FBuffer FResourceAllocator::LoadDataToBuffer(FBuffer& Buffer, VkDeviceSize Size, VkDeviceSize Offset, void* Data)
{
    auto SizeCopy = Size;

    for (int i = 0; Size > 0; ++i)
    {
        VkDeviceSize ChunkSize = (Size > StagingBufferSize) ? StagingBufferSize : Size;
        void *StagingData;
        vkMapMemory(Device, StagingBuffer.Memory, 0, ChunkSize, 0, &StagingData);
        memcpy(StagingData, ((char*)Data + (StagingBufferSize * i)), (std::size_t) ChunkSize);
        vkUnmapMemory(Device, StagingBuffer.Memory);

        CopyBuffer(StagingBuffer, Buffer, ChunkSize, 0, Offset + (StagingBufferSize * i));
        Size -= ChunkSize;
    }

    Buffer.CurrentOffset += SizeCopy;
    return Buffer;
}

void FResourceAllocator::LoadDataFromBuffer(FBuffer& Buffer, VkDeviceSize Size, VkDeviceSize Offset, void* Data)
{
    for (int i = 0; Size > 0; ++i)
    {
        VkDeviceSize ChunkSize = (Size > StagingBufferSize) ? StagingBufferSize : Size;

        CopyBuffer(Buffer, StagingBuffer, ChunkSize, Offset + (StagingBufferSize * i), 0);

        void *StagingData;
        vkMapMemory(Device, StagingBuffer.Memory, 0, ChunkSize, 0, &StagingData);
        memcpy(((char*)Data + (StagingBufferSize * i)), StagingData, (std::size_t) ChunkSize);
        vkUnmapMemory(Device, StagingBuffer.Memory);

        Size -= ChunkSize;
    }
}

FBuffer FResourceAllocator::CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties)
{
    FBuffer Buffer;

    Buffer.BufferSize = Size;

    /// Create buffer
    VkBufferCreateInfo BufferInfo{};
    BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferInfo.size = Size;
    BufferInfo.usage = Usage;
    BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(Device, &BufferInfo, nullptr, &Buffer.Buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create buffer!");
    }

    /// Allocate Device Memory
    VkMemoryRequirements MemRequirements;
    vkGetBufferMemoryRequirements(Device, Buffer.Buffer, &MemRequirements);

    VkMemoryAllocateInfo  AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    AllocInfo.allocationSize = MemRequirements.size;
    AllocInfo.memoryTypeIndex = FindMemoryType(MemRequirements.memoryTypeBits, Properties);

    if (vkAllocateMemory(Device, &AllocInfo, nullptr, &Buffer.Memory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate buffer memory!");
    }

    /// Bind Buffer Memory
    vkBindBufferMemory(Device, Buffer.Buffer, Buffer.Memory, 0);

    return Buffer;
}

void FResourceAllocator::DestroyBuffer(FBuffer& Buffer)
{
    vkDestroyBuffer(Device, Buffer.Buffer, nullptr);
    vkFreeMemory(Device, Buffer.Memory, nullptr);
    Buffer.Buffer = VK_NULL_HANDLE;
    Buffer.Memory = VK_NULL_HANDLE;
}

uint32_t FResourceAllocator::FindMemoryType(uint32_t TypeFilter, VkMemoryPropertyFlags Properties)
{
    for (uint32_t i = 0; i < MemProperties.memoryTypeCount; ++i)
    {
        if (TypeFilter & (1 << i) && (MemProperties.memoryTypes[i].propertyFlags & Properties) == Properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

void FResourceAllocator::CopyBuffer(FBuffer &SrcBuffer, FBuffer &DstBuffer, VkDeviceSize Size, VkDeviceSize SourceOffset, VkDeviceSize DestinationOffset)
{
    Context->CommandBufferManager->RunSingletimeCommand([&, this](VkCommandBuffer CommandBuffer)
    {
        VkBufferCopy CopyRegion{};
        CopyRegion.size = Size;
        CopyRegion.srcOffset = SourceOffset;
        CopyRegion.dstOffset = DestinationOffset;
        vkCmdCopyBuffer(CommandBuffer, SrcBuffer.Buffer, DstBuffer.Buffer, 1, &CopyRegion);
    });
}