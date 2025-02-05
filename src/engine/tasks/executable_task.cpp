#include "executable_task.h"

#include <utility>
#include "vk_context.h"
#include "vk_debug.h"

bool CheckFlag(uint32_t Flags, uint32_t Flag)
{
	return (Flags & Flag) == Flag;
}

FGPUTimer::FGPUTimer(VkCommandBuffer CommandBufferIn, VkPipelineStageFlagBits PipelineStageFlagBitsStartIn, VkPipelineStageFlagBits PipelineStageFlagBitsEndIn, VkQueryPool QueryPoolIn, uint32_t QueryIn) :
	CommandBuffer(CommandBufferIn), PipelineStageFlagBitsStart(PipelineStageFlagBitsStartIn), PipelineStageFlagBitsEnd(PipelineStageFlagBitsEndIn), QueryPool(QueryPoolIn), Query(QueryIn)
{
	vkCmdWriteTimestamp(CommandBuffer, PipelineStageFlagBitsStart, QueryPool, Query * 2);
}

FGPUTimer::~FGPUTimer()
{
	vkCmdWriteTimestamp(CommandBuffer, PipelineStageFlagBitsEnd, QueryPool, Query * 2 + 1);
}

FExecutableTask::FExecutableTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
        Width(WidthIn), Height(HeightIn),
        SubmitX(SubmitXIn), SubmitY(SubmitYIn), LogicalDevice(LogicalDevice), TaskStates(SubmitYIn, UNDEFINED)
{
	TotalSize = SubmitX * SubmitY;
    CreateSyncObjects();
	DitryFlags |= UNINITIALIZED | OUTDATED_DESCRIPTOR_SET | OUTDATED_COMMAND_BUFFER;
	QueryPool = VK_CONTEXT()->CreateQueryPool( SubmitXIn * SubmitYIn * 2, VK_QUERY_TYPE_TIMESTAMP);
}

FExecutableTask::~FExecutableTask()
{
    Inputs.clear();
    Outputs.clear();

    for (auto& CommandBuffer : CommandBuffers)
    {
        COMMAND_BUFFER_MANAGER()->FreeCommandBuffer(CommandBuffer, QueueFlagsBits);
    }

    VK_CONTEXT()->DescriptorSetManager->DestroyPipelineLayout(Name);

    if (Pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(LogicalDevice, Pipeline, nullptr);
    }

    VK_CONTEXT()->DescriptorSetManager->Reset(Name);

    FreeSyncObjects();

	vkDestroyQueryPool(LogicalDevice, QueryPool, nullptr);
}

void FExecutableTask::CreateSyncObjects()
{
	SignalSemaphores.reserve(TotalSize);

    for (int i = 0; i < TotalSize; ++i)
    {
        SignalSemaphores.push_back(VK_CONTEXT()->CreateSemaphore());
        V::SetName(LogicalDevice, SignalSemaphores.back(), Name);
    }
}

void FExecutableTask::FreeSyncObjects()
{
    for (int i = 0; i < TotalSize; ++i)
    {
        if (SignalSemaphores[i] != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(LogicalDevice, SignalSemaphores[i], nullptr);
            SignalSemaphores[i] = VK_NULL_HANDLE;
        }
    }

    SignalSemaphores.clear();
}

void FExecutableTask::RegisterInput(const std::string& InputName, ImagePtr Image)
{
    if (Inputs.find(InputName) != Inputs.end())
    {
		throw std::runtime_error("An attempt to override an existing Input");
    }

    Inputs[InputName] = std::move(Image);
}

void FExecutableTask::RegisterOutput(const std::string& InputName, ImagePtr Image)
{
	if (Inputs.find(InputName) != Inputs.end())
	{
		throw std::runtime_error("An attempt to override an existing Output");
	}

    Outputs[InputName] = std::move(Image);
}

void FExecutableTask::Reload(FCompileDefinitions* CompileDefinitions)
{
	if (CheckFlag(DitryFlags, DirtyType::UNINITIALIZED))
	{
		Init(CompileDefinitions);
	}

	if (CheckFlag(DitryFlags, DirtyType::OUTDATED_DESCRIPTOR_SET))
	{
		UpdateDescriptorSets();
	}

	if (CheckFlag(DitryFlags, DirtyType::OUTDATED_COMMAND_BUFFER))
	{
		RecordCommands();
	}

	DitryFlags = 0u;
}

