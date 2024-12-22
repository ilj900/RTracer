#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

#include "vk_shader_compiler.h"
#include "texture_manager.h"

#include "task_clear_buffer.h"

#include "common_structures.h"

#include "utils.h"

FClearBufferTask::FClearBufferTask(const std::string& BufferNameIn, uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
	BufferName(BufferNameIn),
	FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
    Name = "Clear" + BufferName + " buffer pipeline";

    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, CLEAR_BUFFER_LAYOUT_INDEX, CLEAR_BUFFER_BUFFER_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

	VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FViewportResolutionPushConstants)};
	DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

void FClearBufferTask::Init()
{
    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    auto ClearImageShader = FShader("../../../src/shaders/clear_buffer.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

    Pipeline = VK_CONTEXT()->CreateComputePipeline(ClearImageShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, CLEAR_BUFFER_LAYOUT_INDEX, TotalSize);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FClearBufferTask::UpdateDescriptorSets()
{
    for (uint32_t i = 0; i < TotalSize; ++i)
    {
        UpdateDescriptorSet(CLEAR_BUFFER_LAYOUT_INDEX, CLEAR_BUFFER_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(BufferName));
    }
};

void FClearBufferTask::RecordCommands()
{
    CommandBuffers.resize(TotalSize);

    for (uint32_t i = 0; i < TotalSize; ++i)
    {
        CommandBuffers[i] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
			ResetQueryPool(CommandBuffer, i);
			GPU_TIMER();

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto DescriptorSet = VK_CONTEXT()->DescriptorSetManager->GetSet(Name, CLEAR_BUFFER_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &DescriptorSet, 0, nullptr);

			FBuffer Buffer = RESOURCE_ALLOCATOR()->GetBuffer(BufferName);
			uint32_t GroupCount = CalculateMaxGroupCount(Buffer.BufferSize / 4, BASIC_CHUNK_SIZE);

			FViewportResolutionPushConstants PushConstants = {uint32_t(Buffer.BufferSize / 4), 1};
			vkCmdPushConstants(CommandBuffer, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
				VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FViewportResolutionPushConstants), &PushConstants);

            vkCmdDispatch(CommandBuffer, GroupCount, 1, 1);
        }, QueueFlagsBits);

        V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
    }
};
