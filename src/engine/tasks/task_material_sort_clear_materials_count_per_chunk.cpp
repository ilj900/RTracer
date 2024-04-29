#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "utils.h"

#include "vk_shader_compiler.h"

#include "task_material_sort_clear_materials_count_per_chunk.h"

FClearMaterialsCountPerChunkTask::FClearMaterialsCountPerChunkTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
    Name = "Material sort clear materials count per chunk pipeline";

    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_CLEAR_MATERIALS_COUNT_PER_CHUNK_LAYOUT_INDEX, MATERIAL_SORT_CLEAR_MATERIALS_COUNT_PER_CHUNK_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    FBuffer CountedMaterialsPerChunkBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(sizeof(uint32_t) * TOTAL_MATERIALS * CalculateMaxGroupCount(Width * Height, BASIC_CHUNK_SIZE), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "CountedMaterialsPerChunkBuffer");
    RESOURCE_ALLOCATOR()->RegisterBuffer(CountedMaterialsPerChunkBuffer, "CountedMaterialsPerChunkBuffer");

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

FClearMaterialsCountPerChunkTask::~FClearMaterialsCountPerChunkTask()
{
    RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer("CountedMaterialsPerChunkBuffer");
};

void FClearMaterialsCountPerChunkTask::Init()
{
    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    auto MaterialCountShader = FShader("../../../src/shaders/material_sort_clear_material_count_per_chunk.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);
    Pipeline = VK_CONTEXT()->CreateComputePipeline(MaterialCountShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, MATERIAL_SORT_CLEAR_MATERIALS_COUNT_PER_CHUNK_LAYOUT_INDEX, TotalSize);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FClearMaterialsCountPerChunkTask::UpdateDescriptorSets()
{
    for (uint32_t i = 0; i < TotalSize; ++i)
    {
        UpdateDescriptorSet(MATERIAL_SORT_CLEAR_MATERIALS_COUNT_PER_CHUNK_LAYOUT_INDEX, MATERIAL_SORT_CLEAR_MATERIALS_COUNT_PER_CHUNK_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("CountedMaterialsPerChunkBuffer"));
    }
};

void FClearMaterialsCountPerChunkTask::RecordCommands()
{
    CommandBuffers.resize(TotalSize);

    for (uint32_t i = 0; i < TotalSize; ++i)
    {
        CommandBuffers[i] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
			ResetQueryPool(CommandBuffer, i);
			GPU_TIMER();

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto ComputeDescriptorSet = VK_CONTEXT()->DescriptorSetManager->GetSet(Name, MATERIAL_SORT_CLEAR_MATERIALS_COUNT_PER_CHUNK_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &ComputeDescriptorSet, 0, nullptr);

            uint32_t PushConstant = CalculateMaxGroupCount(Width * Height, BASIC_CHUNK_SIZE) * TOTAL_MATERIALS;
            vkCmdPushConstants(CommandBuffer, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &PushConstant);

            vkCmdDispatch(CommandBuffer, CalculateMaxGroupCount(PushConstant, BASIC_CHUNK_SIZE), 1, 1);
        }, QueueFlagsBits);

        V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
    }
};
