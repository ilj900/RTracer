#pragma once

#include "executable_task.h"
#include "vk_acceleration_structure.h"

class FSampleSpotLightTask : public FExecutableTask
{
public:
	FSampleSpotLightTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice);
    ~FSampleSpotLightTask() override;

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
	FSynchronizationPoint Submit(VkPipelineStageFlags& PipelineStageFlagsIn, FSynchronizationPoint SynchronizationPoint, uint32_t X, uint32_t Y) override;

    FBuffer SBTBuffer;

    VkStridedDeviceAddressRegionKHR RGenRegion{};
    VkStridedDeviceAddressRegionKHR RMissRegion{};
    VkStridedDeviceAddressRegionKHR RHitRegion{};
    VkStridedDeviceAddressRegionKHR RCallRegion{};
};
