#include "executable_task.h"

#include <utility>
#include "vk_context.h"
#include "vk_debug.h"

FExecutableTask::FExecutableTask(uint32_t WidthIn, uint32_t HeightIn, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        Width(WidthIn), Height(HeightIn),
        NumberOfSimultaneousSubmits(NumberOfSimultaneousSubmits), LogicalDevice(LogicalDevice)
{
    CreateSyncObjects();
}

FExecutableTask::~FExecutableTask()
{
    Inputs.clear();
    Outputs.clear();

    for (auto& CommandBuffer : CommandBuffers)
    {
        COMMAND_BUFFER_MANAGER()->FreeCommandBuffer(CommandBuffer, QueueFlagsBits);
    }

    VK_CONTEXT().DescriptorSetManager->DestroyPipelineLayout(Name);

    if (Pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(LogicalDevice, Pipeline, nullptr);
    }

    VK_CONTEXT().DescriptorSetManager->Reset(Name);

    FreeSyncObjects();
}

void FExecutableTask::CreateSyncObjects()
{
    for (int i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        SignalSemaphores.push_back(VK_CONTEXT().CreateSemaphore());
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

VkSemaphore FExecutableTask::Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkPipelineStageFlags PipelineStageFlagsIn, VkFence WaitFence, VkFence SignalFence, uint32_t IterationIndex)
{
    VkSemaphore WaitSemaphores[] = {WaitSemaphore};
    VkPipelineStageFlags WaitStages[] = {PipelineStageFlagsIn};
    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.waitSemaphoreCount = 1;
    SubmitInfo.pWaitSemaphores = WaitSemaphores;
    SubmitInfo.pWaitDstStageMask = WaitStages;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CommandBuffers[IterationIndex];

    VkSemaphore Semaphores[] = {SignalSemaphores[IterationIndex]};
    SubmitInfo.signalSemaphoreCount = 1;
    SubmitInfo.pSignalSemaphores = Semaphores;

    if(WaitFence != VK_NULL_HANDLE)
    {
        vkWaitForFences(LogicalDevice, 1, &WaitFence, VK_TRUE, UINT64_MAX);
    }

    if (SignalFence != VK_NULL_HANDLE)
    {
        vkResetFences(LogicalDevice, 1, &SignalFence);
    }

    /// Submit computing. When computing finished, appropriate fence will be signalled
    if (vkQueueSubmit(Queue, 1, &SubmitInfo, SignalFence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    return SignalSemaphores[IterationIndex];
}

void FExecutableTask::UpdateDescriptorSet(uint32_t LayoutSetIndex, uint32_t LayoutIndex, int FrameIndex, const FBuffer& Buffer)
{
    VkDescriptorBufferInfo BufferInfo{};
    BufferInfo.buffer = Buffer.Buffer;
    BufferInfo.offset = 0;
    BufferInfo.range = Buffer.BufferSize;
    VK_CONTEXT().DescriptorSetManager->UpdateDescriptorSetInfo(Name, LayoutSetIndex, LayoutIndex, FrameIndex, &BufferInfo);
}

void FExecutableTask::UpdateDescriptorSet(uint32_t LayoutSetIndex, uint32_t LayoutIndex, int FrameIndex, ImagePtr Image)
{
    VkDescriptorImageInfo ImageInfo{};
    ImageInfo.imageLayout = Image->CurrentLayout;
    ImageInfo.imageView = Image->View;
    VK_CONTEXT().DescriptorSetManager->UpdateDescriptorSetInfo(Name, LayoutSetIndex, LayoutIndex, FrameIndex, &ImageInfo);
}

void FExecutableTask::UpdateDescriptorSet(uint32_t LayoutSetIndex, uint32_t LayoutIndex, int FrameIndex, ImagePtr Image, VkSampler Sampler)
{
    VkDescriptorImageInfo ImageInfo{};
    ImageInfo.imageLayout = Image->CurrentLayout;
    ImageInfo.imageView = Image->View;
    ImageInfo.sampler = Sampler;
    VK_CONTEXT().DescriptorSetManager->UpdateDescriptorSetInfo(Name, LayoutSetIndex, LayoutIndex, FrameIndex, &ImageInfo);
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