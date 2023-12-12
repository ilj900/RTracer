#include "executable_task.h"
#include "vk_context.h"
#include "vk_debug.h"

FExecutableTask::FExecutableTask(int WidthIn, int HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
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