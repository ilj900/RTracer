#pragma once

#include "executable_task.h"
#include "vk_acceleration_structure.h"

class FMasterShader : public FExecutableTask
{
public:
	FMasterShader(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice);
    ~FMasterShader() override;

    void Init(FCompileDefinitions* CompileDefinitions = nullptr) override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;

	VkSampler MaterialTextureSampler = VK_NULL_HANDLE;
	std::vector<VkPipeline> MaterialPipelines;
	VkSampler IBLSampler = VK_NULL_HANDLE;

    std::vector<FBuffer> SBTBuffers;

    std::vector<VkStridedDeviceAddressRegionKHR> RGenRegions;
    VkStridedDeviceAddressRegionKHR RMissRegion{};
    VkStridedDeviceAddressRegionKHR RHitRegion{};
    VkStridedDeviceAddressRegionKHR RCallRegion{};
};
