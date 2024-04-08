#include "task_passthrough.h"

#include "mesh_component.h"

#include "vk_context.h"
#include "vk_shader_compiler.h"
#include "texture_manager.h"
#include "vk_debug.h"

#include "common_defines.h"

FPassthroughTask::FPassthroughTask(uint32_t WidthIn, uint32_t HeightIn, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Passthrough pipeline";

    auto& DescriptorSetManager = VK_CONTEXT().DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, PASSTHROUGH_PER_FRAME_LAYOUT_INDEX, PASSTHROUGH_TEXTURE_SAMPLER_LAYOUT_INDEX,
                                              {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT});

    PipelineStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    DescriptorSetManager->CreateDescriptorSetLayout({}, Name);
}

FPassthroughTask::~FPassthroughTask()
{
    for (auto Framebuffer : PassthroughFramebuffers)
    {
        vkDestroyFramebuffer(LogicalDevice, Framebuffer, nullptr);
    }

    vkDestroySampler(LogicalDevice, Sampler, nullptr);
    vkDestroyRenderPass(LogicalDevice, RenderPass, nullptr);
}

void FPassthroughTask::Init()
{
    VK_CONTEXT().TimingManager->RegisterTiming(Name, NumberOfSimultaneousSubmits);

    auto& DescriptorSetManager = VK_CONTEXT().DescriptorSetManager;

    Sampler = VK_CONTEXT().CreateTextureSampler(VK_CONTEXT().MipLevels);

    auto VertexShader = FShader("../../../src/shaders/passthrough.vert");
    auto FragmentShader = FShader("../../../src/shaders/passthrough.frag");

    GraphicsPipelineOptions.RegisterColorAttachment(0, Outputs[0], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
    GraphicsPipelineOptions.SetPipelineLayout(DescriptorSetManager->GetPipelineLayout(Name));

    Pipeline = VK_CONTEXT().CreateGraphicsPipeline(VertexShader(), FragmentShader(), Width, Height, GraphicsPipelineOptions);
    RenderPass = GraphicsPipelineOptions.RenderPass;

    PassthroughFramebuffers.resize(NumberOfSimultaneousSubmits);
    for (std::size_t i = 0; i < PassthroughFramebuffers.size(); ++i)
    {
        PassthroughFramebuffers[i] = VK_CONTEXT().CreateFramebuffer(Width, Height, {Outputs[i]}, RenderPass, "V_Passthrough_fb_" + std::to_string(i));
    }

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, PASSTHROUGH_PER_FRAME_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
}

void FPassthroughTask::UpdateDescriptorSets()
{
    for (int i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        UpdateDescriptorSet(PASSTHROUGH_PER_FRAME_LAYOUT_INDEX, PASSTHROUGH_TEXTURE_SAMPLER_LAYOUT_INDEX, i, GetTextureManager()->GetFramebufferImage("EstimatedImage"), Sampler);
    }
}

void FPassthroughTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (int i = 0; i < CommandBuffers.size(); ++i)
    {
        CommandBuffers[i] = VK_CONTEXT().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            VK_CONTEXT().TimingManager->TimestampStart(Name, CommandBuffer, i);

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
            auto PassthroughDescriptorSet = VK_CONTEXT().DescriptorSetManager->GetSet(Name, PASSTHROUGH_PER_FRAME_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VK_CONTEXT().DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &PassthroughDescriptorSet, 0, nullptr);

            vkCmdDraw(CommandBuffer, 3, 1, 0, 0);
            vkCmdEndRenderPass(CommandBuffer);

            VK_CONTEXT().TimingManager->TimestampEnd(Name, CommandBuffer, i);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], Name);
    }
}
