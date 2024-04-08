#pragma once

#include "executable_task.h"
#include "vk_pipeline.h"

class FVulkanContext;

class FImguiTask : public FExecutableTask
{
public:
    FImguiTask(uint32_t WidthIn, uint32_t HeightIn, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
    ~FImguiTask() override;

    void Init() override;
    void UpdateDescriptorSets() override;
    void RecordCommands() override;
    VkSemaphore Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkPipelineStageFlags PipelineStageFlags, VkFence WaitFence, VkFence SignalFence, uint32_t IterationIndex) override;

    FGraphicsPipelineOptions GraphicsPipelineOptions;

    VkRenderPass RenderPass = VK_NULL_HANDLE;

    VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> ImguiFramebuffers;

    int PreviousIterationIndex = 0;
    bool bFirstCall = true;
};