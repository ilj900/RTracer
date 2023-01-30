#include "task_render.h"

#include "components/mesh_component.h"
#include "components/device_transform_component.h"
#include "components/device_renderable_component.h"
#include "components/device_camera_component.h"
#include "systems/mesh_system.h"
#include "systems/camera_system.h"
#include "systems/transform_system.h"
#include "systems/renderable_system.h"
#include "coordinator.h"

#include "vk_context.h"
#include "vk_debug.h"

FRenderTask::FRenderTask(FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(Context, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Render pipeline";
}

FRenderTask::~FRenderTask()
{
    FreeSyncObjects();
}

void FRenderTask::Init()
{
    auto& DescriptorSetManager = Context->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, RENDER_PER_FRAME_LAYOUT_INDEX, TEXTURE_SAMPLER_LAYOUT_INDEX,
                                              {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, RENDER_PER_FRAME_LAYOUT_INDEX, CAMERA_LAYOUT_INDEX,
                                              {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT});

    DescriptorSetManager->AddDescriptorLayout(Name, RENDER_PER_RENDERABLE_LAYOUT_INDEX, TRANSFORM_LAYOUT_INDEX,
                                              {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, RENDER_PER_RENDERABLE_LAYOUT_INDEX, RENDERABLE_LAYOUT_INDEX,
                                              {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT});

    DescriptorSetManager->CreateDescriptorSetLayout(Name);

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
    GraphicsPipelineOptions.SetPipelineLayout(DescriptorSetManager->GetPipelineLayout(Name));
    GraphicsPipelineOptions.SetMSAA(Context->MSAASamples);

    Pipeline = Context->CreateGraphicsPipeline(VertexShader, FragmentShader, Width, Height, GraphicsPipelineOptions);
    RenderPass = GraphicsPipelineOptions.RenderPass;

    vkDestroyShaderModule(LogicalDevice, VertexShader, nullptr);
    vkDestroyShaderModule(LogicalDevice, FragmentShader, nullptr);

    RenderFramebuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i) {
        RenderFramebuffers[i] = Context->CreateFramebuffer({Outputs[0], Outputs[1], Outputs[2], DepthImage, Outputs[3]}, GraphicsPipelineOptions.RenderPass, "V_Render_fb_" + std::to_string(i));
    }

    auto ModelsCount = ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMeshSystem>()->Size();

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, RENDER_PER_FRAME_LAYOUT_INDEX, NumberOfSimultaneousSubmits);
    DescriptorSetManager->ReserveDescriptorSet(Name, RENDER_PER_RENDERABLE_LAYOUT_INDEX, NumberOfSimultaneousSubmits * ModelsCount);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);

    CreateSyncObjects();
}

void FRenderTask::UpdateDescriptorSets()
{
    auto& Coordinator = ECS::GetCoordinator();
    auto MeshSystem = Coordinator.GetSystem<ECS::SYSTEMS::FMeshSystem>();

    VkDeviceSize TransformBufferSize = Coordinator.Size<ECS::COMPONENTS::FDeviceTransformComponent>();
    VkDeviceSize CameraBufferSize = Coordinator.Size<ECS::COMPONENTS::FDeviceCameraComponent>();
    VkDeviceSize RenderableBufferSize = Coordinator.Size<ECS::COMPONENTS::FDeviceRenderableComponent>();

    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        uint32_t j = 0;
        for (auto Mesh : *MeshSystem)
        {
            VkDescriptorBufferInfo TransformBufferInfo{};
            TransformBufferInfo.buffer = TRANSFORM_SYSTEM()->DeviceTransformBuffer.Buffer;
            TransformBufferInfo.offset = TransformBufferSize * i + sizeof(ECS::COMPONENTS::FDeviceTransformComponent) * j;
            TransformBufferInfo.range = sizeof(ECS::COMPONENTS::FDeviceTransformComponent);
            Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, RENDER_PER_RENDERABLE_LAYOUT_INDEX, TRANSFORM_LAYOUT_INDEX, j * NumberOfSimultaneousSubmits + i, TransformBufferInfo);

            VkDescriptorBufferInfo RenderableBufferInfo{};
            RenderableBufferInfo.buffer = RENDERABLE_SYSTEM()->DeviceRenderableBuffer.Buffer;
            RenderableBufferInfo.offset = RenderableBufferSize * i + sizeof(ECS::COMPONENTS::FDeviceRenderableComponent) * j;
            RenderableBufferInfo.range = sizeof(ECS::COMPONENTS::FDeviceRenderableComponent);
            Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, RENDER_PER_RENDERABLE_LAYOUT_INDEX, RENDERABLE_LAYOUT_INDEX, j * NumberOfSimultaneousSubmits + i, RenderableBufferInfo);

            ++j;
        }
    }

    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {

        VkDescriptorBufferInfo CameraBufferInfo{};
        CameraBufferInfo.buffer = CAMERA_SYSTEM()->DeviceCameraBuffer.Buffer;
        CameraBufferInfo.offset = CameraBufferSize * i;
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
    CommandBuffers.resize(RenderFramebuffers.size());

    for (std::size_t i = 0; i < CommandBuffers.size(); ++i)
    {
        CommandBuffers[i] = Context->CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
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
                auto PerRenderableDescriptorSet = Context->DescriptorSetManager->GetSet(Name, RENDER_PER_RENDERABLE_LAYOUT_INDEX, j * NumberOfSimultaneousSubmits + i);
                vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Context->DescriptorSetManager->GetPipelineLayout(Name), 1, 1,
                                        &PerRenderableDescriptorSet, 0, nullptr);

                MeshSystem->Bind(Entity, CommandBuffer);
                MeshSystem->Draw(Entity, CommandBuffer);
                ++j;
            }
            vkCmdEndRenderPass(CommandBuffer);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], "V_GraphicsCommandBuffers" + std::to_string(i));
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

    for (auto& CommandBuffer : CommandBuffers)
    {
        Context->CommandBufferManager->FreeCommandBuffer(CommandBuffer);
    }

    vkDestroyRenderPass(LogicalDevice, RenderPass, nullptr);

    Context->DescriptorSetManager->DestroyPipelineLayout(Name);
    vkDestroyPipeline(LogicalDevice, Pipeline, nullptr);

    Context->DescriptorSetManager->Reset(Name);
}

VkSemaphore FRenderTask::Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex)
{
    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore WaitSemaphores[] = {WaitSemaphore};
    VkPipelineStageFlags WaitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    SubmitInfo.waitSemaphoreCount = 1;
    SubmitInfo.pWaitSemaphores = WaitSemaphores;
    SubmitInfo.pWaitDstStageMask = WaitStages;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CommandBuffers[IterationIndex];

    VkSemaphore Semaphores[] = {SignalSemaphores[IterationIndex]};
    SubmitInfo.signalSemaphoreCount = 1;
    SubmitInfo.pSignalSemaphores = Semaphores;

    if(WaitFence != VK_NULL_HANDLE)
    {
        vkWaitForFences(LogicalDevice, 1, &WaitFence, VK_TRUE, UINT64_MAX);
    }

    if (SignalFence != VK_NULL_HANDLE)
    {
        vkResetFences(LogicalDevice, 1, &SignalFence);
    }

    /// Submit rendering. When rendering finished, appropriate fence will be signalled
    if (vkQueueSubmit(Queue, 1, &SubmitInfo, SignalFence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    return SignalSemaphores[IterationIndex];
}