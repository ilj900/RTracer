#pragma once

#include "image.h"
#include "vk_pipeline.h"

enum DirtyType {UNINITIALIZED = 1u,
				OUTDATED_DESCRIPTOR_SET = 1u << 1,
				OUTDATED_COMMAND_BUFFER = 1u << 2};

class FVulkanContext;

class FExecutableTask
{
public:
    FExecutableTask(uint32_t WidthIn, uint32_t HeightIn, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
    virtual ~FExecutableTask();

    virtual void Init() = 0;
    virtual void UpdateDescriptorSets() = 0;
    virtual void RecordCommands() = 0;
	virtual void Reload();
    virtual VkSemaphore Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkPipelineStageFlags PipelineStageFlagsIn, VkFence WaitFence, VkFence SignalFence, uint32_t IterationIndex);

    void RegisterInput(int Index, ImagePtr Image);
    void RegisterOutput(int Index, ImagePtr Image);
    void UpdateDescriptorSet(uint32_t LayoutSetIndex, uint32_t LayoutIndex, int FrameIndex, const FBuffer& Buffer);
    void UpdateDescriptorSet(uint32_t LayoutSetIndex, uint32_t LayoutIndex, int FrameIndex, ImagePtr Image);
    void UpdateDescriptorSet(uint32_t LayoutSetIndex, uint32_t LayoutIndex, int FrameIndex, ImagePtr Image, VkSampler Sampler);
	void SetDirty(uint32_t Flags);
    VkPipelineStageFlags GetPipelineStageFlags();
    ImagePtr GetInput(int Index);
    ImagePtr GetOutput(int Index);

    std::vector<ImagePtr> Inputs;
    std::vector<ImagePtr> Outputs;

    std::vector<VkCommandBuffer> CommandBuffers;

    std::vector<VkSemaphore> SignalSemaphores;

    std::string Name;

    uint32_t Width = 0;
    uint32_t Height = 0;

    int NumberOfSimultaneousSubmits;
    VkDevice LogicalDevice = VK_NULL_HANDLE;
    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkPipeline Pipeline = VK_NULL_HANDLE;
    VkPipelineStageFlags PipelineStageFlags;
    VkQueueFlagBits QueueFlagsBits;
	uint32_t DitryFlags = true;

protected:
    void CreateSyncObjects();
    void FreeSyncObjects();
};