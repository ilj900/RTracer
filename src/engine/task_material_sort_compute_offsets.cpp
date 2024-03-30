#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "utils.h"

#include "vk_shader_compiler.h"

#include "task_material_sort_compute_offsets.h"

FComputeOffsetsTask::FComputeOffsetsTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, Context, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Material sort compute offsets pipeline";

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_COMPUTE_OFFSETS_LAYOUT_INDEX, MATERIAL_SORT_MATERIALS_COUNT_PER_CHUNK_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_COMPUTE_OFFSETS_LAYOUT_INDEX, MATERIAL_SORT_TOTAL_MATERIAL_OFFSETS_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_COMPUTE_OFFSETS_LAYOUT_INDEX, MATERIAL_SORT_MATERIALS_OFFSETS_PER_CHUNK_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    FBuffer MaterialsOffsetsPerChunkBuffer = Context->ResourceAllocator->CreateBuffer(sizeof(uint32_t) * TOTAL_MATERIALS * CalculateGroupCount(Width * Height, BASIC_CHUNK_SIZE), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "MaterialsOffsetsPerChunkBuffer");
    Context->ResourceAllocator->RegisterBuffer(MaterialsOffsetsPerChunkBuffer, "MaterialsOffsetsPerChunkBuffer");

    VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    CreateSyncObjects();
}

FComputeOffsetsTask::~FComputeOffsetsTask()
{
    FreeSyncObjects();
    Context->ResourceAllocator->UnregisterAndDestroyBuffer("MaterialsOffsetsPerChunkBuffer");
}

void FComputeOffsetsTask::Init()
{
    Context->TimingManager->RegisterTiming(Name, NumberOfSimultaneousSubmits);

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    auto MaterialCountShader = FShader("../../../src/shaders/material_sort_compute_offsets.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);
    Pipeline = Context->CreateComputePipeline(MaterialCountShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, MATERIAL_SORT_COMPUTE_OFFSETS_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FComputeOffsetsTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        UpdateDescriptorSet(MATERIAL_SORT_COMPUTE_OFFSETS_LAYOUT_INDEX, MATERIAL_SORT_MATERIALS_COUNT_PER_CHUNK_BUFFER, i, Context->ResourceAllocator->GetBuffer("CountedMaterialsPerChunkBuffer"));
        UpdateDescriptorSet(MATERIAL_SORT_COMPUTE_OFFSETS_LAYOUT_INDEX, MATERIAL_SORT_TOTAL_MATERIAL_OFFSETS_BUFFER, i, Context->ResourceAllocator->GetBuffer("TotalCountedMaterialsBuffer"));
        UpdateDescriptorSet(MATERIAL_SORT_COMPUTE_OFFSETS_LAYOUT_INDEX, MATERIAL_SORT_MATERIALS_OFFSETS_PER_CHUNK_BUFFER, i, Context->ResourceAllocator->GetBuffer("MaterialsOffsetsPerChunkBuffer"));
    }
};

void FComputeOffsetsTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = GetContext().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            Context->TimingManager->TimestampStart(Name, CommandBuffer, i);

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto ComputeDescriptorSet = Context->DescriptorSetManager->GetSet(Name, MATERIAL_SORT_COMPUTE_OFFSETS_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Context->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &ComputeDescriptorSet, 0, nullptr);

            for (int i = 0; i < CalculateGroupCount(Width * Height, BASIC_CHUNK_SIZE); ++i)
            {
                vkCmdPushConstants(CommandBuffer, Context->DescriptorSetManager->GetPipelineLayout(Name),
                                   VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &i);

                vkCmdDispatch(CommandBuffer, 1, 1, 1);

                VkMemoryBarrier MemoryBarrier{};
                MemoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                MemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                MemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &MemoryBarrier, 0, nullptr, 0, nullptr);
            }

            Context->TimingManager->TimestampEnd(Name, CommandBuffer, i);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], "V::MaterialSort_Compute_Offsets_Command_Buffer");
    }
};

void FComputeOffsetsTask::Cleanup()
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

VkSemaphore FComputeOffsetsTask::Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex)
{
    return FExecutableTask::Submit(Queue, WaitSemaphore, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, WaitFence, SignalFence, IterationIndex);
};
