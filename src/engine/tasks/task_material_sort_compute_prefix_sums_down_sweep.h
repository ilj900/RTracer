#pragma once

#include "executable_task.h"

class FComputePrefixSumsDownSweepTask : public FExecutableTask
{
public:
    FComputePrefixSumsDownSweepTask(uint32_t WidthIn, uint32_t HeightIn, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
};
