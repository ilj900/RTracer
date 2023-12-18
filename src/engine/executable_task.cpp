#include "executable_task.h"
#include "vk_context.h"
#include "vk_debug.h"

FExecutableTask::FExecutableTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        Width(WidthIn), Height(HeightIn), Context(Context),
        NumberOfSimultaneousSubmits(NumberOfSimultaneousSubmits), LogicalDevice(LogicalDevice)
{
}

FExecutableTask::~FExecutableTask()
{

}

void FExecutableTask::CreateSyncObjects()
{
    for (int i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        SignalSemaphores.push_back(Context->CreateSemaphore());
        V::SetName(LogicalDevice, SignalSemaphores.back(), "V_" + Name + "_Semaphore_" + std::to_string(i));
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
    Inputs[Index] = Image;
}

void FExecutableTask::RegisterOutput(int Index, ImagePtr Image)
{
    if (Outputs.size() <= Index)
    {
        Outputs.resize(Index + 1);
    }
    Outputs[Index] = Image;
}

VkSemaphore FExecutableTask::Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkPipelineStageFlags PipelineStageFlags, VkFence WaitFence, VkFence SignalFence, int IterationIndex)
{
    VkSemaphore WaitSemaphores[] = {WaitSemaphore};
    VkPipelineStageFlags WaitStages[] = {PipelineStageFlags};
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
    Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, LayoutSetIndex, LayoutIndex, FrameIndex, &BufferInfo);
}

void FExecutableTask::UpdateDescriptorSet(uint32_t LayoutSetIndex, uint32_t LayoutIndex, int FrameIndex, ImagePtr Image)
{
    VkDescriptorImageInfo ImageInfo{};
    ImageInfo.imageLayout = Image->CurrentLayout;
    ImageInfo.imageView = Image->View;
    Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, LayoutSetIndex, LayoutIndex, FrameIndex, &ImageInfo);
}

void FExecutableTask::UpdateDescriptorSet(uint32_t LayoutSetIndex, uint32_t LayoutIndex, int FrameIndex, ImagePtr Image, VkSampler Sampler)
{
    VkDescriptorImageInfo ImageInfo{};
    ImageInfo.imageLayout = Image->CurrentLayout;
    ImageInfo.imageView = Image->View;
    ImageInfo.sampler = Sampler;
    Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, LayoutSetIndex, LayoutIndex, FrameIndex, &ImageInfo);
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