#pragma once

#include "executable_task.h"
#include "vk_pipeline.h"

class FVulkanContext;

class FImguiTask : public FExecutableTask
{
public:
    FImguiTask(FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
    ~FImguiTask();

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
    void Cleanup() override;
    VkSemaphore Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex) override;

    FGraphicsPipelineOptions GraphicsPipelineOptions;

    VkRenderPass RenderPass = VK_NULL_HANDLE;

    VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> ImguiFramebuffers;
};