#pragma once

#include "executable_task.h"

class FResetActiveRayCountTask : public FExecutableTask
{
public:
	FResetActiveRayCountTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice);
    ~FResetActiveRayCountTask() override;

    void Init(FCompileDefinitions* CompileDefinitions = nullptr) override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
};
