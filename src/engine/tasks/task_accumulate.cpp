#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "common_structures.h"

#include "vk_shader_compiler.h"
#include "texture_manager.h"

#include "task_accumulate.h"

#include "utils.h"

#include "renderer_options.h"

FAccumulateTask::FAccumulateTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
    Name = "Task Accumulate";

    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, ACCUMULATE_PER_FRAME_LAYOUT_INDEX, INCOMING_IMAGE_TO_SAMPLE,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, ACCUMULATE_PER_FRAME_LAYOUT_INDEX, ACCUMULATE_IMAGE_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, ACCUMULATE_PER_FRAME_LAYOUT_INDEX, ESTIMATED_IMAGE_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});

	VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstants)};
	DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

void FAccumulateTask::Init(FCompileDefinitions* CompileDefinitions)
{
    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    auto AccumulateShader = FShader("../src/shaders/accumulate.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

    Pipeline = VK_CONTEXT()->CreateComputePipeline(AccumulateShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, ACCUMULATE_PER_FRAME_LAYOUT_INDEX, TotalSize);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FAccumulateTask::UpdateDescriptorSets()
{
    for (uint32_t i = 0; i < TotalSize; ++i)
    {
		if (RENDER_STATE()->RenderTarget == EOutputType::Color)
		{
			UpdateDescriptorSet(ACCUMULATE_PER_FRAME_LAYOUT_INDEX, INCOMING_IMAGE_TO_SAMPLE, i, TEXTURE_MANAGER()->GetFramebufferImage("ColorImage"));
		}
		else
		{
			if (RENDER_STATE()->RenderTarget >= EOutputType::Max)
			{
				throw std::runtime_error("Unsupported AOV input.");
			}
			else
			{
				UpdateDescriptorSet(ACCUMULATE_PER_FRAME_LAYOUT_INDEX, INCOMING_IMAGE_TO_SAMPLE, i, TEXTURE_MANAGER()->GetFramebufferImage("AOVImage"));
			}

		}
        UpdateDescriptorSet(ACCUMULATE_PER_FRAME_LAYOUT_INDEX, ACCUMULATE_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("AccumulatorImage"));
        UpdateDescriptorSet(ACCUMULATE_PER_FRAME_LAYOUT_INDEX, ESTIMATED_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("EstimatedImage"));
    }
};

void FAccumulateTask::RecordCommands()
{
    CommandBuffers.resize(TotalSize);

    for (uint32_t i = 0; i < TotalSize; ++i)
    {
        CommandBuffers[i] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
			ResetQueryPool(CommandBuffer, i);
			GPU_TIMER();

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto RayTracingDescriptorSet = VK_CONTEXT()->DescriptorSetManager->GetSet(Name, ACCUMULATE_PER_FRAME_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &RayTracingDescriptorSet, 0, nullptr);

            uint32_t GroupCount = CalculateMaxGroupCount(Width * Height, BASIC_CHUNK_SIZE);
			FPushConstants PushConstants = {Width, Height, 1.f / float(Width), 1.f / float(Height), Width * Height, 0, 0};
			vkCmdPushConstants(CommandBuffer, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstants), &PushConstants);
            vkCmdDispatch(CommandBuffer, GroupCount, 1, 1);

        }, QueueFlagsBits);

        V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
    }
};
