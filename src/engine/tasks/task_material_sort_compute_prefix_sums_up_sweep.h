#pragma once

#include "executable_task.h"

class FComputePrefixSumsUpSweepTask : public FExecutableTask
{
public:
    FComputePrefixSumsUpSweepTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
};
