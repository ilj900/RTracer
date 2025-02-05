#pragma once

#include "executable_task.h"

class FComputeShadingDataTask : public FExecutableTask
{
public:
	FComputeShadingDataTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice);
    ~FComputeShadingDataTask() override;

    void Init(FCompileDefinitions* CompileDefinitions = nullptr) override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
};
