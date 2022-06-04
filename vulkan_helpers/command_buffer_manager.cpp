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
    VkCommandPoolCreateInfo PoolInfo{};
    PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    PoolInfo.queueFamilyIndex = QueueIndex;
    PoolInfo.flags = 0;

    if (vkCreateCommandPool(Device, &PoolInfo, nullptr, &CommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool!");
    }
}

VkCommandBuffer FCommandBufferManager::AllocateCommandBuffer()
{
    VkCommandBuffer CommandBuffer;

    VkCommandBufferAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.commandPool = CommandPool;
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandBufferCount = 1u;

    if (vkAllocateCommandBuffers(Device, &AllocInfo, &CommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffers!");
    }

    return CommandBuffer;
}

void FCommandBufferManager::FreeCommandBuffer(VkCommandBuffer& CommandBuffer)
{
    vkFreeCommandBuffers(Device, CommandPool, 1u, &CommandBuffer);
}

VkCommandBuffer FCommandBufferManager::BeginCommand() {
    VkCommandBuffer CommandBuffer = AllocateCommandBuffer();

    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = 0;

    vkBeginCommandBuffer(CommandBuffer, &BeginInfo);

    return CommandBuffer;
}

VkCommandBuffer FCommandBufferManager::BeginSingleTimeCommand()
{
    VkCommandBuffer CommandBuffer = AllocateCommandBuffer();

    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(CommandBuffer, &BeginInfo);

    return CommandBuffer;
}

void FCommandBufferManager::EndCommand(VkCommandBuffer &CommandBuffer)
{
    vkEndCommandBuffer(CommandBuffer);
    }

void FCommandBufferManager::SubmitCommandBuffer(VkCommandBuffer &CommandBuffer)
{
    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CommandBuffer;

    vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(Queue);

    vkFreeCommandBuffers(Device, CommandPool, 1, &CommandBuffer);
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
