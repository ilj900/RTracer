#pragma once

#include "executable_task.h"

class FClearBufferTask : public FExecutableTask
{
public:
	FClearBufferTask(const std::string& BufferNameIn, VkDevice LogicalDevice);

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;

	std::string BufferName;
};
