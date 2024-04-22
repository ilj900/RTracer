#pragma once

#include "executable_task.h"
#include "vk_acceleration_structure.h"

class FRaytraceTask : public FExecutableTask
{
public:
    FRaytraceTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice);
    ~FRaytraceTask() override;

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;

    FBuffer SBTBuffer;

    VkStridedDeviceAddressRegionKHR RGenRegion{};
    VkStridedDeviceAddressRegionKHR RMissRegion{};
    VkStridedDeviceAddressRegionKHR RHitRegion{};
    VkStridedDeviceAddressRegionKHR RCallRegion{};
};
