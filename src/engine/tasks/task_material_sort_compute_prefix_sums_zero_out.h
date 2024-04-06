#pragma once

#include "executable_task.h"

class FComputePrefixSumsZeroOutTask : public FExecutableTask
{
public:
    FComputePrefixSumsZeroOutTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
    ~FComputePrefixSumsZeroOutTask();

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
    void Cleanup() override;

    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkPipeline Pipeline = VK_NULL_HANDLE;
};
