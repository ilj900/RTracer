#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "common_structures.h"
#include "utils.h"

#include "vk_shader_compiler.h"

#include "task_material_sort_sort_materials.h"

FSortMaterialsTask::FSortMaterialsTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
    Name = "Material sort sort materials pipeline";

    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX, MATERIAL_SORT_SORT_MATERIALS_MATERIALS_OFFSETS_PER_CHUNK_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX, MATERIAL_SORT_SORT_MATERIALS_MATERIAL_OFFSETS_PER_MATERIAL_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX, MATERIAL_SORT_SORT_MATERIALS_UNSORTED_MATERIALS_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX, MATERIAL_SORT_SORT_MATERIALS_SORTED_MATERIALS_INDEX_MAP_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    FBuffer SortedMaterialsIndexMapBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(sizeof(uint32_t) * Width * Height, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "SortedMaterialsIndexMapBuffer");
    RESOURCE_ALLOCATOR()->RegisterBuffer(SortedMaterialsIndexMapBuffer, "SortedMaterialsIndexMapBuffer");

    VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstantsCountMaterialsPerChunk)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

FSortMaterialsTask::~FSortMaterialsTask()
{
    RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer("SortedMaterialsIndexMapBuffer");
};

void FSortMaterialsTask::Init()
{
    TIMING_MANAGER()->RegisterTiming(Name, SubmitX, SubmitY);

    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    auto MaterialCountShader = FShader("../../../src/shaders/material_sort_sort_materials.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);
    Pipeline = VK_CONTEXT()->CreateComputePipeline(MaterialCountShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX, TotalSize);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FSortMaterialsTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < TotalSize; ++i)
    {
        UpdateDescriptorSet(MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX, MATERIAL_SORT_SORT_MATERIALS_MATERIALS_OFFSETS_PER_CHUNK_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("CountedMaterialsPerChunkBuffer"));
        UpdateDescriptorSet(MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX, MATERIAL_SORT_SORT_MATERIALS_MATERIAL_OFFSETS_PER_MATERIAL_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("MaterialsOffsetsPerMaterialBuffer"));
        UpdateDescriptorSet(MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX, MATERIAL_SORT_SORT_MATERIALS_UNSORTED_MATERIALS_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("MaterialIndicesAOVBuffer"));
        UpdateDescriptorSet(MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX, MATERIAL_SORT_SORT_MATERIALS_SORTED_MATERIALS_INDEX_MAP_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("SortedMaterialsIndexMapBuffer"));
    }
};

void FSortMaterialsTask::RecordCommands()
{
    CommandBuffers.resize(TotalSize);

    for (std::size_t i = 0; i < TotalSize; ++i)
    {
        CommandBuffers[i] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
			uint32_t X = i % SubmitX;
			uint32_t Y = i / SubmitX;

			if (X == 0)
			{
				TIMING_MANAGER()->TimestampReset(Name, CommandBuffer, Y);
			}
			TIMING_MANAGER()->TimestampStart(Name, CommandBuffer, X, Y);

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto ComputeDescriptorSet = VK_CONTEXT()->DescriptorSetManager->GetSet(Name, MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &ComputeDescriptorSet, 0, nullptr);

            uint32_t GroupSize = CalculateGroupCount(Width * Height, BASIC_CHUNK_SIZE);
            uint32_t MaxGroupSize = CalculateMaxGroupCount(Width * Height, BASIC_CHUNK_SIZE);
            FPushConstantsCountMaterialsPerChunk PushConstantsCountMaterialsPerChunk = {Width * Height, GroupSize, MaxGroupSize};
            vkCmdPushConstants(CommandBuffer, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
                               VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstantsCountMaterialsPerChunk), &PushConstantsCountMaterialsPerChunk);

            vkCmdDispatch(CommandBuffer, CalculateGroupCount(Width * Height, BASIC_CHUNK_SIZE), 1, 1);

            TIMING_MANAGER()->TimestampEnd(Name, CommandBuffer, X, Y);
        }, QueueFlagsBits);

        V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
    }
};
