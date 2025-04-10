#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "utils.h"

#include "vk_shader_compiler.h"

#include "task_material_sort_clear_total_materials_count.h"

FClearTotalMaterialsCountTask::FClearTotalMaterialsCountTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
    Name = "Material sort clear total materials count pipeline";

    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_CLEAR_TOTAL_MATERIALS_COUNT_LAYOUT_INDEX, MATERIAL_SORT_CLEAR_TOTAL_MATERIALS_COUNT_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    DescriptorSetManager->CreateDescriptorSetLayout({}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

FClearTotalMaterialsCountTask::~FClearTotalMaterialsCountTask()
{
};

void FClearTotalMaterialsCountTask::Init(FCompileDefinitions* CompileDefinitions)
{
    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    auto MaterialCountShader = FShader("../src/shaders/material_sort_clear_total_material_count.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);
    Pipeline = VK_CONTEXT()->CreateComputePipeline(MaterialCountShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, MATERIAL_SORT_CLEAR_TOTAL_MATERIALS_COUNT_LAYOUT_INDEX, TotalSize);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FClearTotalMaterialsCountTask::UpdateDescriptorSets()
{
    for (uint32_t i = 0; i < TotalSize; ++i)
    {
        UpdateDescriptorSet(MATERIAL_SORT_CLEAR_TOTAL_MATERIALS_COUNT_LAYOUT_INDEX, MATERIAL_SORT_CLEAR_TOTAL_MATERIALS_COUNT_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer(TOTAL_COUNTED_MATERIALS_BUFFER));
    }
};

void FClearTotalMaterialsCountTask::RecordCommands()
{
    CommandBuffers.resize(TotalSize);

    for (uint32_t i = 0; i < TotalSize; ++i)
    {
        CommandBuffers[i] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
			ResetQueryPool(CommandBuffer, i);
			GPU_TIMER();

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto ComputeDescriptorSet = VK_CONTEXT()->DescriptorSetManager->GetSet(Name, MATERIAL_SORT_CLEAR_TOTAL_MATERIALS_COUNT_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &ComputeDescriptorSet, 0, nullptr);

            vkCmdDispatch(CommandBuffer, 1, 1, 1);
        }, QueueFlagsBits);

        V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
    }
};
