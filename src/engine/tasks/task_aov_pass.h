#pragma once

#include "executable_task.h"

class FAOVPassTask : public FExecutableTask
{
public:
	FAOVPassTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice);
    ~FAOVPassTask() override;

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
};
