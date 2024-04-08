#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "common_structures.h"
#include "utils.h"

#include "vk_shader_compiler.h"

#include "task_material_sort_compute_prefix_sums_up_sweep.h"

#include <random>

FComputePrefixSumsUpSweepTask::FComputePrefixSumsUpSweepTask(uint32_t WidthIn, uint32_t HeightIn, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Material sort compute prefix sums up-sweep pipeline";

    auto& DescriptorSetManager = VK_CONTEXT().DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_UP_SWEEP_LAYOUT_INDEX, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_UP_SWEEP_BUFFER_A,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstantsPrefixSums)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
}

void FComputePrefixSumsUpSweepTask::Init()
{
    VK_CONTEXT().TimingManager->RegisterTiming(Name, NumberOfSimultaneousSubmits);

    auto& DescriptorSetManager = VK_CONTEXT().DescriptorSetManager;

    auto MaterialCountShader = FShader("../../../src/shaders/material_sort_compute_prefix_sums_up_sweep.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);
    Pipeline = VK_CONTEXT().CreateComputePipeline(MaterialCountShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_UP_SWEEP_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FComputePrefixSumsUpSweepTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        UpdateDescriptorSet(MATERIAL_SORT_COMPUTE_PREFIX_SUMS_UP_SWEEP_LAYOUT_INDEX, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_UP_SWEEP_BUFFER_A, i, VK_CONTEXT().ResourceAllocator->GetBuffer("CountedMaterialsPerChunkBuffer"));
    }
};

void FComputePrefixSumsUpSweepTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = VK_CONTEXT().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            VK_CONTEXT().TimingManager->TimestampStart(Name, CommandBuffer, i);

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto ComputeDescriptorSet = VK_CONTEXT().DescriptorSetManager->GetSet(Name, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_UP_SWEEP_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, VK_CONTEXT().DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &ComputeDescriptorSet, 0, nullptr);

            uint32_t GroupCount = CalculateGroupCount(Width * Height, BASIC_CHUNK_SIZE);
            uint32_t DMax = Log2(GroupCount);
            uint32_t TotalGroupCount = 2 << Log2(GroupCount);
            FPushConstantsPrefixSums PushConstantsPrefixSums = {0, TotalGroupCount};

            for (int D = 0; D <= DMax; ++D)
            {
                PushConstantsPrefixSums.D = D;
                vkCmdPushConstants(CommandBuffer, VK_CONTEXT().DescriptorSetManager->GetPipelineLayout(Name),
                                   VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstantsPrefixSums), &PushConstantsPrefixSums);
                TotalGroupCount /= 2;
                vkCmdDispatch(CommandBuffer, TotalGroupCount, 1, 1);

                VkMemoryBarrier MemoryBarrier = {};
                MemoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                MemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                MemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &MemoryBarrier, 0, nullptr, 0, nullptr);
            }

            VK_CONTEXT().TimingManager->TimestampEnd(Name, CommandBuffer, i);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], Name);
    }
};
