#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

#include "material_component.h"
#include "material_system.h"
#include "renderable_system.h"
#include "transform_system.h"
#include "point_light_system.h"
#include "directional_light_system.h"
#include "spot_light_system.h"
#include "acceleration_structure_system.h"
#include "vk_shader_compiler.h"

#include "task_master_shader.h"
#include "texture_manager.h"

FMasterShader::FMasterShader(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
    Name = "Master shader pipeline";
	MaterialPipelines.resize(MATERIAL_SYSTEM()->MAX_MATERIALS, VK_NULL_HANDLE);
    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_TLAS_INDEX,
		{VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_TEXTURE_SAMPLER,
		{VK_DESCRIPTOR_TYPE_SAMPLER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_TEXTURE_ARRAY,
		{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR, MAX_TEXTURES});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_RAYS_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_HITS_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_RENDERABLES_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_TRANSFORMS_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_MATERIALS_OFFSETS,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_PIXEL_INDEX_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_DIRECTIONAL_LIGHTS_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_SPOT_LIGHTS_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_POINT_LIGHTS_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_POINT_LIGHTS_IMPORTANCE_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_IBL_IMPORTANCE_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_IBL_IMAGE_SAMPLER_INDEX,
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_IBL_WEIGHTS_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_UTILITY_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_RENDER_ITERATION_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_CUMULATIVE_MATERIAL_COLOR_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_NORMAL_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_THROUGHPUT_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_COLOR_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_SHADING_NORMAL_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_GEOMETRIC_NORMAL_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_UV_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_WORLD_SPACE_POSITION_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_OPACITY_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_DEPTH_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_ALBEDO_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_LUMINANCE_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_RENDERABLE_INDEX_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_PRIMITIVE_INDEX_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_MATERIAL_INDEX_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_DEBUG_LAYER_IMAGE_0_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_DEBUG_LAYER_IMAGE_1_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_DEBUG_LAYER_IMAGE_2_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_DEBUG_LAYER_IMAGE_3_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});

	VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, sizeof(FPushConstants)};
	DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

FMasterShader::~FMasterShader()
{
	for (auto& Buffer : SBTBuffers)
	{
		if (Buffer.Buffer != VK_NULL_HANDLE)
		{
			GetResourceAllocator()->DestroyBuffer(Buffer);
		}
	}

	vkDestroySampler(LogicalDevice, IBLSampler, nullptr);
	vkDestroySampler(LogicalDevice, MaterialTextureSampler, nullptr);
};

