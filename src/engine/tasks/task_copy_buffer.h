#pragma once

#include "executable_task.h"

class FCopyBufferTask : public FExecutableTask
{
public:
	FCopyBufferTask(const std::string& SrcBufferNameIn, const std::string& DstBufferNameIn, uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice);

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;

	std::string SrcBufferName;
	std::string DstBufferName;
};
