#pragma once

#include "vulkan/vulkan.h"

#include <functional>
#include <string>

class FVulkanContext;

/**
 * A class that manages recording and submitting commands
 * Right now it uses one queue and one queue index
 */
class FCommandBufferManager
{
public:
    FCommandBufferManager(VkDevice Device, VkQueue Queue, uint32_t QueueIndex);

    VkCommandBuffer AllocateCommandBuffer();
    void FreeCommandBuffer(VkCommandBuffer& CommandBuffer);
    VkCommandBuffer BeginCommand();
    VkCommandBuffer BeginSingleTimeCommand(const std::string CommandDescription = "DefaultCommandBufferDescription");
    void EndCommand(VkCommandBuffer &CommandBuffer);
    void SubmitCommandBuffer(VkCommandBuffer &CommandBuffer);
    /**
     * Record command that later could be submited for execution more that one time
     * @param Lambda - [&, this](VkCommandBuffer CommandBuffer){}; type lambda that will be executed after the command buffer allocated, created, begun and before it ended
     * @return
     */
    VkCommandBuffer RecordCommand(const std::function<void(VkCommandBuffer&)> & Lambda);
    /**
     * Run a single time command buffer
     * @param Lambda - [&, this](VkCommandBuffer CommandBuffer){}; type lambda that will be executed after the command buffer allocated, created, begun and before it ended
     */
    void RunSingletimeCommand(const std::function<void(VkCommandBuffer&)> & Lambda, const std::string CommandDescription = "DefaultCommandBufferDescription");

    ~FCommandBufferManager();
private:
    void CreateCommandPool();

private:
    VkDevice Device = VK_NULL_HANDLE;
    FVulkanContext *Context = nullptr;
    VkQueue Queue;
    uint32_t QueueIndex;

    VkCommandPool CommandPool = VK_NULL_HANDLE;
};