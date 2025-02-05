#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

#include "mesh_component.h"
#include "mesh_system.h"
#include "renderable_system.h"
#include "transform_system.h"
#include "acceleration_structure_system.h"
#include "vk_shader_compiler.h"

#include "task_sample_ibl.h"
#include "texture_manager.h"

FSampleIBLTask::FSampleIBLTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
    Name = "IBL sampling pipeline";

    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_TLAS_INDEX,
		{VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,  VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_IMPORTANCE_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_IMAGE_SAMPLER_INDEX,
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_WEIGHTS_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_NORMAL_AOV_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_WORLD_SPACE_POSITION_AOV_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_SAMPLED_IBL_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_TRANSFORM_INDEX_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_DEVICE_TRANSFORM_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_PIXEL_INDEX_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_RENDER_ITERATION_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});

	VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, sizeof(FViewportResolutionPushConstants)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

	Sampler = VK_CONTEXT()->CreateTextureSampler(VK_SAMPLE_COUNT_1_BIT, VK_FILTER_NEAREST);

    PipelineStageFlags = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

FSampleIBLTask::~FSampleIBLTask()
{
	vkDestroySampler(LogicalDevice, Sampler, nullptr);
    GetResourceAllocator()->DestroyBuffer(SBTBuffer);
};

void FSampleIBLTask::Init(FCompileDefinitions* CompileDefinitions)
{
    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    auto RayGenerationShader = FShader("../../../src/shaders/raytrace_sample_ibl.rgen");
    auto RayClosestHitShader = FShader("../../../src/shaders/raytrace_sample_ibl.rchit");
    auto RayMissShader = FShader("../../../src/shaders/raytrace_sample_ibl.rmiss");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

    Pipeline = VK_CONTEXT()->CreateRayTracingPipeline(RayGenerationShader(), RayMissShader(), RayClosestHitShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, TotalSize);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);

	SBTBuffer = VK_CONTEXT()->GenerateSBT(Pipeline, RMissRegion, RHitRegion, RGenRegion);
};

void FSampleIBLTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < TotalSize; ++i)
    {
        VK_CONTEXT()->DescriptorSetManager->UpdateDescriptorSetInfo(Name, RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_TLAS_INDEX, i, &ACCELERATION_STRUCTURE_SYSTEM()->TLAS.AccelerationStructure);
		UpdateDescriptorSet(RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_IMPORTANCE_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer("IBLImportanceBuffer"));
        UpdateDescriptorSet(RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_IMAGE_SAMPLER_INDEX, i, TEXTURE_MANAGER()->GetTexture("IBL Image"), Sampler);
		UpdateDescriptorSet(RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_WEIGHTS_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer("InversePDFWeightBuffer"));
        UpdateDescriptorSet(RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_NORMAL_AOV_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(NORMAL_AOV_BUFFER));
		UpdateDescriptorSet(RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_WORLD_SPACE_POSITION_AOV_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(WORLD_SPACE_POSITION_AOV_BUFFER));
		UpdateDescriptorSet(RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_TRANSFORM_INDEX_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(TRANSFORM_INDEX_BUFFER));
		UpdateDescriptorSet(RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_DEVICE_TRANSFORM_BUFFER_INDEX, i, TRANSFORM_SYSTEM()->DeviceBuffer);
        UpdateDescriptorSet(RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_SAMPLED_IBL_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(SAMPLED_IBL_BUFFER));
		UpdateDescriptorSet(RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_PIXEL_INDEX_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer(PIXEL_INDEX_BUFFER));
		UpdateDescriptorSet(RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, RAYTRACE_SAMPLE_IBL_RENDER_ITERATION_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(RENDER_ITERATION_BUFFER));
    }
};

void FSampleIBLTask::RecordCommands()
{
    CommandBuffers.resize(TotalSize);
	auto ActiveRayCountBufferDeviceAddress = VK_CONTEXT()->GetBufferDeviceAddressInfo(RESOURCE_ALLOCATOR()->GetBuffer(ACTIVE_RAY_COUNT_BUFFER));

    for (std::size_t i = 0; i < TotalSize; ++i)
    {
        CommandBuffers[i] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
			ResetQueryPool(CommandBuffer, i);
			GPU_TIMER();

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, Pipeline);
            auto RayTracingDescriptorSet = VK_CONTEXT()->DescriptorSetManager->GetSet(Name, RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &RayTracingDescriptorSet, 0, nullptr);

			FViewportResolutionPushConstants PushConstants = { Width, Height};
			vkCmdPushConstants(CommandBuffer, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
				VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, sizeof(FViewportResolutionPushConstants), &PushConstants);

            V::vkCmdTraceRaysIndirectKHR(CommandBuffer, &RGenRegion, &RMissRegion, &RHitRegion, &RCallRegion, ActiveRayCountBufferDeviceAddress);
        }, QueueFlagsBits);

        V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
    }
};
