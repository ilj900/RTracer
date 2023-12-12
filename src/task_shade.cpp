#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "common_structures.h"

#include "systems/material_system.h"
#include "systems/renderable_system.h"
#include "systems/transform_system.h"
#include "systems/light_system.h"

#include "vk_shader_compiler.h"

#include "texture_manager.h"

#include "task_shade.h"

FShadeTask::FShadeTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, Context, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Shade pipeline";

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_OUTPUT_IMAGE_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_MATERIAL_BUFFER_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_RENDERABLE_BUFFER_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_HITS_BUFFER_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_TRANSFORM_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_RAYS_BUFFER_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_IBL_IMAGE_INDEX,
                                              {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_TEXTURE_SAMPLER,
                                              {VK_DESCRIPTOR_TYPE_SAMPLER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_TEXTURE_ARRAY,
                                              {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT, MAX_TEXTURES});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_LIGHTS_BUFFER_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstants)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    CreateSyncObjects();
}

FShadeTask::~FShadeTask()
{
    vkDestroySampler(LogicalDevice, IBLImageSampler, nullptr);
    vkDestroySampler(LogicalDevice, MaterialTextureSampler, nullptr);

    FreeSyncObjects();
}

void FShadeTask::Init()
{
    auto& DescriptorSetManager = Context->DescriptorSetManager;

    auto ShadeShader = FShader("../shaders/shade.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

    Pipeline = Context->CreateComputePipeline(ShadeShader(), PipelineLayout);

    IBLImageSampler = Context->CreateTextureSampler(VK_SAMPLE_COUNT_1_BIT);
    MaterialTextureSampler = Context->CreateTextureSampler(VK_SAMPLE_COUNT_1_BIT);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, COMPUTE_SHADE_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FShadeTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_OUTPUT_IMAGE_INDEX, i, Inputs[0]);
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_MATERIAL_BUFFER_INDEX, i, MATERIAL_SYSTEM()->DeviceBuffer);
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_RENDERABLE_BUFFER_INDEX, i, RENDERABLE_SYSTEM()->DeviceBuffer);
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_HITS_BUFFER_INDEX, i, Context->ResourceAllocator->GetBuffer("HitsBuffer"));
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_TRANSFORM_INDEX, i, TRANSFORM_SYSTEM()->DeviceBuffer);
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_RAYS_BUFFER_INDEX, i, Context->ResourceAllocator->GetBuffer("InitialRaysBuffer"));
        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_IBL_IMAGE_INDEX, i, Inputs[1], IBLImageSampler);

        VkDescriptorImageInfo MaterialTextureSamplerInfo{};
        MaterialTextureSamplerInfo.sampler = MaterialTextureSampler;
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_TEXTURE_SAMPLER, i, &MaterialTextureSamplerInfo);

        auto TextureSampler = GetTextureManager()->GetDescriptorImageInfos();
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_TEXTURE_ARRAY, i, TextureSampler);

        UpdateDescriptorSet(COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_LIGHTS_BUFFER_INDEX, i, LIGHT_SYSTEM()->DeviceBuffer);
    }
};

void FShadeTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = GetContext().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto RayTracingDescriptorSet = Context->DescriptorSetManager->GetSet(Name, COMPUTE_SHADE_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Context->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &RayTracingDescriptorSet, 0, nullptr);

            FPushConstants PushConstants = {Width, Height};
            vkCmdPushConstants(CommandBuffer, Context->DescriptorSetManager->GetPipelineLayout(Name), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstants), &PushConstants);

            int GroupSizeX = (Width % 8 == 0) ? (Width / 8) : (Width / 8) + 1;
            int GroupSizeY = (Height % 8 == 0) ? (Height / 8) : (Height / 8) + 1;

            vkCmdDispatch(CommandBuffer, GroupSizeX, GroupSizeY, 1);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], "V::Shade_Command_Buffer");
    }
};

void FShadeTask::Cleanup()
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

VkSemaphore FShadeTask::Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex)
{
    VkSemaphore WaitSemaphores[] = {WaitSemaphore};
    VkPipelineStageFlags WaitStages[] = {VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT};
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

    /// Submit computing. When computing finished, appropriate fence will be signalled
    if (vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    return SignalSemaphores[IterationIndex];
};
