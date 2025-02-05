#pragma once

#include "executable_task.h"

class FComputePrefixSumsDownSweepTask : public FExecutableTask
{
public:
    FComputePrefixSumsDownSweepTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice);

    void Init(FCompileDefinitions* CompileDefinitions = nullptr) override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
};
