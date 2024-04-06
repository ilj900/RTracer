#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "common_structures.h"

#include "utils.h"

#include "vk_shader_compiler.h"

#include "task_material_sort_count_materials_per_chunk.h"

FCountMaterialsPerChunkTask::FCountMaterialsPerChunkTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, Context, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Material sort count materials per chunk pipeline";

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_COUNT_MATERIALS_PER_CHUNK_INDEX, MATERIAL_SORT_COUNT_MATERIALS_MATERIAL_INDICES_AOV_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_COUNT_MATERIALS_PER_CHUNK_INDEX, MATERIAL_SORT_COUNT_MATERIALS_MATERIAL_COUNT_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstantsCountMaterialsPerChunk)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    CreateSyncObjects();
}

void FCountMaterialsPerChunkTask::Init()
{
    Context->TimingManager->RegisterTiming(Name, NumberOfSimultaneousSubmits);

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    auto MaterialCountShader = FShader("../../../src/shaders/material_sort_count_materials_per_chunk.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);
    Pipeline = Context->CreateComputePipeline(MaterialCountShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, MATERIAL_SORT_COUNT_MATERIALS_PER_CHUNK_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FCountMaterialsPerChunkTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        UpdateDescriptorSet(MATERIAL_SORT_COUNT_MATERIALS_PER_CHUNK_INDEX, MATERIAL_SORT_COUNT_MATERIALS_MATERIAL_INDICES_AOV_BUFFER, i, Context->ResourceAllocator->GetBuffer("MaterialIndicesAOVBuffer"));
        UpdateDescriptorSet(MATERIAL_SORT_COUNT_MATERIALS_PER_CHUNK_INDEX, MATERIAL_SORT_COUNT_MATERIALS_MATERIAL_COUNT_BUFFER, i, Context->ResourceAllocator->GetBuffer("CountedMaterialsPerChunkBuffer"));
    }
};

void FCountMaterialsPerChunkTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = GetContext().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            Context->TimingManager->TimestampStart(Name, CommandBuffer, i);

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto ComputeDescriptorSet = Context->DescriptorSetManager->GetSet(Name, MATERIAL_SORT_COUNT_MATERIALS_PER_CHUNK_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Context->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &ComputeDescriptorSet, 0, nullptr);

            FPushConstantsCountMaterialsPerChunk PushConstantsCountMaterialsPerChunk = {Width * Height, CalculateGroupCount(Width * Height, BASIC_CHUNK_SIZE), CalculateMaxGroupCount(Width * Height, BASIC_CHUNK_SIZE)};
            vkCmdPushConstants(CommandBuffer, Context->DescriptorSetManager->GetPipelineLayout(Name), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstantsCountMaterialsPerChunk), &PushConstantsCountMaterialsPerChunk);

            vkCmdDispatch(CommandBuffer, PushConstantsCountMaterialsPerChunk.GroupSize, 1, 1);

            Context->TimingManager->TimestampEnd(Name, CommandBuffer, i);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], "V::MaterialSort_Count_Materials_Per_Chunk_Command_Buffer");
    }
};
