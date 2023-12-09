#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"

#include "systems/material_system.h"
#include "systems/renderable_system.h"
#include "systems/transform_system.h"

#include "vk_shader_compiler.h"

#include "texture_manager.h"

#include "task_shade.h"

FShadeTask::FShadeTask(int WidthIn, int HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
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

    DescriptorSetManager->CreateDescriptorSetLayout(Name);

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
        VkDescriptorImageInfo InputImageInfo{};
        InputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        InputImageInfo.imageView = Inputs[0]->View;
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, COMPUTE_SHADE_LAYOUT_INDEX, COMPUTE_SHADE_OUTPUT_IMAGE_INDEX, i, &InputImageInfo);

        VkDescriptorBufferInfo MaterialDescriptorBufferInfo{};
        MaterialDescriptorBufferInfo.buffer = MATERIAL_SYSTEM()->DeviceBuffer.Buffer;
        MaterialDescriptorBufferInfo.offset = 0;
        MaterialDescriptorBufferInfo.range = MATERIAL_SYSTEM()->GetTotalSize();
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_MATERIAL_BUFFER_INDEX, i, &MaterialDescriptorBufferInfo);

        VkDescriptorBufferInfo RenderableDescriptorBufferInfo{};
        RenderableDescriptorBufferInfo.buffer = RENDERABLE_SYSTEM()->DeviceBuffer.Buffer;
        RenderableDescriptorBufferInfo.offset = 0;
        RenderableDescriptorBufferInfo.range = RENDERABLE_SYSTEM()->GetTotalSize();
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_RENDERABLE_BUFFER_INDEX, i, &RenderableDescriptorBufferInfo);

        auto HitsBuffer = Context->ResourceAllocator->GetBuffer("HitsBuffer");
        VkDescriptorBufferInfo HitsBufferInfo{};
        HitsBufferInfo.buffer = HitsBuffer.Buffer;
        HitsBufferInfo.offset = 0;
        HitsBufferInfo.range = HitsBuffer.BufferSize;
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_HITS_BUFFER_INDEX, i, &HitsBufferInfo);

        VkDescriptorBufferInfo TransformDescriptorBufferInfo{};
        TransformDescriptorBufferInfo.buffer = TRANSFORM_SYSTEM()->DeviceBuffer.Buffer;
        TransformDescriptorBufferInfo.offset = 0;
        TransformDescriptorBufferInfo.range = TRANSFORM_SYSTEM()->GetTotalSize();
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_TRANSFORM_INDEX, i, &TransformDescriptorBufferInfo);

        auto RaysDataBuffer = Context->ResourceAllocator->GetBuffer("InitialRaysBuffer");
        VkDescriptorBufferInfo RaysDataBufferInfo{};
        RaysDataBufferInfo.buffer = RaysDataBuffer.Buffer;
        RaysDataBufferInfo.offset = 0;
        RaysDataBufferInfo.range = RaysDataBuffer.BufferSize;
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_RAYS_BUFFER_INDEX, i, &RaysDataBufferInfo);

        VkDescriptorImageInfo IBLSamplerImage{};
        IBLSamplerImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        IBLSamplerImage.imageView = Inputs[1]->View;
        IBLSamplerImage.sampler = IBLImageSampler;
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_IBL_IMAGE_INDEX, i, &IBLSamplerImage);

        VkDescriptorImageInfo MaterialTextureSamplerInfo{};
        MaterialTextureSamplerInfo.sampler = MaterialTextureSampler;
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_TEXTURE_SAMPLER, i, &MaterialTextureSamplerInfo);

        auto TextureSampler = GetTextureManager()->GetDescriptorImageInfos();
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, COMPUTE_SHADE_LAYOUT_INDEX, RAYTRACE_SHADE_TEXTURE_ARRAY, i, TextureSampler);
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
