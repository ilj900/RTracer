#pragma once

#include "image.h"
#include "vk_pipeline.h"
#include "executable_task.h"

class FVulkanContext;

class FRenderTask : public FExecutableTask
{
public:
    FRenderTask(FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
    ~FRenderTask() override;

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
    void Cleanup() override;
    VkSemaphore Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex) override;

    VkSampler Sampler = VK_NULL_HANDLE;

    ImagePtr DepthImage;

    std::vector<VkFramebuffer> RenderFramebuffers;

    FGraphicsPipelineOptions GraphicsPipelineOptions;

    VkPipeline Pipeline = VK_NULL_HANDLE;
    VkRenderPass RenderPass = VK_NULL_HANDLE;

    /// Task set indices
    const uint32_t RENDER_PER_FRAME_LAYOUT_INDEX = 0;
    const uint32_t RENDER_PER_RENDERABLE_LAYOUT_INDEX = 1;

    /// Task descriptor indices
    const uint32_t TEXTURE_SAMPLER_LAYOUT_INDEX = 0;
    const uint32_t CAMERA_LAYOUT_INDEX = 1;
    const uint32_t TRANSFORM_LAYOUT_INDEX = 0;
    const uint32_t RENDERABLE_LAYOUT_INDEX = 1;
};