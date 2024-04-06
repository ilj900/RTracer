#pragma once

#include "executable_task.h"

class FComputePrefixSumsDownSweepTask : public FExecutableTask
{
public:
    FComputePrefixSumsDownSweepTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
    ~FComputePrefixSumsDownSweepTask();

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
    void Cleanup() override;

    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkPipeline Pipeline = VK_NULL_HANDLE;
};
