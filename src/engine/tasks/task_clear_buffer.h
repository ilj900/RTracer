#pragma once

#include "executable_task.h"

class FClearBufferTask : public FExecutableTask
{
public:
	FClearBufferTask(const std::string& BufferNameIn, uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice);

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;

	std::string BufferName;
};
