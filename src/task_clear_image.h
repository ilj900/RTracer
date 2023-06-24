#pragma once

#include "executable_task.h"

class FClearImageTask : public FExecutableTask
{
public:
    FClearImageTask(int WidthIn, int HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
    ~FClearImageTask();

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
    void Cleanup() override;
    VkSemaphore Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex) override;

    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkPipeline Pipeline = VK_NULL_HANDLE;

    /// Task descriptor indices
    const uint32_t CLEAR_IMAGE_LAYOUT_INDEX = 0;

    const uint32_t IMAGE_TO_CLEAR = 0;

};
