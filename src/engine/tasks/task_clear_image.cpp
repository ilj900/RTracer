#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

#include "vk_shader_compiler.h"
#include "texture_manager.h"

#include "task_clear_image.h"

FClearImageTask::FClearImageTask(uint32_t WidthIn, uint32_t HeightIn, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Clear image pipeline";

    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, CLEAR_IMAGE_LAYOUT_INDEX, IMAGE_TO_CLEAR,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});

    DescriptorSetManager->CreateDescriptorSetLayout({}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

void FClearImageTask::Init()
{
    TIMING_MANAGER()->RegisterTiming(Name, NumberOfSimultaneousSubmits);

    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    auto ClearImageShader = FShader("../../../src/shaders/clear_image.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

    Pipeline = VK_CONTEXT()->CreateComputePipeline(ClearImageShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, CLEAR_IMAGE_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FClearImageTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        UpdateDescriptorSet(CLEAR_IMAGE_LAYOUT_INDEX, IMAGE_TO_CLEAR, i, TEXTURE_MANAGER()->GetFramebufferImage("AccumulatorImage"));
    }
};

void FClearImageTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            TIMING_MANAGER()->TimestampStart(Name, CommandBuffer, i);

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto RayTracingDescriptorSet = VK_CONTEXT()->DescriptorSetManager->GetSet(Name, CLEAR_IMAGE_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &RayTracingDescriptorSet, 0, nullptr);

            int GroupSizeX = (Width % 8 == 0) ? (Width / 8) : (Width / 8) + 1;
            int GroupSizeY = (Height % 8 == 0) ? (Height / 8) : (Height / 8) + 1;

            vkCmdDispatch(CommandBuffer, GroupSizeX, GroupSizeY, 1);

            TIMING_MANAGER()->TimestampEnd(Name, CommandBuffer, i);
        }, QueueFlagsBits);

        V::SetName(LogicalDevice, CommandBuffers[i], Name);
    }
};
