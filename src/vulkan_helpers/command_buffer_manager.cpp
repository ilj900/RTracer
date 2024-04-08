#include "command_buffer_manager.h"
#include "vk_debug.h"
#include "vk_context.h"

#include <stdexcept>

FCommandBufferManager* CommandBufferManager = nullptr;

FCommandBufferManager* GetCommandBufferManager()
{
    if (CommandBufferManager == nullptr)
    {
        CommandBufferManager = new FCommandBufferManager();
    }

    return CommandBufferManager;
}

void FreeCommandBufferManager()
{
    if (CommandBufferManager != nullptr);
    {
        delete CommandBufferManager;
        CommandBufferManager = nullptr;
    }
}

FCommandBufferManager::~FCommandBufferManager()
{
    for (auto Entry : QueueTypeToCommandPoolIndexMap)
    {
        vkDestroyCommandPool(VK_CONTEXT().LogicalDevice, Entry.second, nullptr);
    }
}

void FCommandBufferManager::CreateCommandPool(VkQueueFlagBits QueueType)
{
    VkCommandPoolCreateInfo PoolInfo{};
    PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    PoolInfo.queueFamilyIndex = VK_CONTEXT().GetQueueIndex(QueueType);
    PoolInfo.flags = 0;

    VkCommandPool CommandPool = VK_NULL_HANDLE;

    if (vkCreateCommandPool(VK_CONTEXT().LogicalDevice, &PoolInfo, nullptr, &CommandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool!");
    }

    QueueTypeToCommandPoolIndexMap[QueueType] = CommandPool;
}

VkCommandBuffer FCommandBufferManager::AllocateCommandBuffer(VkQueueFlagBits QueueType)
{
    if (QueueTypeToCommandPoolIndexMap.find(QueueType) == QueueTypeToCommandPoolIndexMap.end())
    {
        CreateCommandPool(QueueType);
    }

    VkCommandBuffer CommandBuffer;

    VkCommandBufferAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.commandPool = QueueTypeToCommandPoolIndexMap[QueueType];
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandBufferCount = 1u;

    if (vkAllocateCommandBuffers(VK_CONTEXT().LogicalDevice, &AllocInfo, &CommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffers!");
    }

    return CommandBuffer;
}

void FCommandBufferManager::FreeCommandBuffer(VkCommandBuffer& CommandBuffer, VkQueueFlagBits QueueType)
{
    vkFreeCommandBuffers(VK_CONTEXT().LogicalDevice, QueueTypeToCommandPoolIndexMap[QueueType], 1u, &CommandBuffer);
}

VkCommandBuffer FCommandBufferManager::BeginCommand(VkQueueFlagBits QueueType) {
    VkCommandBuffer CommandBuffer = AllocateCommandBuffer(QueueType);

    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = 0;

    vkBeginCommandBuffer(CommandBuffer, &BeginInfo);

    return CommandBuffer;
}

VkCommandBuffer FCommandBufferManager::BeginSingleTimeCommand(VkQueueFlagBits QueueType, const std::string& CommandDescription)
{
    VkCommandBuffer CommandBuffer = AllocateCommandBuffer(QueueType);
    V::SetName(VK_CONTEXT().LogicalDevice, CommandBuffer, CommandDescription);

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

void FCommandBufferManager::SubmitCommandBuffer(VkCommandBuffer &CommandBuffer, VkQueueFlagBits QueueType)
{
    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CommandBuffer;

    vkQueueSubmit(VK_CONTEXT().GetQueue(QueueType), 1, &SubmitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(VK_CONTEXT().GetQueue(QueueType));

    vkFreeCommandBuffers(VK_CONTEXT().LogicalDevice, QueueTypeToCommandPoolIndexMap[QueueType], 1, &CommandBuffer);
}

VkCommandBuffer FCommandBufferManager::RecordCommand(const std::function<void(VkCommandBuffer&)> & Lambda, VkQueueFlagBits QueueType)
{
    auto CommandBuffer = BeginCommand(QueueType);
    Lambda(CommandBuffer);
    vkEndCommandBuffer(CommandBuffer);
    return CommandBuffer;
}

void FCommandBufferManager::RunSingletimeCommand(const std::function<void(VkCommandBuffer&)> & Lambda, VkQueueFlagBits QueueType, const std::string& CommandDescription)
{
    auto CommandBuffer = BeginSingleTimeCommand(QueueType, CommandDescription);
    Lambda(CommandBuffer);
    EndCommand(CommandBuffer);
    SubmitCommandBuffer(CommandBuffer, QueueType);
}
