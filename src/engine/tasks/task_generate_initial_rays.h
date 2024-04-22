#pragma once

#include "executable_task.h"

class FGenerateInitialRays : public FExecutableTask
{
public:
    FGenerateInitialRays(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice);
    ~FGenerateInitialRays() override;

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
};