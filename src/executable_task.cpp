#include "executable_task.h"
#include "vk_context.h"
#include "vk_debug.h"

FExecutableTask::FExecutableTask(FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        Context(Context), NumberOfSimultaneousSubmits(NumberOfSimultaneousSubmits), LogicalDevice(LogicalDevice)
{
}

FExecutableTask::~FExecutableTask()
{

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