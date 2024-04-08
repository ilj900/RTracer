#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"

#include "vk_shader_compiler.h"
#include "texture_manager.h"

#include "task_accumulate.h"

FAccumulateTask::FAccumulateTask(uint32_t WidthIn, uint32_t HeightIn, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Task Accumulate";

    auto& DescriptorSetManager = VK_CONTEXT().DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, ACCUMULATE_PER_FRAME_LAYOUT_INDEX, INCOMING_IMAGE_TO_SAMPLE,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, ACCUMULATE_PER_FRAME_LAYOUT_INDEX, ACCUMULATE_IMAGE_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, ACCUMULATE_PER_FRAME_LAYOUT_INDEX, ESTIMATED_IMAGE_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});

    DescriptorSetManager->CreateDescriptorSetLayout({}, Name);

    auto AccumulatorImage = TEXTURE_MANAGER()->CreateStorageImage(Width, Height,"AccumulatorImage");
    TEXTURE_MANAGER()->RegisterFramebuffer(AccumulatorImage, "AccumulatorImage");
    AccumulatorImage->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    auto EstimatedImage = TEXTURE_MANAGER()->CreateSampledStorageImage(Width, Height, "EstimatedImage");
    TEXTURE_MANAGER()->RegisterFramebuffer(EstimatedImage, "EstimatedImage");
    EstimatedImage->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
}

void FAccumulateTask::Init()
{
    TIMING_MANAGER()->RegisterTiming(Name, NumberOfSimultaneousSubmits);

    auto& DescriptorSetManager = VK_CONTEXT().DescriptorSetManager;

    auto AccumulateShader = FShader("../../../src/shaders/accumulate.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

    Pipeline = VK_CONTEXT().CreateComputePipeline(AccumulateShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, ACCUMULATE_PER_FRAME_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FAccumulateTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        UpdateDescriptorSet(ACCUMULATE_PER_FRAME_LAYOUT_INDEX, INCOMING_IMAGE_TO_SAMPLE, i, TEXTURE_MANAGER()->GetFramebufferImage("RayTracingColorImage"));
        UpdateDescriptorSet(ACCUMULATE_PER_FRAME_LAYOUT_INDEX, ACCUMULATE_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("AccumulatorImage"));
        UpdateDescriptorSet(ACCUMULATE_PER_FRAME_LAYOUT_INDEX, ESTIMATED_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("EstimatedImage"));
    }
};

void FAccumulateTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = VK_CONTEXT().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            TIMING_MANAGER()->TimestampStart(Name, CommandBuffer, i);

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto RayTracingDescriptorSet = VK_CONTEXT().DescriptorSetManager->GetSet(Name, ACCUMULATE_PER_FRAME_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, VK_CONTEXT().DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &RayTracingDescriptorSet, 0, nullptr);

            uint32_t GroupSizeX = (Width % 8 == 0) ? (Width / 8) : (Width / 8) + 1;
            uint32_t GroupSizeY = (Height % 8 == 0) ? (Height / 8) : (Height / 8) + 1;

            vkCmdDispatch(CommandBuffer, GroupSizeX, GroupSizeY, 1);

            TIMING_MANAGER()->TimestampEnd(Name, CommandBuffer, i);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], Name);
    }
};
