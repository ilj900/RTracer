#include "resource_allocation.h"

#include <stdexcept>

FResourceAllocator::FResourceAllocator(VkPhysicalDevice PhysicalDevice, VkDevice Device)
    :Device(Device)
{
    vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &MemProperties);
}

FBuffer FResourceAllocator::CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties)
{
    FBuffer Buffer;
    VkBufferCreateInfo BufferInfo{};
    BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferInfo.size = Size;
    BufferInfo.usage = Usage;
    BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(Device, &BufferInfo, nullptr, &Buffer.Buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create buffer!");
    }

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