void FMasterShader::Init(FCompileDefinitions* CompileDefinitions)
{
    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

	auto RayClosestHitShader = FShader("../src/shaders/master_shader.rchit");
	auto RayMissShader = FShader("../src/shaders/master_shader.rmiss");
	RGenRegions.resize(MATERIAL_SYSTEM()->Entities.size());
	SBTBuffers.resize(RGenRegions.size());

	for (auto& Material : *MATERIAL_SYSTEM())
	{
		auto MaterialCode = MATERIAL_SYSTEM()->GenerateMaterialCode(Material);
		FCompileDefinitions CombinedCompileDefinitions(*CompileDefinitions);
		CombinedCompileDefinitions.Push("FDeviceMaterial GetMaterial(vec2 TextureCoords);", MaterialCode);
		auto RayGenerationShader = FShader("../src/shaders/master_shader.rgen", &CombinedCompileDefinitions);
		uint32_t MaterialIndex = COORDINATOR().GetIndex<ECS::COMPONENTS::FMaterialComponent>(Material);
		MaterialPipelines[MaterialIndex] = VK_CONTEXT()->CreateRayTracingPipeline(RayGenerationShader(), RayMissShader(), RayClosestHitShader(), PipelineLayout);
		SBTBuffers[MaterialIndex] = VK_CONTEXT()->GenerateSBT(MaterialPipelines[MaterialIndex], RMissRegion, RHitRegion, RGenRegions[MaterialIndex]);
	}

	MaterialTextureSampler = VK_CONTEXT()->CreateTextureSampler(VK_SAMPLE_COUNT_1_BIT, VK_FILTER_NEAREST);
	IBLSampler = VK_CONTEXT()->CreateTextureSampler(VK_SAMPLE_COUNT_1_BIT, VK_FILTER_NEAREST);
    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, MASTER_SHADER_LAYOUT_INDEX, TotalSize);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FMasterShader::UpdateDescriptorSets()
{
    for (size_t i = 0; i < TotalSize; ++i)
    {
        VK_CONTEXT()->DescriptorSetManager->UpdateDescriptorSetInfo(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_TLAS_INDEX, i, &ACCELERATION_STRUCTURE_SYSTEM()->TLAS.AccelerationStructure);
		VkDescriptorImageInfo MaterialTextureSamplerInfo{};
		MaterialTextureSamplerInfo.sampler = MaterialTextureSampler;
		VK_CONTEXT()->DescriptorSetManager->UpdateDescriptorSetInfo(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_TEXTURE_SAMPLER, i, &MaterialTextureSamplerInfo);
		auto TextureSampler = TEXTURE_MANAGER()->GetDescriptorImageInfos();
		VK_CONTEXT()->DescriptorSetManager->UpdateDescriptorSetInfo(Name, MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_TEXTURE_ARRAY, i, TextureSampler);
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_RAYS_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(INITIAL_RAYS_BUFFER));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_HITS_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(HITS_BUFFER));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_RENDERABLES_BUFFER_INDEX, i, RENDERABLE_SYSTEM()->DeviceBuffer);
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_TRANSFORMS_BUFFER_INDEX, i, TRANSFORM_SYSTEM()->DeviceBuffer);
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_MATERIALS_OFFSETS, i, RESOURCE_ALLOCATOR()->GetBuffer(MATERIALS_OFFSETS_PER_MATERIAL_BUFFER));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_PIXEL_INDEX_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer(PIXEL_INDEX_BUFFER));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_DIRECTIONAL_LIGHTS_BUFFER_INDEX, i, DIRECTIONAL_LIGHT_SYSTEM()->DeviceBuffer);
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_SPOT_LIGHTS_BUFFER_INDEX, i, SPOT_LIGHT_SYSTEM()->DeviceBuffer);
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_POINT_LIGHTS_BUFFER_INDEX, i, POINT_LIGHT_SYSTEM()->DeviceBuffer);
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_POINT_LIGHTS_IMPORTANCE_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(POINT_LIGHTS_IMPORTANCE_BUFFER));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_IBL_IMPORTANCE_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer("IBLImportanceBuffer"));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_IBL_IMAGE_SAMPLER_INDEX, i, TEXTURE_MANAGER()->GetTexture("IBL Image"), IBLSampler);
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_IBL_WEIGHTS_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer("IBLPDFBuffer"));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_UTILITY_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(UTILITY_INFO_BUFFER));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_RENDER_ITERATION_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(RENDER_ITERATION_BUFFER));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_CUMULATIVE_MATERIAL_COLOR_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(CUMULATIVE_MATERIAL_COLOR_BUFFER));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_NORMAL_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer(NORMAL_AOV_BUFFER));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_THROUGHPUT_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer(THROUGHPUT_BUFFER));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_COLOR_AOV_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("ColorImage"));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_SHADING_NORMAL_AOV_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("ShadingNormalAOVImage"));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_GEOMETRIC_NORMAL_AOV_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("GeometricNormalAOVImage"));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_UV_AOV_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("UVAOVImage"));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_WORLD_SPACE_POSITION_AOV_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("WorldSpacePositionAOVImage"));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_OPACITY_AOV_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("OpacityAOVImage"));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_DEPTH_AOV_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("DepthAOVImage"));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_ALBEDO_AOV_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("AlbedoAOVImage"));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_LUMINANCE_AOV_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("LuminanceAOVImage"));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_RENDERABLE_INDEX_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("RenderableIDAOVImage"));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_PRIMITIVE_INDEX_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("PrimitiveIDAOVImage"));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_MATERIAL_INDEX_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("MaterialIDAOVImage"));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_DEBUG_LAYER_IMAGE_0_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("DebugLayerImage0"));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_DEBUG_LAYER_IMAGE_1_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("DebugLayerImage1"));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_DEBUG_LAYER_IMAGE_2_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("DebugLayerImage2"));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX, MASTER_SHADER_DEBUG_LAYER_IMAGE_3_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("DebugLayerImage3"));
    }
};

void FMasterShader::RecordCommands()
{
    CommandBuffers.resize(TotalSize);
	auto TotalCountedMaterialsBuffer = VK_CONTEXT()->GetBufferDeviceAddressInfo(RESOURCE_ALLOCATOR()->GetBuffer(TOTAL_COUNTED_MATERIALS_BUFFER));

    for (uint32_t i = 0; i < TotalSize; ++i)
    {
        CommandBuffers[i] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
			ResetQueryPool(CommandBuffer, i);
			GPU_TIMER();

			for (auto& Material : *MATERIAL_SYSTEM())
			{
				uint32_t MaterialIndex = COORDINATOR().GetIndex<ECS::COMPONENTS::FMaterialComponent>(Material);

				vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, MaterialPipelines[MaterialIndex]);
				auto RayTracingDescriptorSet = VK_CONTEXT()->DescriptorSetManager->GetSet(Name, MASTER_SHADER_LAYOUT_INDEX, i);
				vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
					0, 1, &RayTracingDescriptorSet, 0, nullptr);

				FPushConstants PushConstants = { Width, Height, 1.f / float(Width), 1.f / float(Height), Width * Height, MaterialIndex, i % SubmitX };
				vkCmdPushConstants(CommandBuffer, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name), VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, sizeof(FPushConstants), &PushConstants);

				V::vkCmdTraceRaysIndirectKHR(CommandBuffer, &RGenRegions[MaterialIndex], &RMissRegion, &RHitRegion, &RCallRegion, TotalCountedMaterialsBuffer + (MaterialIndex * 3 * sizeof(uint32_t)));
			}
        }, QueueFlagsBits);

        V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
    }
};
