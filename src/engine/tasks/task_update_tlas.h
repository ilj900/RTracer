#pragma once

#include "executable_task.h"

class FUpdateTLASTask : public FExecutableTask
{
public:
	FUpdateTLASTask(uint32_t WidthIn, uint32_t HeightIn, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
	~FUpdateTLASTask() override;

    void Init() override;
	void UpdateDescriptorSets() override;
    void RecordCommands() override;

	FBuffer ScratchBuffer;
};
