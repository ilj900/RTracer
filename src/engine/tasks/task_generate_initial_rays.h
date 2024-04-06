#pragma once

#include "executable_task.h"

class FGenerateInitialRays : public FExecutableTask
{
public:
    FGenerateInitialRays(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
    ~FGenerateInitialRays() override;

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
};