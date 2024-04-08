#pragma once

#include "executable_task.h"

class FClearTotalMaterialsCountTask : public FExecutableTask
{
public:
    FClearTotalMaterialsCountTask(uint32_t WidthIn, uint32_t HeightIn, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
    ~FClearTotalMaterialsCountTask() override;

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
};
