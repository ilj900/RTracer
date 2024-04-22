#pragma once

#include "executable_task.h"

class FClearMaterialsCountPerChunkTask : public FExecutableTask
{
public:
    FClearMaterialsCountPerChunkTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice);
    ~FClearMaterialsCountPerChunkTask() override;

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
};
