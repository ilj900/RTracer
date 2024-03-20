#pragma once

#include "image.h"
#include "vk_pipeline.h"

class FVulkanContext;

class FExecutableTask
{
public:
    FExecutableTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
    virtual ~FExecutableTask();

    virtual void Init();
    virtual void UpdateDescriptorSets() = 0;
    virtual void RecordCommands() = 0;
    virtual void Cleanup() = 0;
    VkSemaphore Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkPipelineStageFlags PipelineStageFlags, VkFence WaitFence, VkFence SignalFence, int IterationIndex);

    void RegisterInput(int Index, ImagePtr Image);
    void RegisterOutput(int Index, ImagePtr Image);
    void UpdateDescriptorSet(uint32_t LayoutSetIndex, uint32_t LayoutIndex, int FrameIndex, const FBuffer& Buffer);
    void UpdateDescriptorSet(uint32_t LayoutSetIndex, uint32_t LayoutIndex, int FrameIndex, ImagePtr Image);
    void UpdateDescriptorSet(uint32_t LayoutSetIndex, uint32_t LayoutIndex, int FrameIndex, ImagePtr Image, VkSampler Sampler);
    ImagePtr GetInput(int Index);
    ImagePtr GetOutput(int Index);

    std::vector<ImagePtr> Inputs;
    std::vector<ImagePtr> Outputs;

    std::vector<VkCommandBuffer> CommandBuffers;

    std::vector<VkSemaphore> SignalSemaphores;

    std::string Name;

    uint32_t Width = 0;
    uint32_t Height = 0;

    FVulkanContext* Context = nullptr;
    int NumberOfSimultaneousSubmits;
    VkDevice LogicalDevice = VK_NULL_HANDLE;

protected:
    void CreateSyncObjects();
    void FreeSyncObjects();
};