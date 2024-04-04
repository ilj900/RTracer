#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "common_structures.h"

#include "components/material_component.h"
#include "systems/material_system.h"
#include "systems/renderable_system.h"
#include "systems/transform_system.h"
#include "systems/light_system.h"

#include "vk_shader_compiler.h"

#include "texture_manager.h"

#include "utils.h"

#include "task_shade.h"

FShadeTask::FShadeTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, Context, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Shade pipeline";

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_OUTPUT_IMAGE_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_RENDERABLE_BUFFER_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_HITS_BUFFER_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_TRANSFORM_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_RAYS_BUFFER_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_TEXTURE_SAMPLER,
                                              {VK_DESCRIPTOR_TYPE_SAMPLER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_TEXTURE_ARRAY,
                                              {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT, MAX_TEXTURES});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_LIGHTS_BUFFER_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_MATERIAL_INDEX_MAP,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_MATERIAL_INDEX_AOV_MAP,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_MATERIALS_OFFSETS,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstants)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    CreateSyncObjects();
}

FShadeTask::~FShadeTask()
{
    vkDestroySampler(LogicalDevice, MaterialTextureSampler, nullptr);

    FreeSyncObjects();
}

void FShadeTask::Init()
{
    Context->TimingManager->RegisterTiming(Name, NumberOfSimultaneousSubmits);

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

    for (auto& Material : *MATERIAL_SYSTEM())
    {
        auto MaterialCode = MATERIAL_SYSTEM()->GenerateMaterialCode(Material);
        FCompileDefinitions CompileDefinitions;
        CompileDefinitions.Push("FDeviceMaterial GetMaterial();", MaterialCode);
        auto ShadeShader = FShader("../../../src/shaders/shade.comp", &CompileDefinitions);
        uint32_t MaterialIndex = ECS::GetCoordinator().GetIndex<ECS::COMPONENTS::FMaterialComponent>(Material);
        MaterialIndexToPipelineMap[MaterialIndex] = Context->CreateComputePipeline(ShadeShader(), PipelineLayout);
    }

    MaterialTextureSampler = Context->CreateTextureSampler(VK_SAMPLE_COUNT_1_BIT);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, COMPUTE_SHADE_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FShadeTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_OUTPUT_IMAGE_INDEX, i, Inputs[0]);
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_RENDERABLE_BUFFER_INDEX, i, RENDERABLE_SYSTEM()->DeviceBuffer);
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_HITS_BUFFER_INDEX, i, Context->ResourceAllocator->GetBuffer("HitsBuffer"));
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_TRANSFORM_INDEX, i, TRANSFORM_SYSTEM()->DeviceBuffer);
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_RAYS_BUFFER_INDEX, i, Context->ResourceAllocator->GetBuffer("InitialRaysBuffer"));

        VkDescriptorImageInfo MaterialTextureSamplerInfo{};
        MaterialTextureSamplerInfo.sampler = MaterialTextureSampler;
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_TEXTURE_SAMPLER, i, &MaterialTextureSamplerInfo);

        auto TextureSampler = GetTextureManager()->GetDescriptorImageInfos();
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_TEXTURE_ARRAY, i, TextureSampler);

        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_LIGHTS_BUFFER_INDEX, i, LIGHT_SYSTEM()->DeviceBuffer);
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_MATERIAL_INDEX_MAP, i, Context->ResourceAllocator->GetBuffer("SortedMaterialsIndexMapBuffer"));
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_MATERIAL_INDEX_AOV_MAP, i, Context->ResourceAllocator->GetBuffer("MaterialIndicesAOVBuffer"));
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_MATERIALS_OFFSETS, i, Context->ResourceAllocator->GetBuffer("MaterialsOffsetsPerMaterialBuffer"));
    }
};

void FShadeTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = GetContext().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            Context->TimingManager->TimestampStart(Name, CommandBuffer, i);

            auto DispatchBuffer = Context->ResourceAllocator->GetBuffer("TotalCountedMaterialsBuffer");

            for (auto& Material : *MATERIAL_SYSTEM())
            {
                uint32_t MaterialIndex = ECS::GetCoordinator().GetIndex<ECS::COMPONENTS::FMaterialComponent>(Material);

                vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, MaterialIndexToPipelineMap[MaterialIndex]);
                auto RayTracingDescriptorSet = Context->DescriptorSetManager->GetSet(Name, COMPUTE_SHADE_LAYOUT_INDEX, i);
                vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Context->DescriptorSetManager->GetPipelineLayout(Name),
                                        0, 1, &RayTracingDescriptorSet, 0, nullptr);

                FPushConstants PushConstants = {Width, Height, 1.f / Width, 1.f / Height, Width * Height, MaterialIndex};
                vkCmdPushConstants(CommandBuffer, Context->DescriptorSetManager->GetPipelineLayout(Name), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstants), &PushConstants);

                vkCmdDispatchIndirect(CommandBuffer, DispatchBuffer.Buffer, MaterialIndex * 3 * sizeof(uint32_t));
            }

            Context->TimingManager->TimestampEnd(Name, CommandBuffer, i);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], "V::Shade_Command_Buffer");
    }
};

void FShadeTask::Cleanup()
{
    Inputs.clear();
    Outputs.clear();

    for (auto& CommandBuffer : CommandBuffers)
    {
        Context->CommandBufferManager->FreeCommandBuffer(CommandBuffer);
    }

    Context->DescriptorSetManager->DestroyPipelineLayout(Name);
    for (auto& Entry : MaterialIndexToPipelineMap)
    {
        vkDestroyPipeline(LogicalDevice, Entry.second, nullptr);
    }

    MaterialIndexToPipelineMap.clear();

    Context->DescriptorSetManager->Reset(Name);
};

VkSemaphore FShadeTask::Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex)
{
    return FExecutableTask::Submit(Queue, WaitSemaphore, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, WaitFence, SignalFence, IterationIndex);
};
