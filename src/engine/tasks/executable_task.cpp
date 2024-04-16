#include "executable_task.h"

#include <utility>
#include "vk_context.h"
#include "vk_debug.h"

bool CheckFlag(uint32_t Flags, uint32_t Flag)
{
	return (Flags & Flag) == Flag;
}

FExecutableTask::FExecutableTask(uint32_t WidthIn, uint32_t HeightIn, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        Width(WidthIn), Height(HeightIn),
        NumberOfSimultaneousSubmits(NumberOfSimultaneousSubmits), LogicalDevice(LogicalDevice)
{
    CreateSyncObjects();
	DitryFlags |= UNINITIALIZED | OUTDATED_DESCRIPTOR_SET | OUTDATED_COMMAND_BUFFER;
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
}

void FExecutableTask::CreateSyncObjects()
{
    for (int i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        SignalSemaphores.push_back(VK_CONTEXT()->CreateSemaphore());
        V::SetName(LogicalDevice, SignalSemaphores.back(), Name);
    }
}

void FExecutableTask::FreeSyncObjects()
{
    for (int i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        if (SignalSemaphores[i] != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(LogicalDevice, SignalSemaphores[i], nullptr);
            SignalSemaphores[i] = VK_NULL_HANDLE;
        }
    }

    SignalSemaphores.clear();
}

void FExecutableTask::RegisterInput(int Index, ImagePtr Image)
{
    if (Inputs.size() <= Index)
    {
        Inputs.resize(Index + 1);
    }
    Inputs[Index] = std::move(Image);
}

void FExecutableTask::RegisterOutput(int Index, ImagePtr Image)
{
    if (Outputs.size() <= Index)
    {
        Outputs.resize(Index + 1);
    }
    Outputs[Index] = std::move(Image);
}

void FExecutableTask::Reload()
{
	if (CheckFlag(DitryFlags, DirtyType::UNINITIALIZED))
	{
		Init();
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

VkSemaphore FExecutableTask::Submit(VkQueue Queue, VkPipelineStageFlags& PipelineStageFlagsIn, FSynchronizationPoint SynchronizationPoint, uint32_t IterationIndex)
{
    VkPipelineStageFlags WaitStages[] = {PipelineStageFlagsIn};
    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.waitSemaphoreCount = SynchronizationPoint.SemaphoresToWait.size();
    SubmitInfo.pWaitSemaphores = SynchronizationPoint.SemaphoresToWait.empty() ? nullptr : SynchronizationPoint.SemaphoresToWait.data();
    SubmitInfo.pWaitDstStageMask = WaitStages;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CommandBuffers[IterationIndex];

	/// We push the task's semaphore to the list of signal semaphores
	SynchronizationPoint.SemaphoresToSignal.push_back(SignalSemaphores[IterationIndex]);
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
    if (vkQueueSubmit(Queue, 1, &SubmitInfo, FenceToSignal) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

	PipelineStageFlagsIn = PipelineStageFlags;

    return SignalSemaphores[IterationIndex];
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

ImagePtr FExecutableTask::GetInput(int Index)
{
    if (Inputs.size() > Index)
    {
        return Inputs[Index];
    }
    throw std::runtime_error("Wrong input index.");
}

ImagePtr FExecutableTask::GetOutput(int Index)
{
    if (Outputs.size() > Index)
    {
        return Outputs[Index];
    }
    throw std::runtime_error("Wrong output index.");
}