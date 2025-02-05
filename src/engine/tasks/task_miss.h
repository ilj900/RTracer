#pragma once

#include "executable_task.h"

class FMissTask : public FExecutableTask
{
public:
    FMissTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice);
    ~FMissTask() override;

    void Init(FCompileDefinitions* CompileDefinitions = nullptr) override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;

    VkSampler IBLImageSamplerLinear = VK_NULL_HANDLE;
	VkSampler IBLImageSamplerNearest = VK_NULL_HANDLE;
};
