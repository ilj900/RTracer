#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "common_structures.h"

#include "vk_shader_compiler.h"
#include "texture_manager.h"

#include "utils.h"

#include "task_aov_pass.h"

FAOVPassTask::FAOVPassTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
	FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
	Name = "AOV pass pipeline";

	auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

	DescriptorSetManager->AddDescriptorLayout(Name, AOV_PASS_LAYOUT_INDEX, AOV_PASS_NORMAL_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, AOV_PASS_LAYOUT_INDEX, AOV_PASS_NORMAL_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, AOV_PASS_LAYOUT_INDEX, AOV_PASS_UV_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, AOV_PASS_LAYOUT_INDEX, AOV_PASS_UV_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, AOV_PASS_LAYOUT_INDEX, AOV_PASS_WORLD_SPACE_POSITION_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, AOV_PASS_LAYOUT_INDEX, AOV_PASS_WORLD_SPACE_POSITION_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, AOV_PASS_LAYOUT_INDEX, AOV_PASS_DEBUG_LAYER_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, AOV_PASS_LAYOUT_INDEX, AOV_PASS_DEBUG_LAYER_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});

	VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FViewportResolutionPushConstants)};
	DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

	PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

FAOVPassTask::~FAOVPassTask()
{
};

void FAOVPassTask::Init()
{
	auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

	auto ShadeShader = FShader("../../../src/shaders/aov_pass.comp");

	PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

	Pipeline = VK_CONTEXT()->CreateComputePipeline(ShadeShader(), PipelineLayout);

	/// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
	DescriptorSetManager->ReserveDescriptorSet(Name, COMPUTE_MISS_LAYOUT_INDEX, TotalSize);

	DescriptorSetManager->ReserveDescriptorPool(Name);

	DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FAOVPassTask::UpdateDescriptorSets()
{
	for (size_t i = 0; i < TotalSize; ++i)
	{
		UpdateDescriptorSet(AOV_PASS_LAYOUT_INDEX, AOV_PASS_NORMAL_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("NormalAOVBuffer"));
		UpdateDescriptorSet(AOV_PASS_LAYOUT_INDEX, AOV_PASS_NORMAL_AOV_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("NormalAOVImage"));
		UpdateDescriptorSet(AOV_PASS_LAYOUT_INDEX, AOV_PASS_UV_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("UVAOVBuffer"));
		UpdateDescriptorSet(AOV_PASS_LAYOUT_INDEX, AOV_PASS_UV_AOV_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("UVAOVImage"));
		UpdateDescriptorSet(AOV_PASS_LAYOUT_INDEX, AOV_PASS_WORLD_SPACE_POSITION_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("WorldSpacePositionAOVBuffer"));
		UpdateDescriptorSet(AOV_PASS_LAYOUT_INDEX, AOV_PASS_WORLD_SPACE_POSITION_AOV_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("WorldSpacePositionAOVImage"));
		UpdateDescriptorSet(AOV_PASS_LAYOUT_INDEX, AOV_PASS_DEBUG_LAYER_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("DebugLayerBuffer"));
		UpdateDescriptorSet(AOV_PASS_LAYOUT_INDEX, AOV_PASS_DEBUG_LAYER_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("DebugLayerImage"));
	}
};

void FAOVPassTask::RecordCommands()
{
	CommandBuffers.resize(TotalSize);

	for (uint32_t i = 0; i < TotalSize; ++i)
	{
		CommandBuffers[i] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
			{
				ResetQueryPool(CommandBuffer, i);
				GPU_TIMER();

				vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
				auto DescriptorSet = VK_CONTEXT()->DescriptorSetManager->GetSet(Name, COMPUTE_MISS_LAYOUT_INDEX, i);
				vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
					0, 1, &DescriptorSet, 0, nullptr);

				FViewportResolutionPushConstants PushConstants = {Width, Height};
				vkCmdPushConstants(CommandBuffer, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
					VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FViewportResolutionPushConstants), &PushConstants);

				vkCmdDispatch(CommandBuffer, CalculateGroupCount(Width * Height, BASIC_CHUNK_SIZE), 1, 1);
			}, QueueFlagsBits);

		V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
	}
};