#include "task_passthrough.h"

#include "components/mesh_component.h"

#include "vk_context.h"

#include "vk_debug.h"

void FPassthroughTask::Init()
{
    auto& C = GetContext();

    C.DescriptorSetManager->AddDescriptorLayout(Name, PASSTHROUGH_PER_FRAME_LAYOUT_INDEX, PASSTHROUGH_TEXTURE_SAMPLER_LAYOUT_INDEX,
                                              {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT});

    C.DescriptorSetManager->CreateDescriptorSetLayout(Name);

    Sampler = C.CreateTextureSampler(C.MipLevels);

    auto VertexShader = C.CreateShaderFromFile("../shaders/passthrough_vert.spv");
    auto FragmentShader = C.CreateShaderFromFile("../shaders/passthrough_frag.spv");

    GraphicsPipelineOptions.RegisterColorAttachment(0, C.Swapchain->Images[0], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
    GraphicsPipelineOptions.SetPipelineLayout(C.DescriptorSetManager->GetPipelineLayout(Name));

    Pipeline = C.CreateGraphicsPipeline(VertexShader, FragmentShader, C.Swapchain->GetWidth(), C.Swapchain->GetHeight(), GraphicsPipelineOptions);
    RenderPass = GraphicsPipelineOptions.RenderPass;

    vkDestroyShaderModule(C.LogicalDevice, VertexShader, nullptr);
    vkDestroyShaderModule(C.LogicalDevice, FragmentShader, nullptr);

    PassthroughFramebuffers.resize(C.Swapchain->Size());
    for (std::size_t i = 0; i < PassthroughFramebuffers.size(); ++i)
    {
        PassthroughFramebuffers[i] = C.CreateFramebuffer({C.Swapchain->Images[i]}, RenderPass, "V_Passthrough_fb_" + std::to_string(i));
    }

    auto NumberOfSwapChainImages = C.Swapchain->Size();

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    C.DescriptorSetManager->ReserveDescriptorSet(Name, PASSTHROUGH_PER_FRAME_LAYOUT_INDEX, NumberOfSwapChainImages);

    C.DescriptorSetManager->ReserveDescriptorPool(Name);

    C.DescriptorSetManager->AllocateAllDescriptorSets(Name);

    for (int i = 0; i < C.Swapchain->Size(); ++i)
    {
        SignalSemaphores.push_back(C.CreateSemaphore());
    }
}

void FPassthroughTask::UpdateDescriptorSet()
{
    auto& C = GetContext();

    for (size_t i = 0; i < C.Swapchain->Size(); ++i)
    {
        VkDescriptorImageInfo ImageBufferInfo{};
        ImageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ImageBufferInfo.imageView = Inputs[0]->View;
        ImageBufferInfo.sampler = Sampler;
        C.DescriptorSetManager->UpdateDescriptorSetInfo(Name, PASSTHROUGH_PER_FRAME_LAYOUT_INDEX, PASSTHROUGH_TEXTURE_SAMPLER_LAYOUT_INDEX, i, ImageBufferInfo);
    }
}

void FPassthroughTask::RecordCommands()
{
    auto& C = GetContext();

    PassthroughCommandBuffers.resize(PassthroughFramebuffers.size());

    for (std::size_t i = 0; i < PassthroughCommandBuffers.size(); ++i)
    {
        PassthroughCommandBuffers[i] = C.CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            VkImageMemoryBarrier Barrier{};
            Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            Barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            Barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            Barrier.image = Inputs[0]->Image;
            Barrier.subresourceRange.baseMipLevel = 0;
            Barrier.subresourceRange.levelCount = 1;
            Barrier.subresourceRange.baseArrayLayer = 0;
            Barrier.subresourceRange.layerCount = 1;
            Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            Barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &Barrier);

            VkRenderPassBeginInfo RenderPassInfo{};
            RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            RenderPassInfo.renderPass = RenderPass;
            RenderPassInfo.framebuffer = PassthroughFramebuffers[i];
            RenderPassInfo.renderArea.offset = {0, 0};
            RenderPassInfo.renderArea.extent = C.Swapchain->GetExtent2D();

            std::vector<VkClearValue> ClearValues{1};
            ClearValues[0].color = {0.f, 0.f, 1.f, 1.f};
            RenderPassInfo.clearValueCount = static_cast<uint32_t>(ClearValues.size());
            RenderPassInfo.pClearValues = ClearValues.data();

            vkCmdBeginRenderPass(CommandBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);
            auto PassthroughDescriptorSet = C.DescriptorSetManager->GetSet(Name, PASSTHROUGH_PER_FRAME_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, C.DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &PassthroughDescriptorSet, 0, nullptr);

            vkCmdDraw(CommandBuffer, 3, 1, 0, 0);
            vkCmdEndRenderPass(CommandBuffer);
        });

        V::SetName(C.LogicalDevice, PassthroughCommandBuffers[i], "V_PassthroughCommandBuffers" + std::to_string(i));
    }
}

