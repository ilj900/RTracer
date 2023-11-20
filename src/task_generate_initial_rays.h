#pragma once

#include "executable_task.h"

class FGenerateInitialRays : public FExecutableTask
{
public:
    FGenerateInitialRays(int WidthIn, int HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
    ~FGenerateInitialRays();

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
    void Cleanup() override;
    VkSemaphore Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex) override;

    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkPipeline Pipeline = VK_NULL_HANDLE;

    const uint32_t GENERATE_RAYS_LAYOUT_INDEX = 0;
    const uint32_t CAMERA_RAYS_BUFFER = 0;
    const uint32_t CAMERA_POSITION_BUFFER = 1;
};