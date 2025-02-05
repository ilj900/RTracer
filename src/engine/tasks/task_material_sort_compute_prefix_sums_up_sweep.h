#pragma once

#include "executable_task.h"

class FComputePrefixSumsUpSweepTask : public FExecutableTask
{
public:
    FComputePrefixSumsUpSweepTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice);

    void Init(FCompileDefinitions* CompileDefinitions = nullptr) override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
};
