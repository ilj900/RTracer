#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

#include "vk_shader_compiler.h"

#include "task_accumulate.h"

FAccumulateTask::FAccumulateTask(int WidthIn, int HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, Context, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Accumulate pipeline";

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, ACCUMULATE_PER_FRAME_LAYOUT_INDEX, INCOMING_IMAGE_TO_SAMPLE,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, ACCUMULATE_PER_FRAME_LAYOUT_INDEX, ACCUMULATE_IMAGE_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, ACCUMULATE_PER_FRAME_LAYOUT_INDEX, ESTIMATED_IMAGE_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});

    DescriptorSetManager->CreateDescriptorSetLayout(Name);

    CreateSyncObjects();
}

FAccumulateTask::~FAccumulateTask()
{
    FreeSyncObjects();
}

void FAccumulateTask::Init()
{
    auto& DescriptorSetManager = Context->DescriptorSetManager;

    auto AccumulateShader = FShader("../shaders/accumulate.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

    Pipeline = Context->CreateComputePipeline(AccumulateShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, ACCUMULATE_PER_FRAME_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FAccumulateTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        VkDescriptorImageInfo InputImageInfo{};
        InputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        InputImageInfo.imageView = Inputs[0]->View;
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, ACCUMULATE_PER_FRAME_LAYOUT_INDEX, INCOMING_IMAGE_TO_SAMPLE, i, InputImageInfo);

        VkDescriptorImageInfo AccumulateImageInfo{};
        AccumulateImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        AccumulateImageInfo.imageView = Outputs[0]->View;
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, ACCUMULATE_PER_FRAME_LAYOUT_INDEX, ACCUMULATE_IMAGE_INDEX, i, AccumulateImageInfo);

        VkDescriptorImageInfo EstimatedImageInfo{};
        EstimatedImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        EstimatedImageInfo.imageView = Outputs[1]->View;
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, ACCUMULATE_PER_FRAME_LAYOUT_INDEX, ESTIMATED_IMAGE_INDEX, i, EstimatedImageInfo);
    }
};

void FAccumulateTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = GetContext().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto RayTracingDescriptorSet = Context->DescriptorSetManager->GetSet(Name, ACCUMULATE_PER_FRAME_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Context->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &RayTracingDescriptorSet, 0, nullptr);

            int GroupSizeX = (Width % 8 == 0) ? (Width / 8) : (Width / 8) + 1;
            int GroupSizeY = (Height % 8 == 0) ? (Height / 8) : (Height / 8) + 1;

            vkCmdDispatch(CommandBuffer, GroupSizeX, GroupSizeY, 1);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], "V::Accumulator_Command_Buffer");
    }
};

void FAccumulateTask::Cleanup()
{
    Inputs.clear();
    Outputs.clear();

    for (auto& CommandBuffer : CommandBuffers)
    {
        Context->CommandBufferManager->FreeCommandBuffer(CommandBuffer);
    }

    Context->DescriptorSetManager->DestroyPipelineLayout(Name);
    vkDestroyPipeline(LogicalDevice, Pipeline, nullptr);

    Context->DescriptorSetManager->Reset(Name);
};

VkSemaphore FAccumulateTask::Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex)
{
    VkSemaphore WaitSemaphores[] = {WaitSemaphore};
    VkPipelineStageFlags WaitStages[] = {VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT};
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
    if (vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    return SignalSemaphores[IterationIndex];
};
