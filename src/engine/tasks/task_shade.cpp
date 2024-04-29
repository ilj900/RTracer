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

FShadeTask::FShadeTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
    Name = "Shade pipeline";

    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

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

    auto RTColorImage = TEXTURE_MANAGER()->CreateStorageImage(Width, Height, "RayTracingColorImage");
    TEXTURE_MANAGER()->RegisterFramebuffer(RTColorImage, "RayTracingColorImage");
    RTColorImage->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstants)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

FShadeTask::~FShadeTask()
{
    for (auto& Entry : MaterialIndexToPipelineMap)
    {
        vkDestroyPipeline(LogicalDevice, Entry.second, nullptr);
    }

    MaterialIndexToPipelineMap.clear();

    vkDestroySampler(LogicalDevice, MaterialTextureSampler, nullptr);
}

void FShadeTask::Init()
{
    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

    for (auto& Material : *MATERIAL_SYSTEM())
    {
        auto MaterialCode = MATERIAL_SYSTEM()->GenerateMaterialCode(Material);
        FCompileDefinitions CompileDefinitions;
        CompileDefinitions.Push("FDeviceMaterial GetMaterial(vec2 TextureCoords);", MaterialCode);
        auto ShadeShader = FShader("../../../src/shaders/shade.comp", &CompileDefinitions);
        uint32_t MaterialIndex = COORDINATOR().GetIndex<ECS::COMPONENTS::FMaterialComponent>(Material);
        MaterialIndexToPipelineMap[MaterialIndex] = VK_CONTEXT()->CreateComputePipeline(ShadeShader(), PipelineLayout);
    }

    MaterialTextureSampler = VK_CONTEXT()->CreateTextureSampler(VK_SAMPLE_COUNT_1_BIT);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, COMPUTE_SHADE_LAYOUT_INDEX, TotalSize);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FShadeTask::UpdateDescriptorSets()
{
    for (uint32_t i = 0; i < TotalSize; ++i)
    {
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_OUTPUT_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("RayTracingColorImage"));
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_RENDERABLE_BUFFER_INDEX, i, RENDERABLE_SYSTEM()->DeviceBuffer);
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_HITS_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer("HitsBuffer"));
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_TRANSFORM_INDEX, i, TRANSFORM_SYSTEM()->DeviceBuffer);
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_RAYS_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer("InitialRaysBuffer"));

        VkDescriptorImageInfo MaterialTextureSamplerInfo{};
        MaterialTextureSamplerInfo.sampler = MaterialTextureSampler;
        VK_CONTEXT()->DescriptorSetManager->UpdateDescriptorSetInfo(Name, COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_TEXTURE_SAMPLER, i, &MaterialTextureSamplerInfo);

        auto TextureSampler = TEXTURE_MANAGER()->GetDescriptorImageInfos();
        VK_CONTEXT()->DescriptorSetManager->UpdateDescriptorSetInfo(Name, COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_TEXTURE_ARRAY, i, TextureSampler);

        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_LIGHTS_BUFFER_INDEX, i, LIGHT_SYSTEM()->DeviceBuffer);
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_MATERIAL_INDEX_MAP, i, RESOURCE_ALLOCATOR()->GetBuffer("PixelIndexBuffer"));
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_MATERIAL_INDEX_AOV_MAP, i, RESOURCE_ALLOCATOR()->GetBuffer("MaterialIndicesAOVBuffer"));
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_MATERIALS_OFFSETS, i, RESOURCE_ALLOCATOR()->GetBuffer("MaterialsOffsetsPerMaterialBuffer"));
    }
};

void FShadeTask::RecordCommands()
{
    CommandBuffers.resize(TotalSize);
	auto DispatchBuffer = RESOURCE_ALLOCATOR()->GetBuffer("TotalCountedMaterialsBuffer");

	for (uint32_t i = 0; i < TotalSize; ++i)
	{
		CommandBuffers[i] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
		{
			ResetQueryPool(CommandBuffer, i);
			GPU_TIMER();

			for (auto& Material : *MATERIAL_SYSTEM())
			{
				uint32_t MaterialIndex = COORDINATOR().GetIndex<ECS::COMPONENTS::FMaterialComponent>(Material);

				vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, MaterialIndexToPipelineMap[MaterialIndex]);
				auto RayTracingDescriptorSet = VK_CONTEXT()->DescriptorSetManager->GetSet(Name, COMPUTE_SHADE_LAYOUT_INDEX, i);
				vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
					0, 1, &RayTracingDescriptorSet, 0, nullptr);

				FPushConstants PushConstants = { Width, Height, 1.f / float(Width), 1.f / float(Height), Width * Height, MaterialIndex };
				vkCmdPushConstants(CommandBuffer, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstants), &PushConstants);

				vkCmdDispatchIndirect(CommandBuffer, DispatchBuffer.Buffer, MaterialIndex * 3 * sizeof(uint32_t));
			}
		}, QueueFlagsBits);

		V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
    }
};
