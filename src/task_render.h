#pragma once

#include "image.h"
#include "vk_pipeline.h"

class FVulkanContext;

class FRenderTask
{
public:
    FRenderTask(FVulkanContext* Context, int NumberOfFrames, VkDevice LogicalDevice);

    void Init();
    void UpdateDescriptorSets();
    void RecordCommands();
    void Cleanup();
    VkSemaphore Submit(VkQueue Queue, VkSemaphore WaitSemaphore, int IterationIndex);

    void RegisterInput(int Index, ImagePtr Image);
    void RegisterOutput(int Index, ImagePtr Image);
    ImagePtr GetInput(int Index);
    ImagePtr GetOutput(int Index);

    std::vector<ImagePtr> Inputs;
    std::vector<ImagePtr> Outputs;

    VkSampler Sampler;

    ImagePtr DepthImage;

    std::vector<VkFramebuffer> RenderFramebuffers;

    FGraphicsPipelineOptions GraphicsPipelineOptions;

    std::vector<VkCommandBuffer> GraphicsCommandBuffers;

    std::vector<VkSemaphore> SignalSemaphores;

    std::string Name = "Render pipeline";

    VkPipeline Pipeline = VK_NULL_HANDLE;
    VkRenderPass RenderPass = VK_NULL_HANDLE;

    FVulkanContext* Context = nullptr;
    int FramesCount = 2;
    VkDevice LogicalDevice = VK_NULL_HANDLE;

    /// Task set indices
    const uint32_t RENDER_PER_FRAME_LAYOUT_INDEX = 0;
    const uint32_t RENDER_PER_RENDERABLE_LAYOUT_INDEX = 1;

    /// Task descriptor indices
    const uint32_t TEXTURE_SAMPLER_LAYOUT_INDEX = 0;
    const uint32_t CAMERA_LAYOUT_INDEX = 1;
    const uint32_t TRANSFORM_LAYOUT_INDEX = 0;
    const uint32_t RENDERABLE_LAYOUT_INDEX = 1;

};