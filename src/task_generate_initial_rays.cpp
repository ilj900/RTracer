#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

#include "vk_shader_compiler.h"

#include "task_generate_initial_rays.h"

#include "components/device_camera_component.h"
#include "systems/camera_system.h"
#include "common_structures.h"

FGenerateInitialRays::FGenerateInitialRays(int WidthIn, int HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, Context, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Generate rays pipeline";

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, GENERATE_RAYS_LAYOUT_INDEX, CAMERA_RAYS_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, GENERATE_RAYS_LAYOUT_INDEX, CAMERA_POSITION_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    DescriptorSetManager->CreateDescriptorSetLayout(Name);

    FBuffer InitialRaysBuffer = Context->ResourceAllocator->CreateBuffer(sizeof(FRayData) * WidthIn * HeightIn, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Name);
    Context->ResourceAllocator->RegisterBuffer(InitialRaysBuffer, "InitialRaysBuffer");

    CreateSyncObjects();
}

FGenerateInitialRays::~FGenerateInitialRays()
{
    FreeSyncObjects();
    Context->ResourceAllocator->UnregisterAndDestroyBuffer("InitialRaysBuffer");
}

void FGenerateInitialRays::Init()
{
    auto& DescriptorSetManager = Context->DescriptorSetManager;

    auto RayGenerationShader = FShader("../shaders/generate_rays.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);
    Pipeline = Context->CreateComputePipeline(RayGenerationShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, GENERATE_RAYS_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);

};

void FGenerateInitialRays::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        auto InitialRaysBuffer = Context->ResourceAllocator->GetBuffer("InitialRaysBuffer");
        VkDescriptorBufferInfo InitialRaysBufferInfo{};
        InitialRaysBufferInfo.buffer = InitialRaysBuffer.Buffer;
        InitialRaysBufferInfo.offset = 0;
        InitialRaysBufferInfo.range = InitialRaysBuffer.BufferSize;
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, GENERATE_RAYS_LAYOUT_INDEX, CAMERA_RAYS_BUFFER, i, InitialRaysBufferInfo);

        VkDescriptorBufferInfo CameraBufferInfo{};
        CameraBufferInfo.buffer = CAMERA_SYSTEM()->DeviceBuffer.Buffer;
        CameraBufferInfo.offset = 0;
        CameraBufferInfo.range = sizeof(ECS::COMPONENTS::FDeviceCameraComponent);
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, GENERATE_RAYS_LAYOUT_INDEX, CAMERA_POSITION_BUFFER, i, CameraBufferInfo);
    }
};

void FGenerateInitialRays::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = GetContext().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto ComputeDescriptorSet = Context->DescriptorSetManager->GetSet(Name, GENERATE_RAYS_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Context->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &ComputeDescriptorSet, 0, nullptr);
            vkCmdDispatch(CommandBuffer, Width * Height / 256, 1, 1);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], "V::RayTracing_Command_Buffer");
    }
};

void FGenerateInitialRays::Cleanup()
{
    Inputs.clear();
    Outputs.clear();

    for (auto& CommandBuffer : CommandBuffers)
    {
        Context->CommandBufferManager->FreeCommandBuffer(CommandBuffer);
    }

    Context->DescriptorSetManager->DestroyPipelineLayout(Name);
    vkDestroyPipeline(LogicalDevice, Pipeline, nullptr);

    Context->DescriptorSetManager->Reset(Name);
};

VkSemaphore FGenerateInitialRays::Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex)
{
    VkSemaphore WaitSemaphores[] = {WaitSemaphore};
    VkPipelineStageFlags WaitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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
    if (vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    return SignalSemaphores[IterationIndex];
};