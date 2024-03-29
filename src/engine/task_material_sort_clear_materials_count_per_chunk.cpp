#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "utils.h"

#include "vk_shader_compiler.h"

#include "task_material_sort_clear_materials_count_per_chunk.h"

FClearMaterialsCountPerChunkTask::FClearMaterialsCountPerChunkTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, Context, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Material sort clear materials count per chunk pipeline";

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_CLEAR_MATERIALS_COUNT_PER_CHUNK_LAYOUT_INDEX, MATERIAL_SORT_CLEAR_MATERIALS_COUNT_PER_CHUNK_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    DescriptorSetManager->CreateDescriptorSetLayout({}, Name);

    FBuffer CountedMaterialsPerChunkBuffer = Context->ResourceAllocator->CreateBuffer(sizeof(uint32_t) * TOTAL_MATERIALS * CalculateGroupCount(Width * Height, BASIC_CHUNK_SIZE), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "CountedMaterialsPerChunkBuffer");
    Context->ResourceAllocator->RegisterBuffer(CountedMaterialsPerChunkBuffer, "CountedMaterialsPerChunkBuffer");

    CreateSyncObjects();
}

FClearMaterialsCountPerChunkTask::~FClearMaterialsCountPerChunkTask()
{
    FreeSyncObjects();
    Context->ResourceAllocator->UnregisterAndDestroyBuffer("CountedMaterialsPerChunkBuffer");
}

void FClearMaterialsCountPerChunkTask::Init()
{
    Context->TimingManager->RegisterTiming(Name, NumberOfSimultaneousSubmits);

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    auto MaterialCountShader = FShader("../../../src/shaders/material_sort_clear_material_count_per_chunk.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);
    Pipeline = Context->CreateComputePipeline(MaterialCountShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, MATERIAL_SORT_CLEAR_MATERIALS_COUNT_PER_CHUNK_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FClearMaterialsCountPerChunkTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        UpdateDescriptorSet(MATERIAL_SORT_CLEAR_MATERIALS_COUNT_PER_CHUNK_LAYOUT_INDEX, MATERIAL_SORT_CLEAR_MATERIALS_COUNT_PER_CHUNK_BUFFER, i, Context->ResourceAllocator->GetBuffer("CountedMaterialsPerChunkBuffer"));
    }
};

void FClearMaterialsCountPerChunkTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = GetContext().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            Context->TimingManager->TimestampStart(Name, CommandBuffer, i);

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto ComputeDescriptorSet = Context->DescriptorSetManager->GetSet(Name, MATERIAL_SORT_CLEAR_MATERIALS_COUNT_PER_CHUNK_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Context->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &ComputeDescriptorSet, 0, nullptr);

            vkCmdDispatch(CommandBuffer, 1, 1, 1);

            Context->TimingManager->TimestampEnd(Name, CommandBuffer, i);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], "V::MaterialSort_Clear_Materials_Count_Per_Chunk_Command_Buffer");
    }
};

void FClearMaterialsCountPerChunkTask::Cleanup()
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

VkSemaphore FClearMaterialsCountPerChunkTask::Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex)
{
    return FExecutableTask::Submit(Queue, WaitSemaphore, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, WaitFence, SignalFence, IterationIndex);
};
