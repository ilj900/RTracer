#pragma once

#include "executable_task.h"

class FComputeOffsetsPerMaterialTask : public FExecutableTask
{
public:
    FComputeOffsetsPerMaterialTask(uint32_t WidthIn, uint32_t HeightIn, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
    ~FComputeOffsetsPerMaterialTask() override;

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
};
