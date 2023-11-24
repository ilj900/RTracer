#pragma once

#include "executable_task.h"
#include "vk_acceleration_structure.h"

class FRaytraceTask : public FExecutableTask
{
public:
    FRaytraceTask(int WidthIn, int HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
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

    VkSampler Sampler = VK_NULL_HANDLE;

    VkStridedDeviceAddressRegionKHR RGenRegion{};
    VkStridedDeviceAddressRegionKHR RMissRegion{};
    VkStridedDeviceAddressRegionKHR RHitRegion{};
    VkStridedDeviceAddressRegionKHR RCallRegion{};

    /// Task descriptor indices
    const uint32_t RAYTRACE_PER_FRAME_LAYOUT_INDEX = 0;

    const uint32_t TLAS_LAYOUT_INDEX = 0;
    const uint32_t RT_FINAL_IMAGE_INDEX = 1;
    const uint32_t RAYS_DATA_BUFFER = 2;
    const uint32_t RENDERABLE_BUFFER_INDEX = 3;
    const uint32_t MATERIAL_BUFFER_INDEX = 4;
    const uint32_t LIGHT_BUFFER_INDEX = 5;
    const uint32_t IBL_IMAGE_INDEX = 6;
};
