#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "utils.h"

#include "vk_shader_compiler.h"

#include "task_material_sort_clear_total_materials_count.h"

FClearTotalMaterialsCountTask::FClearTotalMaterialsCountTask(uint32_t WidthIn, uint32_t HeightIn, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Material sort clear total materials count pipeline";

    auto& DescriptorSetManager = VK_CONTEXT().DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_CLEAR_TOTAL_MATERIALS_COUNT_LAYOUT_INDEX, MATERIAL_SORT_CLEAR_TOTAL_MATERIALS_COUNT_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    DescriptorSetManager->CreateDescriptorSetLayout({}, Name);

    FBuffer TotalCountedMaterialsBuffer = VK_CONTEXT().ResourceAllocator->CreateBuffer(sizeof(uint32_t) * TOTAL_MATERIALS * 3, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "TotalCountedMaterialsBuffer");
    VK_CONTEXT().ResourceAllocator->RegisterBuffer(TotalCountedMaterialsBuffer, "TotalCountedMaterialsBuffer");

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
}

FClearTotalMaterialsCountTask::~FClearTotalMaterialsCountTask()
{
    VK_CONTEXT().ResourceAllocator->UnregisterAndDestroyBuffer("TotalCountedMaterialsBuffer");
};

void FClearTotalMaterialsCountTask::Init()
{
    VK_CONTEXT().TimingManager->RegisterTiming(Name, NumberOfSimultaneousSubmits);

    auto& DescriptorSetManager = VK_CONTEXT().DescriptorSetManager;

    auto MaterialCountShader = FShader("../../../src/shaders/material_sort_clear_total_material_count.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);
    Pipeline = VK_CONTEXT().CreateComputePipeline(MaterialCountShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, MATERIAL_SORT_CLEAR_TOTAL_MATERIALS_COUNT_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FClearTotalMaterialsCountTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        UpdateDescriptorSet(MATERIAL_SORT_CLEAR_TOTAL_MATERIALS_COUNT_LAYOUT_INDEX, MATERIAL_SORT_CLEAR_TOTAL_MATERIALS_COUNT_BUFFER, i, VK_CONTEXT().ResourceAllocator->GetBuffer("TotalCountedMaterialsBuffer"));
    }
};

void FClearTotalMaterialsCountTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = VK_CONTEXT().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            VK_CONTEXT().TimingManager->TimestampStart(Name, CommandBuffer, i);

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto ComputeDescriptorSet = VK_CONTEXT().DescriptorSetManager->GetSet(Name, MATERIAL_SORT_CLEAR_TOTAL_MATERIALS_COUNT_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, VK_CONTEXT().DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &ComputeDescriptorSet, 0, nullptr);

            vkCmdDispatch(CommandBuffer, 1, 1, 1);

            VK_CONTEXT().TimingManager->TimestampEnd(Name, CommandBuffer, i);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], Name);
    }
};
