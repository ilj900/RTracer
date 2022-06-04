#include "resource_allocation.h"
#include "buffer.h"
#include "context.h"

#include <stdexcept>

FResourceAllocator::FResourceAllocator(VkPhysicalDevice PhysicalDevice, VkDevice Device, FContext* Context)
    :Device(Device),
     Context(Context)
{
    vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &MemProperties);
}

FBuffer FResourceAllocator::CreateBufferWidthData(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties, void* Data)
{
    VkDeviceSize BufferSize = Size;

    /// Create staging buffer
    FBuffer StagingBuffer = CreateBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* StagingData;
    vkMapMemory(Device, StagingBuffer.Memory, 0, BufferSize, 0, &StagingData);
    memcpy(StagingData, Data, (std::size_t)BufferSize);
    vkUnmapMemory(Device, StagingBuffer.Memory);

    FBuffer Buffer = CreateBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | Usage, Properties);
    CopyBuffer(StagingBuffer, Buffer, BufferSize);
    DestroyBuffer(StagingBuffer);

    return Buffer;
}

FBuffer FResourceAllocator::CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties)
{
    FBuffer Buffer;

    Buffer.Size = Size;

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

void FResourceAllocator::CopyBuffer(FBuffer &SrcBuffer, FBuffer &DstBuffer, VkDeviceSize Size)
{
    Context->CommandBufferManager->RunSingletimeCommand([&, this](VkCommandBuffer CommandBuffer)
    {
        VkBufferCopy CopyRegion{};
        CopyRegion.size = Size;
        vkCmdCopyBuffer(CommandBuffer, SrcBuffer.Buffer, DstBuffer.Buffer, 1, &CopyRegion);
    });
}