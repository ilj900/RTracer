#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "common_structures.h"
#include "utils.h"

#include "vk_shader_compiler.h"

#include "task_material_sort_compute_prefix_sums_zero_out.h"

#include <random>

FComputePrefixSumsZeroOutTask::FComputePrefixSumsZeroOutTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, Context, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Material sort compute prefix sums zero out pipeline";

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_ZERO_OUT_LAYOUT_INDEX, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_ZERO_OUT_BUFFER_A,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_ZERO_OUT_LAYOUT_INDEX, MATERIAL_SORT_TOTAL_MATERIAL_OFFSETS_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstantsPrefixSums)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    CreateSyncObjects();
}

FComputePrefixSumsZeroOutTask::~FComputePrefixSumsZeroOutTask()
{
    FreeSyncObjects();
}

void FComputePrefixSumsZeroOutTask::Init()
{
    Context->TimingManager->RegisterTiming(Name, NumberOfSimultaneousSubmits);

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    auto MaterialCountShader = FShader("../../../src/shaders/material_sort_prefix_sums_zero_out.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);
    Pipeline = Context->CreateComputePipeline(MaterialCountShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_ZERO_OUT_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FComputePrefixSumsZeroOutTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        UpdateDescriptorSet(MATERIAL_SORT_COMPUTE_PREFIX_SUMS_ZERO_OUT_LAYOUT_INDEX, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_ZERO_OUT_BUFFER_A, i, Context->ResourceAllocator->GetBuffer("CountedMaterialsPerChunkBuffer"));
        UpdateDescriptorSet(MATERIAL_SORT_COMPUTE_PREFIX_SUMS_ZERO_OUT_LAYOUT_INDEX, MATERIAL_SORT_TOTAL_MATERIAL_OFFSETS_BUFFER, i, Context->ResourceAllocator->GetBuffer("TotalCountedMaterialsBuffer"));
    }
};

void FComputePrefixSumsZeroOutTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = GetContext().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            Context->TimingManager->TimestampStart(Name, CommandBuffer, i);

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto ComputeDescriptorSet = Context->DescriptorSetManager->GetSet(Name, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_ZERO_OUT_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Context->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &ComputeDescriptorSet, 0, nullptr);

            uint32_t GroupCount = CalculateGroupCount(Width * Height, BASIC_CHUNK_SIZE);
            uint32_t TotalGroupCount = 2 << Log2(GroupCount);
            FPushConstantsPrefixSums PushConstantsPrefixSums = {0, TotalGroupCount};

            vkCmdPushConstants(CommandBuffer, Context->DescriptorSetManager->GetPipelineLayout(Name),
                               VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstantsPrefixSums), &PushConstantsPrefixSums);
            vkCmdDispatch(CommandBuffer, 1, 1, 1);

            Context->TimingManager->TimestampEnd(Name, CommandBuffer, i);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], "V::MaterialSort_Compute_Prefix_Sums_Zero_Out_Command_Buffer");
    }
};

void FComputePrefixSumsZeroOutTask::Cleanup()
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
