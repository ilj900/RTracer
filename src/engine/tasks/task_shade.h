#pragma once

#include "executable_task.h"

class FShadeTask : public FExecutableTask
{
public:
    FShadeTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice);
    ~FShadeTask() override;

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;

    VkSampler MaterialTextureSampler = VK_NULL_HANDLE;
    std::unordered_map<uint32_t , VkPipeline> MaterialIndexToPipelineMap;
};
