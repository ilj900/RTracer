#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "common_structures.h"

#include "utils.h"

#include "vk_shader_compiler.h"

#include "task_material_sort_count_materials_per_chunk.h"

FCountMaterialsPerChunkTask::FCountMaterialsPerChunkTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
    Name = "Material sort count materials per chunk pipeline";

    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_COUNT_MATERIALS_PER_CHUNK_INDEX, MATERIAL_SORT_COUNT_MATERIALS_MATERIAL_INDICES_AOV_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_COUNT_MATERIALS_PER_CHUNK_INDEX, MATERIAL_SORT_COUNT_MATERIALS_MATERIAL_COUNT_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstantsCountMaterialsPerChunk)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

void FCountMaterialsPerChunkTask::Init()
{
    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    auto MaterialCountShader = FShader("../../../src/shaders/material_sort_count_materials_per_chunk.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);
    Pipeline = VK_CONTEXT()->CreateComputePipeline(MaterialCountShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, MATERIAL_SORT_COUNT_MATERIALS_PER_CHUNK_INDEX, TotalSize);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FCountMaterialsPerChunkTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < TotalSize; ++i)
    {
        UpdateDescriptorSet(MATERIAL_SORT_COUNT_MATERIALS_PER_CHUNK_INDEX, MATERIAL_SORT_COUNT_MATERIALS_MATERIAL_INDICES_AOV_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("MaterialIndicesAOVBuffer"));
        UpdateDescriptorSet(MATERIAL_SORT_COUNT_MATERIALS_PER_CHUNK_INDEX, MATERIAL_SORT_COUNT_MATERIALS_MATERIAL_COUNT_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer(COUNTED_MATERIALS_PER_CHUNK_BUFFER));
    }
};

void FCountMaterialsPerChunkTask::RecordCommands()
{
    CommandBuffers.resize(TotalSize);

    for (std::size_t i = 0; i < TotalSize; ++i)
    {
        CommandBuffers[i] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
			ResetQueryPool(CommandBuffer, i);
			GPU_TIMER();

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto ComputeDescriptorSet = VK_CONTEXT()->DescriptorSetManager->GetSet(Name, MATERIAL_SORT_COUNT_MATERIALS_PER_CHUNK_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &ComputeDescriptorSet, 0, nullptr);

            FPushConstantsCountMaterialsPerChunk PushConstantsCountMaterialsPerChunk = {Width * Height, CalculateGroupCount(Width * Height, BASIC_CHUNK_SIZE), CalculateMaxGroupCount(Width * Height, BASIC_CHUNK_SIZE)};
            vkCmdPushConstants(CommandBuffer, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstantsCountMaterialsPerChunk), &PushConstantsCountMaterialsPerChunk);

            vkCmdDispatch(CommandBuffer, PushConstantsCountMaterialsPerChunk.GroupSize, 1, 1);
        }, QueueFlagsBits);

        V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
    }
};
