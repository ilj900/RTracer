#pragma once

#include "executable_task.h"
#include "image.h"
#include "vk_pipeline.h"

class FVulkanContext;

class FPassthroughTask : public FExecutableTask
{
public:
    FPassthroughTask(uint32_t WidthIn, uint32_t HeightIn, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
    ~FPassthroughTask() override;

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;

    VkSampler Sampler = VK_NULL_HANDLE;

    FGraphicsPipelineOptions GraphicsPipelineOptions;

    VkRenderPass RenderPass = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> PassthroughFramebuffers;
};