#pragma once

#include "executable_task.h"

class FMissTask : public FExecutableTask
{
public:
    FMissTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
    ~FMissTask() override;

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;

    VkSampler IBLImageSampler = VK_NULL_HANDLE;
};
