#include "command_buffer_manager.h"

#include <stdexcept>

FCommandBufferManager::FCommandBufferManager(VkDevice Device, FContext* Context, VkQueue Queue, uint32_t QueueIndex) :
    Device(Device), Context(Context), Queue(Queue), QueueIndex(QueueIndex)
{
    CreateCommandPool();
}

FCommandBufferManager::~FCommandBufferManager()
{
    vkDestroyCommandPool(Device, CommandPool, nullptr);
}

void FCommandBufferManager::CreateCommandPool()
{
    CommandPool = V::CreateCommandPool(Device, QueueIndex);
}

VkCommandBuffer FCommandBufferManager::AllocateCommandBuffer()
{
    return V::AllocateCommandBuffer(Device, CommandPool);
}

void FCommandBufferManager::FreeCommandBuffer(VkCommandBuffer& CommandBuffer)
{
    vkFreeCommandBuffers(Device, CommandPool, 1u, &CommandBuffer);
}

VkCommandBuffer FCommandBufferManager::BeginCommand()
{
    return V::BeginWithAllocation(Device, CommandPool);
}

VkCommandBuffer FCommandBufferManager::BeginSingleTimeCommand()
{
    return V::BeginSingleTimeCommand(Device, CommandPool);
}

void FCommandBufferManager::EndCommand(VkCommandBuffer &CommandBuffer)
{
    vkEndCommandBuffer(CommandBuffer);
}

void FCommandBufferManager::SubmitCommandBuffer(VkCommandBuffer &CommandBuffer)
{
    V::SubmitCommandBuffer(Device, CommandPool, Queue, CommandBuffer);
}

VkCommandBuffer FCommandBufferManager::RecordCommand(const std::function<void(VkCommandBuffer&)> & Lambda)
{
    auto CommandBuffer = BeginCommand();
    Lambda(CommandBuffer);
    vkEndCommandBuffer(CommandBuffer);
    return CommandBuffer;
}

void FCommandBufferManager::RunSingletimeCommand(const std::function<void(VkCommandBuffer&)> & Lambda)
{
    auto CommandBuffer = BeginSingleTimeCommand();
    Lambda(CommandBuffer);
    EndCommand(CommandBuffer);
    SubmitCommandBuffer(CommandBuffer);
}
