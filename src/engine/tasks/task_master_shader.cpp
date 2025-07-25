#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

#include "area_light_system.h"
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

    DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_TLAS_INDEX,
		{VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_TEXTURE_SAMPLER,
		{VK_DESCRIPTOR_TYPE_SAMPLER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_FLOAT_TEXTURE_ARRAY,
		{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR, MAX_TEXTURES});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_UINT_TEXTURE_ARRAY,
		{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR, MAX_TEXTURES});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_INT_TEXTURE_ARRAY,
		{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR, MAX_TEXTURES});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_IBL_IMPORTANCE_BUFFER_INDEX,
{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_IBL_IMAGE_SAMPLER_INDEX,
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_IBL_WEIGHTS_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_RENDER_ITERATION_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_RAYS_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_HITS_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_MATERIALS_OFFSETS,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_PIXEL_INDEX_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_CUMULATIVE_MATERIAL_COLOR_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_NORMAL_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_THROUGHPUT_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_UTILITY_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_COLOR_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_AOV_RGBA32F_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});


	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, MASTER_SHADER_DIRECTIONAL_LIGHTS_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, MASTER_SHADER_DIRECTIONAL_LIGHTS_IMPORTANCE_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, MASTER_SHADER_SPOT_LIGHTS_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, MASTER_SHADER_SPOT_LIGHTS_IMPORTANCE_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, MASTER_SHADER_POINT_LIGHTS_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, MASTER_SHADER_POINT_LIGHTS_IMPORTANCE_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, MASTER_SHADER_AREA_LIGHTS_BUFFER_INDEX,
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, MASTER_SHADER_AREA_LIGHTS_IMPORTANCE_BUFFER_INDEX,
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, MASTER_SHADER_RENDERABLES_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, MASTER_SHADER_TRANSFORMS_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});

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

    const auto EmissiveMaterials = AREA_LIGHT_SYSTEM()->GetEmissiveMaterials();
    auto EmissiveMaterialCode = MATERIAL_SYSTEM()->GenerateEmissiveMaterialsCode(EmissiveMaterials);
    FCompileDefinitions MasterShaderCompileDefinitions(*CompileDefinitions);
    MasterShaderCompileDefinitions.Push("FDeviceMaterial GetEmissiveMaterial(vec2 TextureCoords, uint MaterialIndex);", EmissiveMaterialCode);

	for (auto& Material : *MATERIAL_SYSTEM())
	{
		auto MaterialCode = MATERIAL_SYSTEM()->GenerateMaterialCode(Material);
		FCompileDefinitions CombinedCompileDefinitions(MasterShaderCompileDefinitions);
		CombinedCompileDefinitions.Push("FDeviceMaterial GetMaterial(vec2 TextureCoords);", MaterialCode);
		auto RayGenerationShader = FShader("../src/shaders/master_shader.rgen", &CombinedCompileDefinitions);
		uint32_t MaterialIndex = COORDINATOR().GetIndex<ECS::COMPONENTS::FMaterialComponent>(Material);
		MaterialPipelines[MaterialIndex] = VK_CONTEXT()->CreateRayTracingPipeline(RayGenerationShader(), RayMissShader(), RayClosestHitShader(), PipelineLayout);
		SBTBuffers[MaterialIndex] = VK_CONTEXT()->GenerateSBT(MaterialPipelines[MaterialIndex], RMissRegion, RHitRegion, RGenRegions[MaterialIndex]);
	}

	MaterialTextureSampler = VK_CONTEXT()->CreateTextureSampler(VK_SAMPLE_COUNT_1_BIT, VK_FILTER_NEAREST);
	IBLSampler = VK_CONTEXT()->CreateTextureSampler(VK_SAMPLE_COUNT_1_BIT, VK_FILTER_LINEAR);
    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, 1);
	DescriptorSetManager->ReserveDescriptorSet(Name, MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, SubmitY);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FMasterShader::UpdateDescriptorSets()
{
	VK_CONTEXT()->DescriptorSetManager->UpdateDescriptorSetInfo(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_TLAS_INDEX, 0, &ACCELERATION_STRUCTURE_SYSTEM()->TLAS.AccelerationStructure);
	VkDescriptorImageInfo MaterialTextureSamplerInfo{};
	MaterialTextureSamplerInfo.sampler = MaterialTextureSampler;
	VK_CONTEXT()->DescriptorSetManager->UpdateDescriptorSetInfo(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_TEXTURE_SAMPLER, 0, &MaterialTextureSamplerInfo);
	auto DescriptorImageInfosFloat = TEXTURE_MANAGER()->GetDescriptorImageInfosFloat();
	VK_CONTEXT()->DescriptorSetManager->UpdateDescriptorSetInfo(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_FLOAT_TEXTURE_ARRAY, 0, DescriptorImageInfosFloat);
    auto DescriptorImageInfosUint = TEXTURE_MANAGER()->GetDescriptorImageInfosUint();
    VK_CONTEXT()->DescriptorSetManager->UpdateDescriptorSetInfo(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_UINT_TEXTURE_ARRAY, 0, DescriptorImageInfosUint);
    auto DescriptorImageInfosInt = TEXTURE_MANAGER()->GetDescriptorImageInfosInt();
    VK_CONTEXT()->DescriptorSetManager->UpdateDescriptorSetInfo(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_INT_TEXTURE_ARRAY, 0, DescriptorImageInfosInt);
    UpdateDescriptorSet(MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_IBL_IMPORTANCE_BUFFER_INDEX, 0, RESOURCE_ALLOCATOR()->GetBuffer("IBLImportanceBuffer"));
    UpdateDescriptorSet(MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_IBL_IMAGE_SAMPLER_INDEX, 0, TEXTURE_MANAGER()->GetIBLImage(), IBLSampler);
    UpdateDescriptorSet(MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_IBL_WEIGHTS_BUFFER_INDEX, 0, RESOURCE_ALLOCATOR()->GetBuffer("IBLPDFBuffer"));
    UpdateDescriptorSet(MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_RENDER_ITERATION_BUFFER_INDEX, 0, RESOURCE_ALLOCATOR()->GetBuffer(RENDER_ITERATION_BUFFER));
	UpdateDescriptorSet(MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_RAYS_BUFFER_INDEX, 0, RESOURCE_ALLOCATOR()->GetBuffer(INITIAL_RAYS_BUFFER));
	UpdateDescriptorSet(MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_HITS_BUFFER_INDEX, 0, RESOURCE_ALLOCATOR()->GetBuffer(HITS_BUFFER));
	UpdateDescriptorSet(MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_MATERIALS_OFFSETS, 0, RESOURCE_ALLOCATOR()->GetBuffer(MATERIALS_OFFSETS_PER_MATERIAL_BUFFER));
	UpdateDescriptorSet(MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_PIXEL_INDEX_BUFFER, 0, RESOURCE_ALLOCATOR()->GetBuffer(PIXEL_INDEX_BUFFER));
	UpdateDescriptorSet(MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_CUMULATIVE_MATERIAL_COLOR_BUFFER_INDEX, 0, RESOURCE_ALLOCATOR()->GetBuffer(CUMULATIVE_MATERIAL_COLOR_BUFFER));
	UpdateDescriptorSet(MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_NORMAL_BUFFER, 0, RESOURCE_ALLOCATOR()->GetBuffer(NORMAL_AOV_BUFFER));
	UpdateDescriptorSet(MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_THROUGHPUT_BUFFER, 0, RESOURCE_ALLOCATOR()->GetBuffer(THROUGHPUT_BUFFER));
	UpdateDescriptorSet(MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_UTILITY_BUFFER_INDEX, 0, RESOURCE_ALLOCATOR()->GetBuffer(UTILITY_INFO_BUFFER));
	UpdateDescriptorSet(MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_COLOR_AOV_IMAGE_INDEX, 0, TEXTURE_MANAGER()->GetFramebufferImage("ColorImage"));
	UpdateDescriptorSet(MASTER_SHADER_LAYOUT_STATIC_INDEX, MASTER_SHADER_AOV_RGBA32F_IMAGE_INDEX, 0, TEXTURE_MANAGER()->GetFramebufferImage("AOVImage"));

    for (int i = 0; i < SubmitY; ++i)
    {
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, MASTER_SHADER_DIRECTIONAL_LIGHTS_BUFFER_INDEX, i, DIRECTIONAL_LIGHT_SYSTEM()->DeviceBuffer);
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, MASTER_SHADER_DIRECTIONAL_LIGHTS_IMPORTANCE_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(DIRECTIONAL_LIGHTS_IMPORTANCE_BUFFER));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, MASTER_SHADER_SPOT_LIGHTS_BUFFER_INDEX, i, SPOT_LIGHT_SYSTEM()->DeviceBuffer);
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, MASTER_SHADER_SPOT_LIGHTS_IMPORTANCE_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(DIRECTIONAL_LIGHTS_IMPORTANCE_BUFFER));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, MASTER_SHADER_POINT_LIGHTS_BUFFER_INDEX, i, POINT_LIGHT_SYSTEM()->DeviceBuffer);
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, MASTER_SHADER_POINT_LIGHTS_IMPORTANCE_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(POINT_LIGHTS_IMPORTANCE_BUFFER));
        UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, MASTER_SHADER_AREA_LIGHTS_BUFFER_INDEX, i, AREA_LIGHT_SYSTEM()->DeviceBuffer);
        UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, MASTER_SHADER_AREA_LIGHTS_IMPORTANCE_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(AREA_LIGHTS_IMPORTANCE_BUFFER));
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, MASTER_SHADER_RENDERABLES_BUFFER_INDEX, i, RENDERABLE_SYSTEM()->DeviceBuffer);
		UpdateDescriptorSet(MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, MASTER_SHADER_TRANSFORMS_BUFFER_INDEX, i, TRANSFORM_SYSTEM()->DeviceBuffer);
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
				std::vector<VkDescriptorSet> RayTracingDescriptorSets = {
					VK_CONTEXT()->DescriptorSetManager->GetSet(Name, MASTER_SHADER_LAYOUT_STATIC_INDEX, 0),
					VK_CONTEXT()->DescriptorSetManager->GetSet(Name, MASTER_SHADER_LAYOUT_INDEX_PER_FRAME, i / SubmitX)};
				vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
					0, RayTracingDescriptorSets.size(), RayTracingDescriptorSets.data(), 0, nullptr);

				FPushConstants PushConstants = { Width, Height, 1.f / float(Width), 1.f / float(Height), Width * Height, MaterialIndex, i % SubmitX };
				vkCmdPushConstants(CommandBuffer, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name), VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, sizeof(FPushConstants), &PushConstants);

				V::vkCmdTraceRaysIndirectKHR(CommandBuffer, &RGenRegions[MaterialIndex], &RMissRegion, &RHitRegion, &RCallRegion, TotalCountedMaterialsBuffer + (MaterialIndex * 3 * sizeof(uint32_t)));
			}
        }, QueueFlagsBits);

        V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
    }
};
