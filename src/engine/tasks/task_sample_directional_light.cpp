#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

#include "common_structures.h"
#include "transform_system.h"
#include "directional_light_system.h"
#include "acceleration_structure_system.h"
#include "vk_shader_compiler.h"

#include "task_sample_directional_light.h"
#include "texture_manager.h"

FSampleDirectionalLightTask::FSampleDirectionalLightTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
    Name = "Directional light sampling pipeline";

    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_LAYOUT_INDEX, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_TLAS_INDEX,
		{VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,  VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_LAYOUT_INDEX, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_DIRECTIONAL_LIGHTS_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_LAYOUT_INDEX, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_UNIFORM_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_LAYOUT_INDEX, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_NORMAL_AOV_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_LAYOUT_INDEX, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_WORLD_SPACE_POSITION_AOV_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_LAYOUT_INDEX, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_SAMPLED_LIGHT_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_LAYOUT_INDEX, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_PIXEL_INDEX_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_LAYOUT_INDEX, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_RENDER_ITERATION_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});

	VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, sizeof(FViewportResolutionPushConstants)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

FSampleDirectionalLightTask::~FSampleDirectionalLightTask()
{
    GetResourceAllocator()->DestroyBuffer(SBTBuffer);
};

void FSampleDirectionalLightTask::Init(FCompileDefinitions* CompileDefinitions)
{
    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    auto RayGenerationShader = FShader("../../../src/shaders/raytrace_sample_direct_light.rgen");
    auto RayClosestHitShader = FShader("../../../src/shaders/raytrace_sample_direct_light.rchit");
    auto RayMissShader = FShader("../../../src/shaders/raytrace_sample_direct_light.rmiss");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

    Pipeline = VK_CONTEXT()->CreateRayTracingPipeline(RayGenerationShader(), RayMissShader(), RayClosestHitShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_LAYOUT_INDEX, TotalSize);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);

	SBTBuffer = VK_CONTEXT()->GenerateSBT(Pipeline, RMissRegion, RHitRegion, RGenRegion);
};

void FSampleDirectionalLightTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < TotalSize; ++i)
    {
        VK_CONTEXT()->DescriptorSetManager->UpdateDescriptorSetInfo(Name, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_LAYOUT_INDEX, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_TLAS_INDEX, i, &ACCELERATION_STRUCTURE_SYSTEM()->TLAS.AccelerationStructure);
		UpdateDescriptorSet(RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_LAYOUT_INDEX, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_DIRECTIONAL_LIGHTS_BUFFER_INDEX, i, DIRECTIONAL_LIGHT_SYSTEM()->DeviceBuffer);
		UpdateDescriptorSet(RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_LAYOUT_INDEX, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_UNIFORM_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(UTILITY_INFO_DIRECTIONAL_LIGHT_BUFFER));
        UpdateDescriptorSet(RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_LAYOUT_INDEX, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_NORMAL_AOV_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(NORMAL_AOV_BUFFER));
		UpdateDescriptorSet(RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_LAYOUT_INDEX, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_WORLD_SPACE_POSITION_AOV_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(WORLD_SPACE_POSITION_AOV_BUFFER));
        UpdateDescriptorSet(RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_LAYOUT_INDEX, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_SAMPLED_LIGHT_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(SAMPLED_DIRECTIONAL_LIGHT_BUFFER));
		UpdateDescriptorSet(RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_LAYOUT_INDEX, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_PIXEL_INDEX_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer(PIXEL_INDEX_BUFFER));
		UpdateDescriptorSet(RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_LAYOUT_INDEX, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_RENDER_ITERATION_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(RENDER_ITERATION_BUFFER));
    }
};

void FSampleDirectionalLightTask::RecordCommands()
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
            auto RayTracingDescriptorSet = VK_CONTEXT()->DescriptorSetManager->GetSet(Name, RAYTRACE_SAMPLE_DIRECTIONAL_LIGHT_LAYOUT_INDEX, i);
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

FSynchronizationPoint FSampleDirectionalLightTask::Submit(VkPipelineStageFlags& PipelineStageFlagsIn, FSynchronizationPoint SynchronizationPoint, uint32_t X, uint32_t Y)
{
	/// If there are no directional light then just skip the task
	if (DIRECTIONAL_LIGHT_SYSTEM()->UtilityDirectionalLight.ActiveLightsCount > 0)
	{
		return FExecutableTask::Submit(PipelineStageFlagsIn, SynchronizationPoint, X, Y);
	}
	else
	{
		return SynchronizationPoint;
	}
}
