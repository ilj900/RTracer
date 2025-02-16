#include "task_passthrough.h"

#include "mesh_component.h"

#include "vk_context.h"
#include "vk_shader_compiler.h"
#include "texture_manager.h"
#include "vk_debug.h"

FPassthroughTask::FPassthroughTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
    Name = "Passthrough pipeline";

    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, PASSTHROUGH_PER_FRAME_LAYOUT_INDEX, PASSTHROUGH_TEXTURE_SAMPLER_LAYOUT_INDEX,
                                              {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT});

	VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(FViewportResolutionPushConstants)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    QueueFlagsBits = VK_QUEUE_GRAPHICS_BIT;
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

void FPassthroughTask::Init(FCompileDefinitions* CompileDefinitions)
{
    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    Sampler = VK_CONTEXT()->CreateTextureSampler(VK_CONTEXT()->MipLevels, VK_FILTER_LINEAR);

    auto VertexShader = FShader("../src/shaders/passthrough.vert");
    auto FragmentShader = FShader("../src/shaders/passthrough.frag");

    GraphicsPipelineOptions.RegisterColorAttachment(0, Outputs["PassthroughOutput" + std::to_string(0)], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
    GraphicsPipelineOptions.SetPipelineLayout(DescriptorSetManager->GetPipelineLayout(Name));

    Pipeline = VK_CONTEXT()->CreateGraphicsPipeline(VertexShader(), FragmentShader(), Width, Height, GraphicsPipelineOptions);
    RenderPass = GraphicsPipelineOptions.RenderPass;

    PassthroughFramebuffers.resize(TotalSize);

    for (std::size_t i = 0; i < PassthroughFramebuffers.size(); ++i)
    {
        PassthroughFramebuffers[i] = VK_CONTEXT()->CreateFramebuffer(Width, Height, {Outputs["PassthroughOutput" + std::to_string(i)]}, RenderPass, "V_Passthrough_fb_" + std::to_string(i));
    }

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, PASSTHROUGH_PER_FRAME_LAYOUT_INDEX, TotalSize);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
}

void FPassthroughTask::UpdateDescriptorSets()
{
    for (int i = 0; i < TotalSize; ++i)
    {
        UpdateDescriptorSet(PASSTHROUGH_PER_FRAME_LAYOUT_INDEX, PASSTHROUGH_TEXTURE_SAMPLER_LAYOUT_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("EstimatedImage"), Sampler);
    }
}

void FPassthroughTask::RecordCommands()
{
    CommandBuffers.resize(TotalSize);

    for (int i = 0; i < TotalSize; ++i)
    {
        CommandBuffers[i] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
			ResetQueryPool(CommandBuffer, i);
			GPU_TIMER();

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
            auto PassthroughDescriptorSet = VK_CONTEXT()->DescriptorSetManager->GetSet(Name, PASSTHROUGH_PER_FRAME_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &PassthroughDescriptorSet, 0, nullptr);

			FViewportResolutionPushConstants PushConstants = {Width, Height};
			vkCmdPushConstants(CommandBuffer, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
				VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(FViewportResolutionPushConstants), &PushConstants);

            vkCmdDraw(CommandBuffer, 3, 1, 0, 0);
            vkCmdEndRenderPass(CommandBuffer);
        }, QueueFlagsBits);

        V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
    }
}
