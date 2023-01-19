#include "task_render.h"

#include "components/mesh_component.h"
#include "components/device_transform_component.h"
#include "components/device_renderable_component.h"
#include "components/device_camera_component.h"
#include "systems/mesh_system.h"
#include "coordinator.h"

#include "vk_context.h"
#include "vk_debug.h"

FRenderTask::FRenderTask(FVulkanContext* Context, int NumberOfFrames, VkDevice LogicalDevice) :
    Context(Context), FramesCount(NumberOfFrames), LogicalDevice(LogicalDevice)
{
}

void FRenderTask::Init()
{
    Context->DescriptorSetManager->AddDescriptorLayout(Name, RENDER_PER_FRAME_LAYOUT_INDEX, TEXTURE_SAMPLER_LAYOUT_INDEX,
                                              {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT});
    Context->DescriptorSetManager->AddDescriptorLayout(Name, RENDER_PER_FRAME_LAYOUT_INDEX, CAMERA_LAYOUT_INDEX,
                                              {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT});

    Context->DescriptorSetManager->AddDescriptorLayout(Name, RENDER_PER_RENDERABLE_LAYOUT_INDEX, TRANSFORM_LAYOUT_INDEX,
                                              {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT});
    Context->DescriptorSetManager->AddDescriptorLayout(Name, RENDER_PER_RENDERABLE_LAYOUT_INDEX, RENDERABLE_LAYOUT_INDEX,
                                              {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT});

    Context->DescriptorSetManager->CreateDescriptorSetLayout(Name);

    uint32_t Width = Context->Swapchain->GetWidth();
    uint32_t Height = Context->Swapchain->GetHeight();

    /// Create Image and ImageView for Depth
    VkFormat DepthFormat = Context->FindDepthFormat();
    DepthImage = Context->CreateImage2D(Width, Height, false, Context->MSAASamples, DepthFormat, VK_IMAGE_TILING_OPTIMAL,
                                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                    VK_IMAGE_ASPECT_DEPTH_BIT, LogicalDevice, "V_DepthImage");


    DepthImage->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    Sampler = Context->CreateTextureSampler(Context->MipLevels);

    auto VertexShader = Context->CreateShaderFromFile("../shaders/triangle_vert.spv");
    auto FragmentShader = Context->CreateShaderFromFile("../shaders/triangle_frag.spv");

    auto AttributeDescriptions = FVertex::GetAttributeDescriptions();
    for (auto& Entry : AttributeDescriptions)
    {
        GraphicsPipelineOptions.AddVertexInputAttributeDescription(Entry);
    }
    GraphicsPipelineOptions.AddVertexInputBindingDescription(FVertex::GetBindingDescription());
    GraphicsPipelineOptions.RegisterDepthStencilAttachment(DepthImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
    GraphicsPipelineOptions.RegisterColorAttachment(0, Outputs[0], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
    GraphicsPipelineOptions.RegisterColorAttachment(1, Outputs[1], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
    GraphicsPipelineOptions.RegisterColorAttachment(2, Outputs[2], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
    GraphicsPipelineOptions.RegisterResolveAttachment(0, Outputs[3], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
    GraphicsPipelineOptions.SetPipelineLayout(Context->DescriptorSetManager->GetPipelineLayout(Name));
    GraphicsPipelineOptions.SetMSAA(Context->MSAASamples);

    Pipeline = Context->CreateGraphicsPipeline(VertexShader, FragmentShader, Width, Height, GraphicsPipelineOptions);
    RenderPass = GraphicsPipelineOptions.RenderPass;

    vkDestroyShaderModule(LogicalDevice, VertexShader, nullptr);
    vkDestroyShaderModule(LogicalDevice, FragmentShader, nullptr);

    RenderFramebuffers.resize(FramesCount);

    for (std::size_t i = 0; i < FramesCount; ++i) {
        RenderFramebuffers[i] = Context->CreateFramebuffer({Outputs[0], Outputs[1], Outputs[2], DepthImage, Outputs[3]}, GraphicsPipelineOptions.RenderPass, "V_Render_fb_" + std::to_string(i));
    }

    auto ModelsCount = ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMeshSystem>()->Size();

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    Context->DescriptorSetManager->ReserveDescriptorSet(Name, RENDER_PER_FRAME_LAYOUT_INDEX, FramesCount);
    Context->DescriptorSetManager->ReserveDescriptorSet(Name, RENDER_PER_RENDERABLE_LAYOUT_INDEX, FramesCount * ModelsCount);

    Context->DescriptorSetManager->ReserveDescriptorPool(Name);

    Context->DescriptorSetManager->AllocateAllDescriptorSets(Name);

    for (int i = 0; i < FramesCount; ++i)
    {
        SignalSemaphores.push_back(Context->CreateSemaphore());
    }
}

void FRenderTask::UpdateDescriptorSets()
{
    auto MeshSystem = ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMeshSystem>();

    for (size_t i = 0; i < FramesCount; ++i)
    {
        uint32_t j = 0;
        for (auto Mesh : *MeshSystem)
        {
            VkDescriptorBufferInfo TransformBufferInfo{};
            TransformBufferInfo.buffer = Context->DeviceTransformBuffers[i].Buffer;
            TransformBufferInfo.offset = sizeof(ECS::COMPONENTS::FDeviceTransformComponent) * j;
            TransformBufferInfo.range = sizeof(ECS::COMPONENTS::FDeviceTransformComponent);
            Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, RENDER_PER_RENDERABLE_LAYOUT_INDEX, TRANSFORM_LAYOUT_INDEX, j * FramesCount + i, TransformBufferInfo);

            VkDescriptorBufferInfo RenderableBufferInfo{};
            RenderableBufferInfo.buffer = Context->DeviceRenderableBuffers[i].Buffer;
            RenderableBufferInfo.offset = sizeof(ECS::COMPONENTS::FDeviceRenderableComponent) * j;
            RenderableBufferInfo.range = sizeof(ECS::COMPONENTS::FDeviceRenderableComponent);
            Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, RENDER_PER_RENDERABLE_LAYOUT_INDEX, RENDERABLE_LAYOUT_INDEX, j * FramesCount + i, RenderableBufferInfo);

            ++j;
        }
    }

    for (size_t i = 0; i < FramesCount; ++i)
    {

        VkDescriptorBufferInfo CameraBufferInfo{};
        CameraBufferInfo.buffer = Context->DeviceCameraBuffers[i].Buffer;
        CameraBufferInfo.offset = 0;
        CameraBufferInfo.range = sizeof(ECS::COMPONENTS::FDeviceCameraComponent);
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, RENDER_PER_FRAME_LAYOUT_INDEX, CAMERA_LAYOUT_INDEX, i, CameraBufferInfo);

        VkDescriptorImageInfo ImageBufferInfo{};
        ImageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ImageBufferInfo.imageView = Context->TextureImage->View;
        ImageBufferInfo.sampler = Sampler;
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, RENDER_PER_FRAME_LAYOUT_INDEX, TEXTURE_SAMPLER_LAYOUT_INDEX, i, ImageBufferInfo);
    }
}

void FRenderTask::RecordCommands()
{
    GraphicsCommandBuffers.resize(RenderFramebuffers.size());

    for (std::size_t i = 0; i < GraphicsCommandBuffers.size(); ++i)
    {
        GraphicsCommandBuffers[i] = Context->CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            VkRenderPassBeginInfo RenderPassInfo{};
            RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            RenderPassInfo.renderPass = GraphicsPipelineOptions.RenderPass;
            RenderPassInfo.framebuffer = RenderFramebuffers[i];
            RenderPassInfo.renderArea.offset = {0, 0};
            RenderPassInfo.renderArea.extent = Context->Swapchain->GetExtent2D();

            std::vector<VkClearValue> ClearValues{7};
            ClearValues[0].color = {0.f, 0.f, 0.f, 1.f};
            ClearValues[1].color = {0.0f, 0.f, 0.f, 1.f};
            ClearValues[2].color = {0, 0, 0, 0};
            ClearValues[3].depthStencil = {1.f, 0};
            ClearValues[4].color = {0.f, 0.f, 0.f, 0.f};
            RenderPassInfo.clearValueCount = static_cast<uint32_t>(ClearValues.size());
            RenderPassInfo.pClearValues = ClearValues.data();

            vkCmdBeginRenderPass(CommandBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);

            auto PerFrameDescriptorSet = Context->DescriptorSetManager->GetSet(Name, RENDER_PER_FRAME_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Context->DescriptorSetManager->GetPipelineLayout(Name), 0, 1, &PerFrameDescriptorSet, 0,
                                    nullptr);
            auto& Coordinator = ECS::GetCoordinator();
            auto MeshSystem = Coordinator.GetSystem<ECS::SYSTEMS::FMeshSystem>();

            uint32_t j = 0;
            for (auto Entity : *MeshSystem)
            {
                auto PerRenderableDescriptorSet = Context->DescriptorSetManager->GetSet(Name, RENDER_PER_RENDERABLE_LAYOUT_INDEX, j * FramesCount + i);
                vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Context->DescriptorSetManager->GetPipelineLayout(Name), 1, 1,
                                        &PerRenderableDescriptorSet, 0, nullptr);

                MeshSystem->Bind(Entity, CommandBuffer);
                MeshSystem->Draw(Entity, CommandBuffer);
                ++j;
            }
            vkCmdEndRenderPass(CommandBuffer);
        });

        V::SetName(LogicalDevice, GraphicsCommandBuffers[i], "V_GraphicsCommandBuffers" + std::to_string(i));
    }
}

