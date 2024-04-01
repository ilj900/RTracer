#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "common_structures.h"
#include "utils.h"

#include "vk_shader_compiler.h"

#include "task_material_sort_compute_prefix_sums.h"

FComputePrefixSumsTask::FComputePrefixSumsTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, Context, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Material sort sort materials pipeline";

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_LAYOUT_INDEX, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_BUFFER_A,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_LAYOUT_INDEX, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_BUFFER_B,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    FBuffer BufferA = Context->ResourceAllocator->CreateBuffer(sizeof(uint32_t) * TOTAL_MATERIALS * CalculateGroupCount(Width * Height, BASIC_CHUNK_SIZE), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "BufferA");
    Context->ResourceAllocator->RegisterBuffer(BufferA, "BufferA");

    FBuffer BufferB = Context->ResourceAllocator->CreateBuffer(sizeof(uint32_t) * TOTAL_MATERIALS * CalculateGroupCount(Width * Height, BASIC_CHUNK_SIZE), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "BufferB");
    Context->ResourceAllocator->RegisterBuffer(BufferB, "BufferB");

    VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstants)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    CreateSyncObjects();
}

FComputePrefixSumsTask::~FComputePrefixSumsTask()
{
    FreeSyncObjects();
    Context->ResourceAllocator->UnregisterAndDestroyBuffer("BufferA");
    Context->ResourceAllocator->UnregisterAndDestroyBuffer("BufferB");
}

void FComputePrefixSumsTask::Init()
{
    Context->TimingManager->RegisterTiming(Name, NumberOfSimultaneousSubmits);

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    auto MaterialCountShader = FShader("../../../src/shaders/material_sort_compute_prefix_sums.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);
    Pipeline = Context->CreateComputePipeline(MaterialCountShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FComputePrefixSumsTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        UpdateDescriptorSet(MATERIAL_SORT_COMPUTE_PREFIX_SUMS_LAYOUT_INDEX, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_BUFFER_A, i, Context->ResourceAllocator->GetBuffer("BufferA"));
        UpdateDescriptorSet(MATERIAL_SORT_COMPUTE_PREFIX_SUMS_LAYOUT_INDEX, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_BUFFER_B, i, Context->ResourceAllocator->GetBuffer("BufferB"));
    }
};

void FComputePrefixSumsTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = GetContext().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            Context->TimingManager->TimestampStart(Name, CommandBuffer, i);

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto ComputeDescriptorSet = Context->DescriptorSetManager->GetSet(Name, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Context->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &ComputeDescriptorSet, 0, nullptr);

            FPushConstants PushConstants = {Width, Height, 1.f / Width, 1.f / Height, Width * Height, 0};
            vkCmdPushConstants(CommandBuffer, Context->DescriptorSetManager->GetPipelineLayout(Name),
                               VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstants), &PushConstants);

            vkCmdDispatch(CommandBuffer, CalculateGroupCount(Width * Height, BASIC_CHUNK_SIZE), 1, 1);

            Context->TimingManager->TimestampEnd(Name, CommandBuffer, i);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], "V::MaterialSort_Sort_Materials_Command_Buffer");
    }
};

void FComputePrefixSumsTask::Cleanup()
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

VkSemaphore FComputePrefixSumsTask::Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex)
{
    return FExecutableTask::Submit(Queue, WaitSemaphore, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, WaitFence, SignalFence, IterationIndex);
};
