#pragma once

#include "executable_task.h"

class FReset : public FExecutableTask
{
public:
	FReset(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice);
    ~FReset() override;

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
};