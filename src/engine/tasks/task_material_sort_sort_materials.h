#pragma once

#include "executable_task.h"

class FSortMaterialsTask : public FExecutableTask
{
public:
    FSortMaterialsTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
    ~FSortMaterialsTask() override;

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
};
