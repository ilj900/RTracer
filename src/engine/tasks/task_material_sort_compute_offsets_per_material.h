#pragma once

#include "executable_task.h"

class FComputeOffsetsPerMaterialTask : public FExecutableTask
{
public:
    FComputeOffsetsPerMaterialTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice);
    ~FComputeOffsetsPerMaterialTask() override;

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
};
