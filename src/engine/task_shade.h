#pragma once

#include "executable_task.h"

class FShadeTask : public FExecutableTask
{
public:
    FShadeTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
    ~FShadeTask();

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
    void Cleanup() override;

    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;

    VkSampler MaterialTextureSampler = VK_NULL_HANDLE;

    std::unordered_map<uint32_t , VkPipeline> MaterialIndexToPipelineMap;
};
