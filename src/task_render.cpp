#include "task_render.h"

#include "components/mesh_component.h"
#include "components/device_transform_component.h"
#include "components/device_renderable_component.h"
#include "components/device_camera_component.h"
#include "systems/mesh_system.h"
#include "coordinator.h"

#include "vk_context.h"
#include "vk_debug.h"

void FRenderTask::Init()
{
    auto& C = GetContext();

    C.DescriptorSetManager->AddDescriptorLayout(Name, RENDER_PER_FRAME_LAYOUT_INDEX, TEXTURE_SAMPLER_LAYOUT_INDEX,
                                              {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT});
    C.DescriptorSetManager->AddDescriptorLayout(Name, RENDER_PER_FRAME_LAYOUT_INDEX, CAMERA_LAYOUT_INDEX,
                                              {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT});

    C.DescriptorSetManager->AddDescriptorLayout(Name, RENDER_PER_RENDERABLE_LAYOUT_INDEX, TRANSFORM_LAYOUT_INDEX,
                                              {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT});
    C.DescriptorSetManager->AddDescriptorLayout(Name, RENDER_PER_RENDERABLE_LAYOUT_INDEX, RENDERABLE_LAYOUT_INDEX,
                                              {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT});

    C.DescriptorSetManager->CreateDescriptorSetLayout(Name);

    uint32_t Width = C.Swapchain->GetWidth();
    uint32_t Height = C.Swapchain->GetHeight();


    ColorImage = C.CreateImage2D(Width, Height, false, C.MSAASamples, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               VK_IMAGE_ASPECT_COLOR_BIT, C.LogicalDevice, "V_ColorImage");


    NormalsImage = C.CreateImage2D(Width, Height, false, C.MSAASamples, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 VK_IMAGE_ASPECT_COLOR_BIT, C.LogicalDevice, "V_NormalsImage");


    RenderableIndexImage = C.CreateImage2D(Width, Height, false, C.MSAASamples, VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL,
                                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                         VK_IMAGE_ASPECT_COLOR_BIT, C.LogicalDevice, "V_RenderableIndexImage");

    ResolvedColorImage = C.CreateImage2D(Width, Height, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                       VK_IMAGE_ASPECT_COLOR_BIT, C.LogicalDevice, "V_ResolvedColorImage");

    /// Create Image and ImageView for Depth
    VkFormat DepthFormat = C.FindDepthFormat();
    DepthImage = C.CreateImage2D(Width, Height, false, C.MSAASamples, DepthFormat, VK_IMAGE_TILING_OPTIMAL,
                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               VK_IMAGE_ASPECT_DEPTH_BIT, C.LogicalDevice, "V_DepthImage");


    DepthImage->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    auto VertexShader = C.CreateShaderFromFile("../shaders/triangle_vert.spv");
    auto FragmentShader = C.CreateShaderFromFile("../shaders/triangle_frag.spv");

    auto AttributeDescriptions = FVertex::GetAttributeDescriptions();
    for (auto& Entry : AttributeDescriptions)
    {
        GraphicsPipelineOptions.AddVertexInputAttributeDescription(Entry);
    }
    GraphicsPipelineOptions.AddVertexInputBindingDescription(FVertex::GetBindingDescription());
    GraphicsPipelineOptions.RegisterDepthStencilAttachment(DepthImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
    GraphicsPipelineOptions.RegisterColorAttachment(0, ColorImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
    GraphicsPipelineOptions.RegisterColorAttachment(1, NormalsImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
    GraphicsPipelineOptions.RegisterColorAttachment(2, RenderableIndexImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
    GraphicsPipelineOptions.RegisterResolveAttachment(0, ResolvedColorImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR);
    GraphicsPipelineOptions.SetPipelineLayout(C.DescriptorSetManager->GetPipelineLayout(Name));
    GraphicsPipelineOptions.SetMSAA(C.MSAASamples);

    Pipeline = C.CreateGraphicsPipeline(VertexShader, FragmentShader, C.Swapchain->GetWidth(), C.Swapchain->GetHeight(), GraphicsPipelineOptions);

    vkDestroyShaderModule(C.LogicalDevice, VertexShader, nullptr);
    vkDestroyShaderModule(C.LogicalDevice, FragmentShader, nullptr);

    RenderFramebuffers.resize(C.Swapchain->Size());

    for (std::size_t i = 0; i < C.Swapchain->Size(); ++i) {
        RenderFramebuffers[i] = C.CreateFramebuffer({ColorImage, NormalsImage, RenderableIndexImage, DepthImage, ResolvedColorImage}, GraphicsPipelineOptions.RenderPass, "V_Render_fb_" + std::to_string(i));
    }

    auto ModelsCount = ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMeshSystem>()->Size();
    auto NumberOfSwapChainImages = C.Swapchain->Size();

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    C.DescriptorSetManager->ReserveDescriptorSet(Name, RENDER_PER_FRAME_LAYOUT_INDEX, NumberOfSwapChainImages);
    C.DescriptorSetManager->ReserveDescriptorSet(Name, RENDER_PER_RENDERABLE_LAYOUT_INDEX, NumberOfSwapChainImages * ModelsCount);

    C.DescriptorSetManager->ReserveDescriptorPool(Name);

    C.DescriptorSetManager->AllocateAllDescriptorSets(Name);
}

void FRenderTask::UpdateDescriptorSets()
{
    auto MeshSystem = ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMeshSystem>();

    auto& C = GetContext();

    for (size_t i = 0; i < C.Swapchain->Size(); ++i)
    {
        uint32_t j = 0;
        for (auto Mesh : *MeshSystem)
        {
            VkDescriptorBufferInfo TransformBufferInfo{};
            TransformBufferInfo.buffer = C.DeviceTransformBuffers[i].Buffer;
            TransformBufferInfo.offset = sizeof(ECS::COMPONENTS::FDeviceTransformComponent) * j;
            TransformBufferInfo.range = sizeof(ECS::COMPONENTS::FDeviceTransformComponent);
            C.DescriptorSetManager->UpdateDescriptorSetInfo(Name, RENDER_PER_RENDERABLE_LAYOUT_INDEX, TRANSFORM_LAYOUT_INDEX, j * C.Swapchain->Size() + i, TransformBufferInfo);

            VkDescriptorBufferInfo RenderableBufferInfo{};
            RenderableBufferInfo.buffer = C.DeviceRenderableBuffers[i].Buffer;
            RenderableBufferInfo.offset = sizeof(ECS::COMPONENTS::FDeviceRenderableComponent) * j;
            RenderableBufferInfo.range = sizeof(ECS::COMPONENTS::FDeviceRenderableComponent);
            C.DescriptorSetManager->UpdateDescriptorSetInfo(Name, RENDER_PER_RENDERABLE_LAYOUT_INDEX, RENDERABLE_LAYOUT_INDEX, j * C.Swapchain->Size() + i, RenderableBufferInfo);

            ++j;
        }
    }

    for (size_t i = 0; i < C.Swapchain->Size(); ++i)
    {

        VkDescriptorBufferInfo CameraBufferInfo{};
        CameraBufferInfo.buffer = C.DeviceCameraBuffers[i].Buffer;
        CameraBufferInfo.offset = 0;
        CameraBufferInfo.range = sizeof(ECS::COMPONENTS::FDeviceCameraComponent);
        C.DescriptorSetManager->UpdateDescriptorSetInfo(Name, RENDER_PER_FRAME_LAYOUT_INDEX, CAMERA_LAYOUT_INDEX, i, CameraBufferInfo);

        VkDescriptorImageInfo ImageBufferInfo{};
        ImageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ImageBufferInfo.imageView = C.TextureImage->View;
        ImageBufferInfo.sampler = C.TextureSampler;
        C.DescriptorSetManager->UpdateDescriptorSetInfo(Name, RENDER_PER_FRAME_LAYOUT_INDEX, TEXTURE_SAMPLER_LAYOUT_INDEX, i, ImageBufferInfo);
    }
}

void FRenderTask::RecordCommands()
{
    auto& C = GetContext();

    GraphicsCommandBuffers.resize(RenderFramebuffers.size());

    for (std::size_t i = 0; i < GraphicsCommandBuffers.size(); ++i)
    {
        GraphicsCommandBuffers[i] = C.CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            VkRenderPassBeginInfo RenderPassInfo{};
            RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            RenderPassInfo.renderPass = GraphicsPipelineOptions.RenderPass;
            RenderPassInfo.framebuffer = RenderFramebuffers[i];
            RenderPassInfo.renderArea.offset = {0, 0};
            RenderPassInfo.renderArea.extent = C.Swapchain->GetExtent2D();

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

            auto PerFrameDescriptorSet = C.DescriptorSetManager->GetSet(Name, RENDER_PER_FRAME_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, C.DescriptorSetManager->GetPipelineLayout(Name), 0, 1, &PerFrameDescriptorSet, 0,
                                    nullptr);
            auto& Coordinator = ECS::GetCoordinator();
            auto MeshSystem = Coordinator.GetSystem<ECS::SYSTEMS::FMeshSystem>();

            uint32_t j = 0;
            for (auto Entity : *MeshSystem)
            {
                auto PerRenderableDescriptorSet = C.DescriptorSetManager->GetSet(Name, RENDER_PER_RENDERABLE_LAYOUT_INDEX, j * C.Swapchain->Size() + i);
                vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, C.DescriptorSetManager->GetPipelineLayout(Name), 1, 1,
                                        &PerRenderableDescriptorSet, 0, nullptr);

                MeshSystem->Bind(Entity, CommandBuffer);
                MeshSystem->Draw(Entity, CommandBuffer);
                ++j;
            }
            vkCmdEndRenderPass(CommandBuffer);
        });

        V::SetName(C.LogicalDevice, GraphicsCommandBuffers[i], "V_GraphicsCommandBuffers" + std::to_string(i));
    }
}

void FRenderTask::Cleanup()
{
    auto& C = GetContext();

    ColorImage = nullptr;
    ResolvedColorImage = nullptr;
    NormalsImage = nullptr;
    RenderableIndexImage = nullptr;
    DepthImage = nullptr;

    for (auto Framebuffer : RenderFramebuffers)
    {
        vkDestroyFramebuffer(C.LogicalDevice, Framebuffer, nullptr);
    }

    for (auto& CommandBuffer : GraphicsCommandBuffers)
    {
        C.CommandBufferManager->FreeCommandBuffer(CommandBuffer);
    }

    C.DescriptorSetManager->DestroyPipelineLayout(Name);
    vkDestroyPipeline(C.LogicalDevice, Pipeline, nullptr);

    vkDestroyRenderPass(C.LogicalDevice, RenderPass, nullptr);

    C.DescriptorSetManager->Reset(Name);
}