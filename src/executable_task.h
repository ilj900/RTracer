#pragma once

#include "image.h"
#include "vk_pipeline.h"

class FVulkanContext;

class FExecutableTask
{
public:
    FExecutableTask(FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);

    virtual void Init() = 0;
    virtual void UpdateDescriptorSets() = 0;
    virtual void RecordCommands() = 0;
    virtual void Cleanup() = 0;
    virtual VkSemaphore Submit(VkQueue Queue, VkSemaphore WaitSemaphore, int IterationIndex) = 0;

    void RegisterInput(int Index, ImagePtr Image);
    void RegisterOutput(int Index, ImagePtr Image);
    ImagePtr GetInput(int Index);
    ImagePtr GetOutput(int Index);

    std::vector<ImagePtr> Inputs;
    std::vector<ImagePtr> Outputs;

    std::vector<VkCommandBuffer> CommandBuffers;

    std::vector<VkSemaphore> SignalSemaphores;

    std::string Name;

    FVulkanContext* Context = nullptr;
    int NumberOfSimultaneousSubmits;
    VkDevice LogicalDevice = VK_NULL_HANDLE;

};