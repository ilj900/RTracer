#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "common_structures.h"

#include "systems/renderable_system.h"
#include "systems/transform_system.h"

#include "vk_shader_compiler.h"

#include "task_compute_shading_data.h"

FComputeShadingDataTask::FComputeShadingDataTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
    Name = "Compute shading data pipeline";

    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADING_DATA_LAYOUT_INDEX, COMPUTE_SHADING_DATA_RENDERABLES_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADING_DATA_LAYOUT_INDEX, COMPUTE_SHADING_DATA_TRANSFORMS_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADING_DATA_LAYOUT_INDEX, COMPUTE_SHADING_DATA_HITS_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADING_DATA_LAYOUT_INDEX, COMPUTE_SHADING_DATA_RAYS_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADING_DATA_LAYOUT_INDEX, COMPUTE_SHADING_DATA_TRANSFORM_INDEX_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADING_DATA_LAYOUT_INDEX, COMPUTE_SHADING_DATA_RAY_INDEX_MAP_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADING_DATA_LAYOUT_INDEX, COMPUTE_SHADING_DATA_NORMAL_AOV_DATA_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADING_DATA_LAYOUT_INDEX, COMPUTE_SHADING_DATA_UV_AOV_DATA_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADING_DATA_LAYOUT_INDEX, COMPUTE_SHADING_DATA_WORLD_SPACE_POSITION_AOV_DATA_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FViewportResolutionPushConstants)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

FComputeShadingDataTask::~FComputeShadingDataTask()
{
}

void FComputeShadingDataTask::Init()
{
    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

	auto Shader = FShader("../../../src/shaders/compute_shading_data.comp");

	Pipeline = VK_CONTEXT()->CreateComputePipeline(Shader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, COMPUTE_SHADE_LAYOUT_INDEX, TotalSize);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FComputeShadingDataTask::UpdateDescriptorSets()
{
    for (uint32_t i = 0; i < TotalSize; ++i)
    {
        UpdateDescriptorSet(COMPUTE_SHADING_DATA_LAYOUT_INDEX, COMPUTE_SHADING_DATA_RENDERABLES_BUFFER_INDEX, i, RENDERABLE_SYSTEM()->DeviceBuffer);
        UpdateDescriptorSet(COMPUTE_SHADING_DATA_LAYOUT_INDEX, COMPUTE_SHADING_DATA_TRANSFORMS_BUFFER_INDEX, i, TRANSFORM_SYSTEM()->DeviceBuffer);
        UpdateDescriptorSet(COMPUTE_SHADING_DATA_LAYOUT_INDEX, COMPUTE_SHADING_DATA_HITS_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer("HitsBuffer"));
        UpdateDescriptorSet(COMPUTE_SHADING_DATA_LAYOUT_INDEX, COMPUTE_SHADING_DATA_RAYS_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer("InitialRaysBuffer"));
		UpdateDescriptorSet(COMPUTE_SHADING_DATA_LAYOUT_INDEX, COMPUTE_SHADING_DATA_TRANSFORM_INDEX_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer("TransformIndexBuffer"));
		UpdateDescriptorSet(COMPUTE_SHADING_DATA_LAYOUT_INDEX, COMPUTE_SHADING_DATA_RAY_INDEX_MAP_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer("PixelIndexBuffer"));
        UpdateDescriptorSet(COMPUTE_SHADING_DATA_LAYOUT_INDEX, COMPUTE_SHADING_DATA_NORMAL_AOV_DATA_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer("NormalAOVBuffer"));
        UpdateDescriptorSet(COMPUTE_SHADING_DATA_LAYOUT_INDEX, COMPUTE_SHADING_DATA_UV_AOV_DATA_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer("UVAOVBuffer"));
        UpdateDescriptorSet(COMPUTE_SHADING_DATA_LAYOUT_INDEX, COMPUTE_SHADING_DATA_WORLD_SPACE_POSITION_AOV_DATA_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer("WorldSpacePositionAOVBuffer"));
    }
};

void FComputeShadingDataTask::RecordCommands()
{
    CommandBuffers.resize(TotalSize);
	auto ActiveRayCountBufferDeviceAddress = RESOURCE_ALLOCATOR()->GetBuffer("ActiveRayCountBuffer");

	for (uint32_t i = 0; i < TotalSize; ++i)
	{
		CommandBuffers[i] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
		{
			ResetQueryPool(CommandBuffer, i);
			GPU_TIMER();

			vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
			auto DescriptorSet = VK_CONTEXT()->DescriptorSetManager->GetSet(Name, COMPUTE_SHADING_DATA_LAYOUT_INDEX, i);
			vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
				0, 1, &DescriptorSet, 0, nullptr);

			FViewportResolutionPushConstants PushConstants = { Width, Height, i % SubmitX};
			vkCmdPushConstants(CommandBuffer, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
				VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FViewportResolutionPushConstants), &PushConstants);

			vkCmdDispatchIndirect(CommandBuffer, ActiveRayCountBufferDeviceAddress.Buffer, 0);
		}, QueueFlagsBits);

		V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
    }
};