void FPassthroughTask::Cleanup()
{
    auto& C = GetContext();

    Inputs.clear();
    Outputs.clear();

    vkDestroySampler(C.LogicalDevice, Sampler, nullptr);

    for (auto Framebuffer : PassthroughFramebuffers)
    {
        vkDestroyFramebuffer(C.LogicalDevice, Framebuffer, nullptr);
    }

    for (auto& CommandBuffer : PassthroughCommandBuffers)
    {
        C.CommandBufferManager->FreeCommandBuffer(CommandBuffer);
    }

    vkDestroyRenderPass(C.LogicalDevice, RenderPass, nullptr);

    C.DescriptorSetManager->DestroyPipelineLayout(Name);
    vkDestroyPipeline(C.LogicalDevice, Pipeline, nullptr);

    C.DescriptorSetManager->Reset(Name);

    for (auto Semaphore : SignalSemaphores)
    {
        vkDestroySemaphore(C.LogicalDevice, Semaphore, nullptr);
    }
}

VkSemaphore FPassthroughTask::Submit(VkQueue Queue, VkSemaphore WaitSemaphore, int IterationIndex)
{
    auto& C = GetContext();

    VkSubmitInfo PassThroughSubmitInfo{};
    PassThroughSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore PassthroughWaitSemaphores[] = {WaitSemaphore};
    VkPipelineStageFlags PassthroughWaitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    PassThroughSubmitInfo.waitSemaphoreCount = 1;
    PassThroughSubmitInfo.pWaitSemaphores = PassthroughWaitSemaphores;
    PassThroughSubmitInfo.pWaitDstStageMask = PassthroughWaitStages;
    PassThroughSubmitInfo.commandBufferCount = 1;
    PassThroughSubmitInfo.pCommandBuffers = &PassthroughCommandBuffers[IterationIndex];

    VkSemaphore PassthroughSignalSemaphores[] = {SignalSemaphores[IterationIndex]};
    PassThroughSubmitInfo.signalSemaphoreCount = 1;
    PassThroughSubmitInfo.pSignalSemaphores = PassthroughSignalSemaphores;

    /// Submit rendering. When rendering finished, appropriate fence will be signalled
    if (vkQueueSubmit(Queue, 1, &PassThroughSubmitInfo, C.RenderingFinishedFences[IterationIndex]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }
    
    return SignalSemaphores[IterationIndex];
}

void FPassthroughTask::RegisterInput(int Index, ImagePtr Image)
{
    if (Inputs.size() <= Index)
    {
        Inputs.resize(Index + 1);
    }
    Inputs[Index] = Image;

}

void FPassthroughTask::RegisterOutput(int Index, ImagePtr Image)
{
    if (Outputs.size() <= Index)
    {
        Outputs.resize(Index + 1);
    }
    Outputs[Index] = Image;
}

ImagePtr FPassthroughTask::GetInput(int Index)
{
    if (Inputs.size() < Index)
    {
        return Inputs[Index];
    }
    throw std::runtime_error("Wrong input index.");
}

ImagePtr FPassthroughTask::GetOutput(int Index)
{
    if (Inputs.size() < Index)
    {
        return Inputs[Index];
    }
    throw std::runtime_error("Wrong output index.");
}