#pragma once

#include "executable_task.h"
#include "image.h"
#include "vk_pipeline.h"

class FVulkanContext;

class FPassthroughTask : public FExecutableTask
{
public:
    FPassthroughTask(FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
    void Cleanup() override;
    VkSemaphore Submit(VkQueue Queue, VkSemaphore WaitSemaphore, int IterationIndex) override;

    VkSampler Sampler;

    FGraphicsPipelineOptions GraphicsPipelineOptions;

    VkPipeline Pipeline = VK_NULL_HANDLE;
    VkRenderPass RenderPass = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> PassthroughFramebuffers;

    /// Task set indices
    const uint32_t PASSTHROUGH_PER_FRAME_LAYOUT_INDEX = 0;

    /// Task descriptor indices
    const uint32_t PASSTHROUGH_TEXTURE_SAMPLER_LAYOUT_INDEX = 0;
};