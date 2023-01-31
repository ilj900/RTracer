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