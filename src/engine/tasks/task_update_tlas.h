#pragma once

#include "executable_task.h"

class FUpdateTLASTask : public FExecutableTask
{
public:
	FUpdateTLASTask(uint32_t WidthIn, uint32_t HeightIn, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);

    void Init() override;
	void UpdateDescriptorSets() override;
    void RecordCommands() override;
};
