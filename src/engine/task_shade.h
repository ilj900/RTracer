#pragma once

#include "executable_task.h"

class FShadeTask : public FExecutableTask
{
public:
    FShadeTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
    ~FShadeTask();

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
    void Cleanup() override;
    VkSemaphore Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex);

    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkPipeline Pipeline = VK_NULL_HANDLE;

    VkSampler IBLImageSampler = VK_NULL_HANDLE;
    VkSampler MaterialTextureSampler = VK_NULL_HANDLE;
};
