#pragma once

#include "executable_task.h"
#include "image.h"
#include "vk_pipeline.h"

class FVulkanContext;

class FPassthroughTask : public FExecutableTask
{
public:
    FPassthroughTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
    ~FPassthroughTask() override;

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
    void Cleanup() override;
    VkSemaphore Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex);

    VkSampler Sampler = VK_NULL_HANDLE;

    FGraphicsPipelineOptions GraphicsPipelineOptions;

    VkPipeline Pipeline = VK_NULL_HANDLE;
    VkRenderPass RenderPass = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> PassthroughFramebuffers;
};