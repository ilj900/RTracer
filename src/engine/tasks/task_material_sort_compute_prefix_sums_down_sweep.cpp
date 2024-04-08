#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "common_structures.h"
#include "utils.h"

#include "vk_shader_compiler.h"

#include "task_material_sort_compute_prefix_sums_down_sweep.h"

#include <random>

FComputePrefixSumsDownSweepTask::FComputePrefixSumsDownSweepTask(uint32_t WidthIn, uint32_t HeightIn, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Material sort compute prefix sums down-sweep pipeline";

    auto& DescriptorSetManager = VK_CONTEXT().DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_DOWN_SWEEP_LAYOUT_INDEX, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_DOWN_SWEEP_BUFFER_A,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstantsPrefixSums)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
}

void FComputePrefixSumsDownSweepTask::Init()
{
    TIMING_MANAGER()->RegisterTiming(Name, NumberOfSimultaneousSubmits);

    auto& DescriptorSetManager = VK_CONTEXT().DescriptorSetManager;

    auto MaterialCountShader = FShader("../../../src/shaders/material_sort_compute_prefix_sums_down_sweep.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);
    Pipeline = VK_CONTEXT().CreateComputePipeline(MaterialCountShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_DOWN_SWEEP_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FComputePrefixSumsDownSweepTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        UpdateDescriptorSet(MATERIAL_SORT_COMPUTE_PREFIX_SUMS_DOWN_SWEEP_LAYOUT_INDEX, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_DOWN_SWEEP_BUFFER_A, i, RESOURCE_ALLOCATOR()->GetBuffer("CountedMaterialsPerChunkBuffer"));
    }
};

void FComputePrefixSumsDownSweepTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = VK_CONTEXT().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            TIMING_MANAGER()->TimestampStart(Name, CommandBuffer, i);

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto ComputeDescriptorSet = VK_CONTEXT().DescriptorSetManager->GetSet(Name, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_DOWN_SWEEP_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, VK_CONTEXT().DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &ComputeDescriptorSet, 0, nullptr);

            uint32_t GroupCount = CalculateGroupCount(Width * Height, BASIC_CHUNK_SIZE);
            uint32_t DMax = Log2(GroupCount);
            uint32_t TotalGroupCount = 1;
            FPushConstantsPrefixSums PushConstantsPrefixSums = {0, TotalGroupCount};

            for (int D = DMax; D >= 0; --D)
            {
                PushConstantsPrefixSums.D = D;
                vkCmdPushConstants(CommandBuffer, VK_CONTEXT().DescriptorSetManager->GetPipelineLayout(Name),
                                   VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstantsPrefixSums), &PushConstantsPrefixSums);

                vkCmdDispatch(CommandBuffer, TotalGroupCount, 1, 1);
                TotalGroupCount *= 2;

                VkMemoryBarrier MemoryBarrier = {};
                MemoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                MemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                MemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &MemoryBarrier, 0, nullptr, 0, nullptr);
            }

            TIMING_MANAGER()->TimestampEnd(Name, CommandBuffer, i);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], Name);
    }
};