FSynchronizationPoint FExecutableTask::Submit(VkPipelineStageFlags& PipelineStageFlagsIn, FSynchronizationPoint SynchronizationPoint, uint32_t X, uint32_t Y)
{
	uint32_t SubmitIndex = Y * SubmitX + X;

    VkPipelineStageFlags WaitStages[] = {PipelineStageFlagsIn};
    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.waitSemaphoreCount = SynchronizationPoint.SemaphoresToWait.size();
    SubmitInfo.pWaitSemaphores = SynchronizationPoint.SemaphoresToWait.empty() ? nullptr : SynchronizationPoint.SemaphoresToWait.data();
    SubmitInfo.pWaitDstStageMask = WaitStages;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CommandBuffers[SubmitIndex];

	/// We push the task's semaphore to the list of signal semaphores
	SynchronizationPoint.SemaphoresToSignal.push_back(SignalSemaphores[SubmitIndex]);
    SubmitInfo.signalSemaphoreCount = SynchronizationPoint.SemaphoresToSignal.size();
    SubmitInfo.pSignalSemaphores = SynchronizationPoint.SemaphoresToSignal.data();

    if(!SynchronizationPoint.FencesToWait.empty())
    {
		vkWaitForFences(LogicalDevice, SynchronizationPoint.FencesToWait.size(), SynchronizationPoint.FencesToWait.data(), VK_TRUE, UINT64_MAX);
    }

	VkFence FenceToSignal = VK_NULL_HANDLE;

    if (!SynchronizationPoint.FencesToSignal.empty())
    {
#ifndef NDEBUG
		if (SynchronizationPoint.FencesToSignal.size() > 1)
			U::Log("There are more that one fence to signal. This might be an error");
#endif
        vkResetFences(LogicalDevice, SynchronizationPoint.FencesToSignal.size(), SynchronizationPoint.FencesToSignal.data());
		FenceToSignal = SynchronizationPoint.FencesToSignal[0];
    }

    /// Submit computing. When computing finished, appropriate fence will be signalled
    if (vkQueueSubmit(VK_CONTEXT()->GetQueue(QueueFlagsBits), 1, &SubmitInfo, FenceToSignal) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

	PipelineStageFlagsIn = PipelineStageFlags;
	TaskStates[Y] = SUBMITTED;

	return {{SignalSemaphores[SubmitIndex]}, {}, {}, {}};
}

void FExecutableTask::UpdateDescriptorSet(uint32_t LayoutSetIndex, uint32_t LayoutIndex, int FrameIndex, const FBuffer& Buffer)
{
    VkDescriptorBufferInfo BufferInfo{};
    BufferInfo.buffer = Buffer.Buffer;
    BufferInfo.offset = 0;
    BufferInfo.range = Buffer.BufferSize;
    VK_CONTEXT()->DescriptorSetManager->UpdateDescriptorSetInfo(Name, LayoutSetIndex, LayoutIndex, FrameIndex, &BufferInfo);
}

void FExecutableTask::UpdateDescriptorSet(uint32_t LayoutSetIndex, uint32_t LayoutIndex, int FrameIndex, ImagePtr Image)
{
    VkDescriptorImageInfo ImageInfo{};
    ImageInfo.imageLayout = Image->CurrentLayout;
    ImageInfo.imageView = Image->View;
    VK_CONTEXT()->DescriptorSetManager->UpdateDescriptorSetInfo(Name, LayoutSetIndex, LayoutIndex, FrameIndex, &ImageInfo);
}

void FExecutableTask::UpdateDescriptorSet(uint32_t LayoutSetIndex, uint32_t LayoutIndex, int FrameIndex, ImagePtr Image, VkSampler Sampler)
{
    VkDescriptorImageInfo ImageInfo{};
    ImageInfo.imageLayout = Image->CurrentLayout;
    ImageInfo.imageView = Image->View;
    ImageInfo.sampler = Sampler;
    VK_CONTEXT()->DescriptorSetManager->UpdateDescriptorSetInfo(Name, LayoutSetIndex, LayoutIndex, FrameIndex, &ImageInfo);
}

void FExecutableTask::SetDirty(uint32_t Flags)
{
	DitryFlags |= Flags;
}

VkPipelineStageFlags FExecutableTask::GetPipelineStageFlags()
{
    return PipelineStageFlags;
}

ImagePtr FExecutableTask::GetInput(const std::string& InputName)
{
	if (Inputs.find(InputName) != Inputs.end())
	{
		return Inputs[InputName];
	}

    throw std::runtime_error("No image registered for the input " + InputName);
}

ImagePtr FExecutableTask::GetOutput(const std::string& InputName)
{
	if (Outputs.find(InputName) != Outputs.end())
    {
        return Outputs[InputName];
    }
    throw std::runtime_error("No image registered for the output " + InputName);
}

void FExecutableTask::ResetQueryPool(VkCommandBuffer CommandBuffer, uint32_t Index)
{
	if ((Index % SubmitX) == 0)
	{
		uint32_t Y = Index / SubmitX;
		vkCmdResetQueryPool(CommandBuffer, QueryPool, Y * 2 * SubmitX, 2 * SubmitX);
	}
}

std::vector<float> FExecutableTask::RequestTiming(uint32_t Y)
{
	if (TaskStates[Y] != SUBMITTED)
	{
		return {};
	}

	std::vector<uint64_t> TimeStamps(SubmitX * 2);
	vkGetQueryPoolResults(VK_CONTEXT()->LogicalDevice, QueryPool, Y * 2 * SubmitX, 2 * SubmitX, sizeof(uint64_t) * TimeStamps.size(), TimeStamps.data(), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

	std::vector<float> Timings(SubmitX, 0.f);

	for (int i = 0; i < SubmitX; ++i)
	{
		Timings[i] = float(TimeStamps[i * 2 + 1] - TimeStamps[i * 2]) * VK_CONTEXT()->TimestampPeriod / 1000000000.f;
	}

	TaskStates[Y] = QUERIED;

	return Timings;
}