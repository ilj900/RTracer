#pragma once

#include "executable_task.h"
#include "vk_acceleration_structure.h"

class FRaytraceTask : public FExecutableTask
{
public:
    FRaytraceTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
    ~FRaytraceTask();

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
    void Cleanup() override;
    VkSemaphore Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex) override;

    std::vector<FAccelerationStructure> BLASVector;
    FAccelerationStructure TLAS;
    FBuffer SBTBuffer;

    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkPipeline Pipeline = VK_NULL_HANDLE;

    VkStridedDeviceAddressRegionKHR RGenRegion{};
    VkStridedDeviceAddressRegionKHR RMissRegion{};
    VkStridedDeviceAddressRegionKHR RHitRegion{};
    VkStridedDeviceAddressRegionKHR RCallRegion{};
};
