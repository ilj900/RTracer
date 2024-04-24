#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"

#include "vk_shader_compiler.h"

#include "utils.h"

#include "task_reset_active_ray_count.h"

FResetActiveRayCountTask::FResetActiveRayCountTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
	FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
	Name = "Reset active ray count pipeline";

	auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

	DescriptorSetManager->AddDescriptorLayout(Name, RESET_ACTIVE_RAY_COUNT_LAYOUT_INDEX, ACTIVE_RAY_COUNT_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

	VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)};
	DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

	auto ActiveRayCountBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(sizeof(uint32_t) * 3, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "ActiveRayCountBuffer");
	RESOURCE_ALLOCATOR()->RegisterBuffer(ActiveRayCountBuffer, "ActiveRayCountBuffer");

	PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

FResetActiveRayCountTask::~FResetActiveRayCountTask()
{
};

void FResetActiveRayCountTask::Init()
{
	TIMING_MANAGER()->RegisterTiming(Name, SubmitX, SubmitY);

	auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

	auto ShadeShader = FShader("../../../src/shaders/reset_active_ray_count.comp");

	PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

	Pipeline = VK_CONTEXT()->CreateComputePipeline(ShadeShader(), PipelineLayout);

	/// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
	DescriptorSetManager->ReserveDescriptorSet(Name, RESET_ACTIVE_RAY_COUNT_LAYOUT_INDEX, TotalSize);

	DescriptorSetManager->ReserveDescriptorPool(Name);

	DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FResetActiveRayCountTask::UpdateDescriptorSets()
{
	for (size_t i = 0; i < TotalSize; ++i)
	{
		UpdateDescriptorSet(RESET_ACTIVE_RAY_COUNT_LAYOUT_INDEX, ACTIVE_RAY_COUNT_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("ActiveRayCountBuffer"));
	}
};

void FResetActiveRayCountTask::RecordCommands()
{
	CommandBuffers.resize(TotalSize);

	for (std::size_t i = 0; i < TotalSize; ++i)
	{
		CommandBuffers[i] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
			{
				uint32_t X = i % SubmitX;
				uint32_t Y = i / SubmitX;

				if (X == 0)
				{
					TIMING_MANAGER()->TimestampReset(Name, CommandBuffer, Y);
				}
				TIMING_MANAGER()->TimestampStart(Name, CommandBuffer, X, Y);

				vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
				auto DescriptorSet = VK_CONTEXT()->DescriptorSetManager->GetSet(Name, RESET_ACTIVE_RAY_COUNT_LAYOUT_INDEX, i);
				vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
					0, 1, &DescriptorSet, 0, nullptr);

				uint32_t PushConstants = {Width * Height};
				vkCmdPushConstants(CommandBuffer, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
					VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &PushConstants);

				vkCmdDispatch(CommandBuffer, 1, 1, 1);

				TIMING_MANAGER()->TimestampEnd(Name, CommandBuffer, X, Y);
			}, QueueFlagsBits);

		V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
	}
};
