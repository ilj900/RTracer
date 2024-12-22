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

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

void FClearBufferTask::Init()
{
};

void FClearBufferTask::UpdateDescriptorSets()
{
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
			auto Buffer = RESOURCE_ALLOCATOR()->GetBuffer(BufferName);
			vkCmdFillBuffer(CommandBuffer, Buffer.Buffer, 0, VK_WHOLE_SIZE , 0);
        }, QueueFlagsBits);

        V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
    }
};
