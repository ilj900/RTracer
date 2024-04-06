#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "common_structures.h"
#include "utils.h"

#include "vk_shader_compiler.h"

#include "task_material_sort_sort_materials.h"

FSortMaterialsTask::FSortMaterialsTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, Context, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Material sort sort materials pipeline";

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX, MATERIAL_SORT_SORT_MATERIALS_MATERIALS_OFFSETS_PER_CHUNK_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX, MATERIAL_SORT_SORT_MATERIALS_MATERIAL_OFFSETS_PER_MATERIAL_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX, MATERIAL_SORT_SORT_MATERIALS_UNSORTED_MATERIALS_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX, MATERIAL_SORT_SORT_MATERIALS_SORTED_MATERIALS_INDEX_MAP_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    FBuffer SortedMaterialsIndexMapBuffer = Context->ResourceAllocator->CreateBuffer(sizeof(uint32_t) * Width * Height, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "SortedMaterialsIndexMapBuffer");
    Context->ResourceAllocator->RegisterBuffer(SortedMaterialsIndexMapBuffer, "SortedMaterialsIndexMapBuffer");

    VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstantsCountMaterialsPerChunk)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    CreateSyncObjects();
}

FSortMaterialsTask::~FSortMaterialsTask()
{
    Context->ResourceAllocator->UnregisterAndDestroyBuffer("SortedMaterialsIndexMapBuffer");
};

void FSortMaterialsTask::Init()
{
    Context->TimingManager->RegisterTiming(Name, NumberOfSimultaneousSubmits);

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    auto MaterialCountShader = FShader("../../../src/shaders/material_sort_sort_materials.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);
    Pipeline = Context->CreateComputePipeline(MaterialCountShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FSortMaterialsTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        UpdateDescriptorSet(MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX, MATERIAL_SORT_SORT_MATERIALS_MATERIALS_OFFSETS_PER_CHUNK_BUFFER, i, Context->ResourceAllocator->GetBuffer("CountedMaterialsPerChunkBuffer"));
        UpdateDescriptorSet(MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX, MATERIAL_SORT_SORT_MATERIALS_MATERIAL_OFFSETS_PER_MATERIAL_BUFFER, i, Context->ResourceAllocator->GetBuffer("MaterialsOffsetsPerMaterialBuffer"));
        UpdateDescriptorSet(MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX, MATERIAL_SORT_SORT_MATERIALS_UNSORTED_MATERIALS_BUFFER, i, Context->ResourceAllocator->GetBuffer("MaterialIndicesAOVBuffer"));
        UpdateDescriptorSet(MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX, MATERIAL_SORT_SORT_MATERIALS_SORTED_MATERIALS_INDEX_MAP_BUFFER, i, Context->ResourceAllocator->GetBuffer("SortedMaterialsIndexMapBuffer"));
    }
};

void FSortMaterialsTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = GetContext().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            Context->TimingManager->TimestampStart(Name, CommandBuffer, i);

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto ComputeDescriptorSet = Context->DescriptorSetManager->GetSet(Name, MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Context->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &ComputeDescriptorSet, 0, nullptr);

            uint32_t GroupSize = CalculateGroupCount(Width * Height, BASIC_CHUNK_SIZE);
            uint32_t MaxGroupSize = CalculateMaxGroupCount(Width * Height, BASIC_CHUNK_SIZE);
            FPushConstantsCountMaterialsPerChunk PushConstantsCountMaterialsPerChunk = {Width * Height, GroupSize, MaxGroupSize};
            vkCmdPushConstants(CommandBuffer, Context->DescriptorSetManager->GetPipelineLayout(Name),
                               VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstantsCountMaterialsPerChunk), &PushConstantsCountMaterialsPerChunk);

            vkCmdDispatch(CommandBuffer, CalculateGroupCount(Width * Height, BASIC_CHUNK_SIZE), 1, 1);

            Context->TimingManager->TimestampEnd(Name, CommandBuffer, i);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], "V::MaterialSort_Sort_Materials_Command_Buffer");
    }
};
