#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

#include "mesh_system.h"
#include "systems/camera_system.h"
#include "renderable_system.h"
#include "acceleration_structure_system.h"
#include "vk_shader_compiler.h"

#include "task_raytrace.h"
#include "texture_manager.h"

#include "common_structures.h"

FRaytraceTask::FRaytraceTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
    Name = "RayTracing pipeline";

    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_TLAS_LAYOUT_INDEX,
		{VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_RAYS_DATA_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_PIXEL_INDEX_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_RENDERABLE_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_HIT_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_MATERIAL_INDEX_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_CAMERA_POSITION_BUFFER,
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_RENDER_ITERATION_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_THROUGHPUT_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR});

	VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstants)};
	DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

FRaytraceTask::~FRaytraceTask()
{
    GetResourceAllocator()->DestroyBuffer(SBTBuffer);
};

void FRaytraceTask::Init(FCompileDefinitions* CompileDefinitions)
{
    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    auto RayGenerationShader = FShader("../src/shaders/raytrace.rgen");
    auto RayClosestHitShader = FShader("../src/shaders/raytrace.rchit");
    auto RayMissShader = FShader("../src/shaders/raytrace.rmiss");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

    Pipeline = VK_CONTEXT()->CreateRayTracingPipeline(RayGenerationShader(), RayMissShader(), RayClosestHitShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, RAYTRACE_LAYOUT_INDEX, TotalSize);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);

    SBTBuffer = VK_CONTEXT()->GenerateSBT(Pipeline, RMissRegion, RHitRegion, RGenRegion);
};

void FRaytraceTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < TotalSize; ++i)
    {
        VK_CONTEXT()->DescriptorSetManager->UpdateDescriptorSetInfo(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_TLAS_LAYOUT_INDEX, i, &ACCELERATION_STRUCTURE_SYSTEM()->TLAS.AccelerationStructure);
        UpdateDescriptorSet(RAYTRACE_LAYOUT_INDEX, RAYTRACE_RAYS_DATA_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer(INITIAL_RAYS_BUFFER));
		UpdateDescriptorSet(RAYTRACE_LAYOUT_INDEX, RAYTRACE_PIXEL_INDEX_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer(PIXEL_INDEX_BUFFER));
        UpdateDescriptorSet(RAYTRACE_LAYOUT_INDEX, RAYTRACE_RENDERABLE_BUFFER_INDEX, i, RENDERABLE_SYSTEM()->DeviceBuffer);
        UpdateDescriptorSet(RAYTRACE_LAYOUT_INDEX, RAYTRACE_HIT_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer(HITS_BUFFER));
        UpdateDescriptorSet(RAYTRACE_LAYOUT_INDEX, RAYTRACE_MATERIAL_INDEX_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer(MATERIAL_INDEX_AOV_BUFFER));
		UpdateDescriptorSet(RAYTRACE_LAYOUT_INDEX, RAYTRACE_CAMERA_POSITION_BUFFER, i, CAMERA_SYSTEM()->DeviceBuffer);
		UpdateDescriptorSet(RAYTRACE_LAYOUT_INDEX, RAYTRACE_RENDER_ITERATION_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer(RENDER_ITERATION_BUFFER));
		UpdateDescriptorSet(RAYTRACE_LAYOUT_INDEX, RAYTRACE_THROUGHPUT_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer(THROUGHPUT_BUFFER));
    }
};

void FRaytraceTask::RecordCommands()
{
    CommandBuffers.resize(TotalSize);
	auto ActiveRayCountBufferDeviceAddress = VK_CONTEXT()->GetBufferDeviceAddressInfo(RESOURCE_ALLOCATOR()->GetBuffer(ACTIVE_RAY_COUNT_BUFFER));

    for (uint32_t i = 0; i < TotalSize; ++i)
    {
        CommandBuffers[i] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
			ResetQueryPool(CommandBuffer, i);
			GPU_TIMER();

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, Pipeline);
            auto RayTracingDescriptorSet = VK_CONTEXT()->DescriptorSetManager->GetSet(Name, RAYTRACE_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &RayTracingDescriptorSet, 0, nullptr);

			FPushConstants PushConstants = {Width, Height, 1.f / float(Width), 1.f / float(Height), Width * Height, 0, i % SubmitX};
			vkCmdPushConstants(CommandBuffer, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name), VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, sizeof(FPushConstants), &PushConstants);

            V::vkCmdTraceRaysIndirectKHR(CommandBuffer, &RGenRegion, &RMissRegion, &RHitRegion, &RCallRegion, ActiveRayCountBufferDeviceAddress);
        }, QueueFlagsBits);

        V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
    }
};
