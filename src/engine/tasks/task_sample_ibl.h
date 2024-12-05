#pragma once

#include "executable_task.h"
#include "vk_acceleration_structure.h"

class FSampleIBLTask : public FExecutableTask
{
public:
	FSampleIBLTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice);
    ~FSampleIBLTask() override;

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;

	VkSampler Sampler = VK_NULL_HANDLE;

    FBuffer SBTBuffer;

    VkStridedDeviceAddressRegionKHR RGenRegion{};
    VkStridedDeviceAddressRegionKHR RMissRegion{};
    VkStridedDeviceAddressRegionKHR RHitRegion{};
    VkStridedDeviceAddressRegionKHR RCallRegion{};
};
