#pragma once

#include "executable_task.h"

class FAccumulateTask : public FExecutableTask
{
public:
    FAccumulateTask(int WidthIn, int HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
    ~FAccumulateTask();

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
    void Cleanup() override;
    VkSemaphore Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex) override;

    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkPipeline Pipeline = VK_NULL_HANDLE;

    /// Task descriptor indices
    const uint32_t ACCUMULATE_PER_FRAME_LAYOUT_INDEX = 0;

    const uint32_t INCOMING_IMAGE_TO_SAMPLE = 0;
    const uint32_t ACCUMULATE_IMAGE_INDEX = 1;
    const uint32_t ESTIMATED_IMAGE_INDEX = 2;

};
