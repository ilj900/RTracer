#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

#include "vk_shader_compiler.h"

#include "task_clear_image.h"
#include "common_defines.h"

#include <iostream>
#include <iomanip>

FClearImageTask::FClearImageTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, Context, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Clear image pipeline";

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, CLEAR_IMAGE_LAYOUT_INDEX, IMAGE_TO_CLEAR,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});

    DescriptorSetManager->CreateDescriptorSetLayout({}, Name);

    CreateSyncObjects();
}

FClearImageTask::~FClearImageTask()
{
    FreeSyncObjects();
}

void FClearImageTask::Init()
{
    FExecutableTask::Init();

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    auto ClearImageShader = FShader("../../../src/shaders/clear_image.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

    Pipeline = Context->CreateComputePipeline(ClearImageShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, CLEAR_IMAGE_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FClearImageTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        UpdateDescriptorSet(CLEAR_IMAGE_LAYOUT_INDEX, IMAGE_TO_CLEAR, i, Outputs[0]);
    }
};

void FClearImageTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = GetContext().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            vkCmdResetQueryPool(CommandBuffer, QueryPool, i * 2, 2);
            vkCmdWriteTimestamp(CommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, QueryPool, i * 2);

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto RayTracingDescriptorSet = Context->DescriptorSetManager->GetSet(Name, CLEAR_IMAGE_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Context->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &RayTracingDescriptorSet, 0, nullptr);

            int GroupSizeX = (Width % 8 == 0) ? (Width / 8) : (Width / 8) + 1;
            int GroupSizeY = (Height % 8 == 0) ? (Height / 8) : (Height / 8) + 1;

            vkCmdDispatch(CommandBuffer, GroupSizeX, GroupSizeY, 1);

            vkCmdWriteTimestamp(CommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, QueryPool, i * 2 + 1);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], "V::ClearImage_Command_Buffer");
    }
};

void FClearImageTask::Cleanup()
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

VkSemaphore FClearImageTask::Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex)
{
    auto Result = FExecutableTask::Submit(Queue, WaitSemaphore, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, WaitFence, SignalFence, IterationIndex);

    static std::vector<uint64_t> TimeStamps(NumberOfSimultaneousSubmits * 2);
    vkGetQueryPoolResults(LogicalDevice, QueryPool, IterationIndex * 2, 2, sizeof(uint64_t) * 2, TimeStamps.data() + IterationIndex * 2, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    uint64_t Delta = (TimeStamps[IterationIndex * 2 + 1] - TimeStamps[IterationIndex * 2]);
    std::cout << std::setprecision(2) << Name << " delta in ms:" << (float(Delta) / 1000000.f) << std::endl;
    return Result;
};
