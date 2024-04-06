#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "common_structures.h"

#include "systems/material_system.h"

#include "vk_shader_compiler.h"

#include "utils.h"

#include "task_miss.h"

FMissTask::FMissTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, Context, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Miss pipeline";

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_OUTPUT_IMAGE_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_RAYS_BUFFER_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_IBL_IMAGE_INDEX,
                                              {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_MATERIAL_INDEX_MAP,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_MATERIAL_INDEX_AOV_MAP,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_MATERIALS_OFFSETS,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstants)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    CreateSyncObjects();
}

FMissTask::~FMissTask()
{
    vkDestroySampler(LogicalDevice, IBLImageSampler, nullptr);
};

void FMissTask::Init()
{
    Context->TimingManager->RegisterTiming(Name, NumberOfSimultaneousSubmits);

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    auto ShadeShader = FShader("../../../src/shaders/miss.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

    Pipeline = Context->CreateComputePipeline(ShadeShader(), PipelineLayout);

    IBLImageSampler = Context->CreateTextureSampler(VK_SAMPLE_COUNT_1_BIT);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, COMPUTE_MISS_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FMissTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_OUTPUT_IMAGE_INDEX, i, Inputs[0]);
        UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_RAYS_BUFFER_INDEX, i, Context->ResourceAllocator->GetBuffer("InitialRaysBuffer"));
        UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_IBL_IMAGE_INDEX, i, Inputs[1], IBLImageSampler);
        UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_MATERIAL_INDEX_MAP, i, Context->ResourceAllocator->GetBuffer("SortedMaterialsIndexMapBuffer"));
        UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_MATERIAL_INDEX_AOV_MAP, i, Context->ResourceAllocator->GetBuffer("MaterialIndicesAOVBuffer"));
        UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_MATERIALS_OFFSETS, i, Context->ResourceAllocator->GetBuffer("MaterialsOffsetsPerMaterialBuffer"));
    }
};

void FMissTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = GetContext().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            Context->TimingManager->TimestampStart(Name, CommandBuffer, i);

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto RayTracingDescriptorSet = Context->DescriptorSetManager->GetSet(Name, COMPUTE_MISS_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Context->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &RayTracingDescriptorSet, 0, nullptr);

            auto DispatchBuffer = Context->ResourceAllocator->GetBuffer("TotalCountedMaterialsBuffer");

            uint32_t MaterialIndex = TOTAL_MATERIALS - 1;
            FPushConstants PushConstants = {Width, Height, 1.f / Width, 1.f / Height, Width * Height, MaterialIndex};
            vkCmdPushConstants(CommandBuffer, Context->DescriptorSetManager->GetPipelineLayout(Name),
                               VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstants), &PushConstants);

            vkCmdDispatchIndirect(CommandBuffer, DispatchBuffer.Buffer, MaterialIndex * 3 * sizeof(uint32_t));

            Context->TimingManager->TimestampEnd(Name, CommandBuffer, i);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], "V::Miss_Command_Buffer");
    }
};