void FRenderTask::Cleanup()
{
    DepthImage = nullptr;

    Inputs.clear();
    Outputs.clear();

    vkDestroySampler(LogicalDevice, Sampler, nullptr);

    for (auto Framebuffer : RenderFramebuffers)
    {
        vkDestroyFramebuffer(LogicalDevice, Framebuffer, nullptr);
    }

    for (auto& CommandBuffer : GraphicsCommandBuffers)
    {
        Context->CommandBufferManager->FreeCommandBuffer(CommandBuffer);
    }

    vkDestroyRenderPass(LogicalDevice, RenderPass, nullptr);

    Context->DescriptorSetManager->DestroyPipelineLayout(Name);
    vkDestroyPipeline(LogicalDevice, Pipeline, nullptr);

    Context->DescriptorSetManager->Reset(Name);

    for (auto Semaphore : SignalSemaphores)
    {
        vkDestroySemaphore(LogicalDevice, Semaphore, nullptr);
    }
}

VkSemaphore FRenderTask::Submit(VkQueue Queue, VkSemaphore WaitSemaphore, int IterationIndex)
{
    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore WaitSemaphores[] = {WaitSemaphore};
    VkPipelineStageFlags WaitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    SubmitInfo.waitSemaphoreCount = 1;
    SubmitInfo.pWaitSemaphores = WaitSemaphores;
    SubmitInfo.pWaitDstStageMask = WaitStages;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &GraphicsCommandBuffers[IterationIndex];

    VkSemaphore Semaphores[] = {SignalSemaphores[IterationIndex]};
    SubmitInfo.signalSemaphoreCount = 1;
    SubmitInfo.pSignalSemaphores = Semaphores;

    /// Submit rendering. When rendering finished, appropriate fence will be signalled
    if (vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    return SignalSemaphores[IterationIndex];
}

void FRenderTask::RegisterInput(int Index, ImagePtr Image)
{
    if (Inputs.size() <= Index)
    {
        Inputs.resize(Index + 1);
    }
    Inputs[Index] = Image;

}

void FRenderTask::RegisterOutput(int Index, ImagePtr Image)
{
    if (Outputs.size() <= Index)
    {
        Outputs.resize(Index + 1);
    }
    Outputs[Index] = Image;
}

ImagePtr FRenderTask::GetInput(int Index)
{
    if (Inputs.size() > Index)
    {
        return Inputs[Index];
    }
    throw std::runtime_error("Wrong input index.");
}

ImagePtr FRenderTask::GetOutput(int Index)
{
    if (Outputs.size() > Index)
    {
        return Outputs[Index];
    }
    throw std::runtime_error("Wrong output index.");
}