#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

#include "task_copy_buffer.h"

#include "utils.h"

FCopyBufferTask::FCopyBufferTask(const std::string& SrcBufferNameIn, const std::string& DstBufferNameIn, uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
	SrcBufferName(SrcBufferNameIn),
	DstBufferName(DstBufferNameIn),
	FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
    Name = "Copy" + SrcBufferName + " to " + DstBufferName + " buffer pipeline";

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

void FCopyBufferTask::Init(FCompileDefinitions* CompileDefinitions)
{
};

void FCopyBufferTask::UpdateDescriptorSets()
{
};

void FCopyBufferTask::RecordCommands()
{
    CommandBuffers.resize(TotalSize);

    for (uint32_t i = 0; i < TotalSize; ++i)
    {
        CommandBuffers[i] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
			ResetQueryPool(CommandBuffer, i);
			GPU_TIMER();
			auto SrcBuffer = RESOURCE_ALLOCATOR()->GetBuffer(SrcBufferName);
			auto DstBuffer = RESOURCE_ALLOCATOR()->GetBuffer(DstBufferName);
			VkBufferCopy BufferCopy{0, 0, DstBuffer.BufferSize};
			vkCmdCopyBuffer(CommandBuffer, SrcBuffer.Buffer, DstBuffer.Buffer, 1, &BufferCopy);
        }, QueueFlagsBits);

        V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
    }
};
