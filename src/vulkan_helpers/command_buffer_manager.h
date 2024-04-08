#pragma once

#include "vulkan/vulkan.h"

#include <functional>
#include <string>
#include <unordered_map>

class FVulkanContext;

/**
 * A class that manages recording and submitting commands
 * Right now it uses one queue and one queue index
 */
class FCommandBufferManager
{
public:
    FCommandBufferManager() = default;
    ~FCommandBufferManager();

    VkCommandBuffer AllocateCommandBuffer(VkQueueFlagBits QueueType);
    void FreeCommandBuffer(VkCommandBuffer& CommandBuffer, VkQueueFlagBits QueueType);
    VkCommandBuffer BeginCommand(VkQueueFlagBits QueueType);
    VkCommandBuffer BeginSingleTimeCommand(VkQueueFlagBits QueueType, const std::string& CommandDescription = "DefaultCommandBufferDescription");
    void EndCommand(VkCommandBuffer &CommandBuffer);
    void SubmitCommandBuffer(VkCommandBuffer &CommandBuffer, VkQueueFlagBits QueueType);
    /**
     * Record command that later could be submited for execution more that one time
     * @param Lambda - [&, this](VkCommandBuffer CommandBuffer){}; type lambda that will be executed after the command buffer allocated, created, begun and before it ended
     * @return
     */
    VkCommandBuffer RecordCommand(const std::function<void(VkCommandBuffer&)> & Lambda, VkQueueFlagBits QueueType);
    /**
     * Run a single time command buffer
     * @param Lambda - [&, this](VkCommandBuffer CommandBuffer){}; type lambda that will be executed after the command buffer allocated, created, begun and before it ended
     */
    void RunSingletimeCommand(const std::function<void(VkCommandBuffer&)> & Lambda, VkQueueFlagBits QueueType, const std::string& CommandDescription = "DefaultCommandBufferDescription");

private:
    void CreateCommandPool(VkQueueFlagBits QueueType);

private:
    std::unordered_map<VkQueueFlagBits, VkCommandPool> QueueTypeToCommandPoolIndexMap;
};

FCommandBufferManager* GetCommandBufferManager();
void FreeCommandBufferManager();

#define COMMAND_BUFFER_MANAGER() GetCommandBufferManager()
#define FREE_COMMAND_BUFFER_MANAGER() FreeCommandBufferManager()