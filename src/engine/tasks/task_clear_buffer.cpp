#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

#include "task_clear_buffer.h"

#include "utils.h"

FClearBufferTask::FClearBufferTask(const std::vector<std::string>& BufferNamesIn, uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice, uint32_t ClearValueIn) :
	BufferNames(BufferNamesIn),
	ClearValue(ClearValueIn),
	FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
	static int Counter = 0;
	
	if (BufferNames.size() == 1)
	{
		Name = "Clear buffer " + BufferNames[0] + " pipeline";
	}
	else
	{
		Name = "Clear buffer(s) pipeline " + std::to_string(Counter);
		++Counter;
	}

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

FClearBufferTask::FClearBufferTask(const std::string& BufferNameIn, uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice, uint32_t ClearValueIn) :
	BufferNames(1, BufferNameIn),
	ClearValue(ClearValueIn),
	FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
	Name = "Clear buffer " + BufferNames[0] + " pipeline";
	PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

void FClearBufferTask::Init(FCompileDefinitions* CompileDefinitions)
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
			for (auto& BufferName : BufferNames)
			{
				auto Buffer = RESOURCE_ALLOCATOR()->GetBuffer(BufferName);
				vkCmdFillBuffer(CommandBuffer, Buffer.Buffer, 0, VK_WHOLE_SIZE , ClearValue);
			}
        }, QueueFlagsBits);

        V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
    }
};
