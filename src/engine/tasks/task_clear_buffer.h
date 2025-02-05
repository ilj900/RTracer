#pragma once

#include "executable_task.h"

class FClearBufferTask : public FExecutableTask
{
public:
	FClearBufferTask(const std::vector<std::string>& BufferNamesIn, uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice, uint32_t ClearValueIn = 0);
	FClearBufferTask(const std::string& BufferNameIn, uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice, uint32_t ClearValueIn = 0);

    void Init(FCompileDefinitions* CompileDefinitions = nullptr) override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;

	std::vector<std::string> BufferNames;
	uint32_t ClearValue;
};
