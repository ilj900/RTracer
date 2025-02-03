#pragma once

#include "image.h"
#include "vk_pipeline.h"
#include "vk_utils.h"
#include "named_resources.h"

enum DirtyType {UNINITIALIZED 			= 1u,
				OUTDATED_DESCRIPTOR_SET = 1u << 1,
				OUTDATED_COMMAND_BUFFER = 1u << 2};

class FVulkanContext;

enum TaskState {UNDEFINED 	= 1u,
				SUBMITTED 	= 1u << 1,
				QUERIED 	= 1u << 2};

class FGPUTimer
{
public:
	FGPUTimer(VkCommandBuffer CommandBufferIn, VkPipelineStageFlagBits PipelineStageFlagBitsStartIn, VkPipelineStageFlagBits PipelineStageFlagBitsEndIn, VkQueryPool QueryPoolIn, uint32_t QueryIn);
	~FGPUTimer();

private:
	VkCommandBuffer CommandBuffer;
	VkQueryPool QueryPool;
	uint32_t Query;
	VkPipelineStageFlagBits PipelineStageFlagBitsStart;
	VkPipelineStageFlagBits PipelineStageFlagBitsEnd;
};

#define GPU_TIMER() FGPUTimer GPUTimer(CommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, QueryPool, i)

class FExecutableTask
{
public:
	/// Creates a task that can be submitted to a queue for an execution
	/// SubmitXIn parameter describes how many times a task might be submitted during one render() call (for multiple bounces)
	/// SubmitYIn parameter describes how many frames will be submitted in parallel
    FExecutableTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice);
    virtual ~FExecutableTask();

    virtual void Init() = 0;
    virtual void UpdateDescriptorSets() = 0;
    virtual void RecordCommands() = 0;
	virtual void Reload();
	/// SynchronizationPoint will be updated after this
	/// X - for bounce
	/// Y - for frame
    virtual FSynchronizationPoint Submit(VkPipelineStageFlags& PipelineStageFlagsIn, FSynchronizationPoint SynchronizationPoint, uint32_t X, uint32_t Y);

    void RegisterInput(const std::string& InputName, ImagePtr Image);
    void RegisterOutput(const std::string& InputName, ImagePtr Image);
    void UpdateDescriptorSet(uint32_t LayoutSetIndex, uint32_t LayoutIndex, int FrameIndex, const FBuffer& Buffer);
    void UpdateDescriptorSet(uint32_t LayoutSetIndex, uint32_t LayoutIndex, int FrameIndex, ImagePtr Image);
    void UpdateDescriptorSet(uint32_t LayoutSetIndex, uint32_t LayoutIndex, int FrameIndex, ImagePtr Image, VkSampler Sampler);
	void SetDirty(uint32_t Flags);
    VkPipelineStageFlags GetPipelineStageFlags();
    ImagePtr GetInput(const std::string& InputName);
    ImagePtr GetOutput(const std::string& InputName);

	void ResetQueryPool(VkCommandBuffer CommandBuffer, uint32_t Index);
	std::vector<float> RequestTiming(uint32_t Y);
	std::vector<TaskState> TaskStates;

    std::unordered_map<std::string, ImagePtr> Inputs;
    std::unordered_map<std::string, ImagePtr> Outputs;

    std::vector<VkCommandBuffer> CommandBuffers;

    std::vector<VkSemaphore> SignalSemaphores;

    std::string Name;

    uint32_t Width = 0;
    uint32_t Height = 0;

    uint32_t SubmitX;
	uint32_t SubmitY;
	uint32_t TotalSize;
    VkDevice LogicalDevice = VK_NULL_HANDLE;
    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkPipeline Pipeline = VK_NULL_HANDLE;
    VkPipelineStageFlags PipelineStageFlags;
    VkQueueFlagBits QueueFlagsBits;
	VkQueryPool QueryPool = VK_NULL_HANDLE;
	uint32_t DitryFlags = true;

protected:
    void CreateSyncObjects();
    void FreeSyncObjects();
};