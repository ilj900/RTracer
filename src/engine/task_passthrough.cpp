#include "task_passthrough.h"

#include "components/mesh_component.h"

#include "vk_context.h"
#include "vk_shader_compiler.h"
#include "vk_debug.h"

#include "common_defines.h"

#include <iostream>
#include <iomanip>

FPassthroughTask::FPassthroughTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, Context, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Passthrough pipeline";

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, PASSTHROUGH_PER_FRAME_LAYOUT_INDEX, PASSTHROUGH_TEXTURE_SAMPLER_LAYOUT_INDEX,
                                              {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT});

    DescriptorSetManager->CreateDescriptorSetLayout({}, Name);
}

FPassthroughTask::~FPassthroughTask()
{
    FreeSyncObjects();
}

void FPassthroughTask::Init()
{
    FExecutableTask::Init();

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    Sampler = Context->CreateTextureSampler(Context->MipLevels);

    auto VertexShader = FShader("../../../src/shaders/passthrough.vert");
    auto FragmentShader = FShader("../../../src/shaders/passthrough.frag");

    GraphicsPipelineOptions.RegisterColorAttachment(0, Outputs[0], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
    GraphicsPipelineOptions.SetPipelineLayout(DescriptorSetManager->GetPipelineLayout(Name));

    Pipeline = Context->CreateGraphicsPipeline(VertexShader(), FragmentShader(), Width, Height, GraphicsPipelineOptions);
    RenderPass = GraphicsPipelineOptions.RenderPass;

    PassthroughFramebuffers.resize(NumberOfSimultaneousSubmits);
    for (std::size_t i = 0; i < PassthroughFramebuffers.size(); ++i)
    {
        PassthroughFramebuffers[i] = Context->CreateFramebuffer(Width, Height, {Outputs[i]}, RenderPass, "V_Passthrough_fb_" + std::to_string(i));
    }

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, PASSTHROUGH_PER_FRAME_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);

    CreateSyncObjects();
}

void FPassthroughTask::UpdateDescriptorSets()
{
    for (int i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        UpdateDescriptorSet(PASSTHROUGH_PER_FRAME_LAYOUT_INDEX, PASSTHROUGH_TEXTURE_SAMPLER_LAYOUT_INDEX, i, Inputs[0], Sampler);
    }
}

void FPassthroughTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (int i = 0; i < CommandBuffers.size(); ++i)
    {
        CommandBuffers[i] = Context->CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            vkCmdResetQueryPool(CommandBuffer, QueryPool, i * 2, 2);
            vkCmdWriteTimestamp(CommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, QueryPool, i * 2);

            VkRenderPassBeginInfo RenderPassInfo{};
            RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            RenderPassInfo.renderPass = RenderPass;
            RenderPassInfo.framebuffer = PassthroughFramebuffers[i];
            RenderPassInfo.renderArea.offset = {0, 0};
            /// TODO: find a better way to pass extent
            RenderPassInfo.renderArea.extent = {uint32_t(Width), uint32_t(Height)};

            std::vector<VkClearValue> ClearValues{1};
            ClearValues[0].color = {0.f, 0.f, 1.f, 1.f};
            RenderPassInfo.clearValueCount = static_cast<uint32_t>(ClearValues.size());
            RenderPassInfo.pClearValues = ClearValues.data();

            vkCmdBeginRenderPass(CommandBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);
            auto PassthroughDescriptorSet = Context->DescriptorSetManager->GetSet(Name, PASSTHROUGH_PER_FRAME_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Context->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &PassthroughDescriptorSet, 0, nullptr);

            vkCmdDraw(CommandBuffer, 3, 1, 0, 0);
            vkCmdEndRenderPass(CommandBuffer);

            vkCmdWriteTimestamp(CommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, QueryPool, i * 2 + 1);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], "V_PassthroughCommandBuffers" + std::to_string(i));
    }
}

void FPassthroughTask::Cleanup()
{
    Inputs.clear();
    Outputs.clear();

    vkDestroySampler(LogicalDevice, Sampler, nullptr);

    for (auto Framebuffer : PassthroughFramebuffers)
    {
        vkDestroyFramebuffer(LogicalDevice, Framebuffer, nullptr);
    }

    for (auto& CommandBuffer : CommandBuffers)
    {
        Context->CommandBufferManager->FreeCommandBuffer(CommandBuffer);
    }

    vkDestroyRenderPass(LogicalDevice, RenderPass, nullptr);

    Context->DescriptorSetManager->DestroyPipelineLayout(Name);
    vkDestroyPipeline(LogicalDevice, Pipeline, nullptr);

    Context->DescriptorSetManager->Reset(Name);
}

VkSemaphore FPassthroughTask::Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex)
{
    auto Result = FExecutableTask::Submit(Queue, WaitSemaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, WaitFence, SignalFence, IterationIndex);

    static std::vector<uint64_t> TimeStamps(NumberOfSimultaneousSubmits * 2);
    vkGetQueryPoolResults(LogicalDevice, QueryPool, IterationIndex * 2, 2, sizeof(uint64_t) * 2, TimeStamps.data() + IterationIndex * 2, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    uint64_t Delta = (TimeStamps[IterationIndex * 2 + 1] - TimeStamps[IterationIndex * 2]);
    std::cout << std::setprecision(2) << Name << " delta in ms:" << (float(Delta) / 1000000.f) << std::endl;
    return Result;
}