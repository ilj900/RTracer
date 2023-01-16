#pragma once

#include "image.h"
#include "vk_pipeline.h"

class FPassthroughTask
{
public:
    FPassthroughTask() = default;

    void Init();
    void UpdateDescriptorSet();
    void RecordCommands();
    void Cleanup();
    VkSemaphore Submit(VkQueue Queue, VkSemaphore WaitSemaphore, int IterationIndex);

    void RegisterInput(int Index, ImagePtr Image);
    void RegisterOutput(int Index, ImagePtr Image);
    ImagePtr GetInput(int Index);
    ImagePtr GetOutput(int Index);

    std::vector<ImagePtr> Inputs;
    std::vector<ImagePtr> Outputs;

    std::vector<VkSemaphore> SignalSemaphores;

    VkSampler Sampler;

    FGraphicsPipelineOptions GraphicsPipelineOptions;

    std::string Name = "Passthrough pipeline";

    VkPipeline Pipeline = VK_NULL_HANDLE;
    VkRenderPass RenderPass = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> PassthroughFramebuffers;
    std::vector<VkCommandBuffer> PassthroughCommandBuffers;

    /// Task set indices
    const uint32_t PASSTHROUGH_PER_FRAME_LAYOUT_INDEX = 0;

    /// Task descriptor indices
    const uint32_t PASSTHROUGH_TEXTURE_SAMPLER_LAYOUT_INDEX = 0;
};