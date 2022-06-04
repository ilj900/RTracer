#pragma once

#include "vulkan/vulkan.h"

#include <functional>

class FContext;

class FCommandBufferManager
{
public:
    FCommandBufferManager(VkDevice Device, FContext* Context, VkQueue Queue, uint32_t QueueIndex);

    VkCommandBuffer AllocateCommandBuffer();
    void FreeCommandBuffer(VkCommandBuffer& CommandBuffer);
    VkCommandBuffer BeginCommand();
    VkCommandBuffer BeginSingleTimeCommand();
    void EndCommand(VkCommandBuffer &CommandBuffer);
    void SubmitCommandBuffer(VkCommandBuffer &CommandBuffer);
    VkCommandBuffer RecordCommand(const std::function<void(VkCommandBuffer&)> & Lambda);
    void RunSingletimeCommand(const std::function<void(VkCommandBuffer&)> & Lambda);

    ~FCommandBufferManager();
private:
    void CreateCommandPool();

private:
    VkDevice Device = VK_NULL_HANDLE;
    FContext *Context = nullptr;
    VkQueue Queue;
    uint32_t QueueIndex;

    VkCommandPool CommandPool = VK_NULL_HANDLE;
};