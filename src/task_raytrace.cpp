#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

#include "components/mesh_component.h"
#include "components/device_mesh_component.h"
#include "components/device_transform_component.h"
#include "components/device_camera_component.h"
#include "systems/material_system.h"
#include "systems/camera_system.h"
#include "systems/mesh_system.h"
#include "systems/renderable_system.h"
#include "systems/light_system.h"
#include "vk_shader_compiler.h"

#include "task_raytrace.h"

#include "common_defines.h"

FRaytraceTask::FRaytraceTask(int WidthIn, int HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, Context, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "RayTracing pipeline";

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_TLAS_LAYOUT_INDEX,
                                              {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,  VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_FINAL_IMAGE_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_RAYS_DATA_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_RENDERABLE_BUFFER_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_MATERIAL_BUFFER_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_LIGHT_BUFFER_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_IBL_IMAGE_INDEX,
                                              {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_MISS_BIT_KHR});

    DescriptorSetManager->CreateDescriptorSetLayout(Name);

    CreateSyncObjects();
}

FRaytraceTask::~FRaytraceTask()
{
    FreeSyncObjects();

    Context->DestroyAccelerationStructure(TLAS);
    for (auto BLAS : BLASVector)
    {
        Context->DestroyAccelerationStructure(BLAS);
    }
}

void FRaytraceTask::Init()
{
    auto& DescriptorSetManager = Context->DescriptorSetManager;

    auto RayGenerationShader = FShader("../shaders/raytrace.rgen");
    auto RayClosestHitShader = FShader("../shaders/raytrace.rchit");
    auto RayMissShader = FShader("../shaders/raytrace.rmiss");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

    Pipeline = Context->CreateRayTracingPipeline(RayGenerationShader(), RayMissShader(), RayClosestHitShader(), Width, Height, PipelineLayout);

    auto MeshSystem = ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMeshSystem>();

    for(auto Mesh : *MeshSystem)
    {
        auto MeshComponent = ECS::GetCoordinator().GetComponent<ECS::COMPONENTS::FMeshComponent>(Mesh);
        auto DeviceMeshComponent = ECS::GetCoordinator().GetComponent<ECS::COMPONENTS::FDeviceMeshComponent>(Mesh);
        BLASVector.emplace_back(Context->GenerateBlas(MESH_SYSTEM()->VertexBuffer, MESH_SYSTEM()->IndexBuffer,
                                                      sizeof (FVertex), MeshComponent.Vertices.size(),
                                                      DeviceMeshComponent.VertexPtr, DeviceMeshComponent.IndexPtr));
    }

    std::vector<FMatrix4> Transforms;
    std::vector<uint32_t> BlasIndices;
    int i = 0;

    for(auto Mesh : *MeshSystem)
    {
        Transforms.push_back(ECS::GetCoordinator().GetComponent<ECS::COMPONENTS::FDeviceTransformComponent>(Mesh).ModelMatrix);
        BlasIndices.push_back(i++);
    }

    TLAS = Context->GenerateTlas(BLASVector, Transforms, BlasIndices);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, RAYTRACE_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);

    auto RTProperties = Context->GetRTProperties();

    uint32_t MissCount = 1;
    uint32_t HitCount = 1;
    auto HandleCount = 1 + MissCount + HitCount;
    uint32_t HandleSize = RTProperties.shaderGroupHandleSize;

    auto AlignUp = [](uint32_t X, uint32_t A)-> uint32_t
    {
        return (X + (A - 1)) & ~(A - 1);
    };

    uint32_t HandleSizeAligned = AlignUp(HandleSize, RTProperties.shaderGroupHandleAlignment);

    RGenRegion.stride = AlignUp(HandleSize, RTProperties.shaderGroupBaseAlignment);
    RGenRegion.size = RGenRegion.stride;

    RMissRegion.stride = HandleSizeAligned;
    RMissRegion.size = AlignUp(MissCount * HandleSizeAligned, RTProperties.shaderGroupBaseAlignment);

    RHitRegion.stride = HandleSizeAligned;
    RHitRegion.size = AlignUp(HitCount * HandleSizeAligned, RTProperties.shaderGroupBaseAlignment);

    uint32_t DataSize = HandleCount * HandleSize;
    std::vector<uint8_t> Handles(DataSize);

    auto Result = V::vkGetRayTracingShaderGroupHandlesKHR(LogicalDevice, Pipeline, 0, HandleCount, DataSize, Handles.data());
    assert(Result == VK_SUCCESS && "Failed to get handles for SBT");

    VkDeviceSize SBTSize = RGenRegion.size + RMissRegion.size + RHitRegion.size;
    SBTBuffer = Context->ResourceAllocator->CreateBuffer(SBTSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "V::SBT_Buffer");

    auto SBTBufferAddress = Context->GetBufferDeviceAddressInfo(SBTBuffer);
    RGenRegion.deviceAddress = SBTBufferAddress;
    RMissRegion.deviceAddress = RGenRegion.deviceAddress + RGenRegion.size;
    RHitRegion.deviceAddress = RMissRegion.deviceAddress + RMissRegion.size;

    auto GetHandle = [&](int i)
    {
        return Handles.data() + i * HandleSize;
    };

    auto* SBTBufferPtr = reinterpret_cast<uint8_t*>(Context->ResourceAllocator->Map(SBTBuffer));
    uint8_t* DataPtr{nullptr};
    uint32_t HandleIndex{0};

    DataPtr = SBTBufferPtr;
    memcpy(DataPtr, GetHandle(HandleIndex++), HandleSize);

    DataPtr = SBTBufferPtr + RGenRegion.size;
    memcpy(DataPtr, GetHandle(HandleIndex++), HandleSize);

    DataPtr = SBTBufferPtr + RGenRegion.size + RMissRegion.size;
    memcpy(DataPtr, GetHandle(HandleIndex++), HandleSize);

    Context->ResourceAllocator->Unmap(SBTBuffer);

    Sampler = Context->CreateTextureSampler(VK_SAMPLE_COUNT_1_BIT);
};

void FRaytraceTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_LAYOUT_INDEX, i, TLAS.AccelerationStructure);

        VkDescriptorImageInfo RTImageBufferInfo{};
        RTImageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        RTImageBufferInfo.imageView = Outputs[0]->View;
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_FINAL_IMAGE_INDEX, i, RTImageBufferInfo);

        auto InitialRaysBuffer = Context->ResourceAllocator->GetBuffer("InitialRaysBuffer");
        VkDescriptorBufferInfo RayDataBufferInfo{};
        RayDataBufferInfo.buffer = InitialRaysBuffer.Buffer;
        RayDataBufferInfo.offset = 0;
        RayDataBufferInfo.range = InitialRaysBuffer.BufferSize;
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_RAYS_DATA_BUFFER, i, RayDataBufferInfo);

        VkDescriptorBufferInfo RenderableBufferInfo{};
        RenderableBufferInfo.buffer = RENDERABLE_SYSTEM()->DeviceBuffer.Buffer;
        RenderableBufferInfo.offset = 0;
        RenderableBufferInfo.range = RENDERABLE_SYSTEM()->GetTotalSize();
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_RENDERABLE_BUFFER_INDEX, i, RenderableBufferInfo);

        VkDescriptorBufferInfo MaterialBufferInfo{};
        MaterialBufferInfo.buffer = MATERIAL_SYSTEM()->DeviceBuffer.Buffer;
        MaterialBufferInfo.offset = 0;
        MaterialBufferInfo.range = MATERIAL_SYSTEM()->GetTotalSize();
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_MATERIAL_BUFFER_INDEX, i, MaterialBufferInfo);

        VkDescriptorBufferInfo LightBufferInfo{};
        LightBufferInfo.buffer = LIGHT_SYSTEM()->DeviceBuffer.Buffer;
        LightBufferInfo.offset = 0;
        LightBufferInfo.range = LIGHT_SYSTEM()->GetTotalSize();
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_LIGHT_BUFFER_INDEX, i, LightBufferInfo);

        VkDescriptorImageInfo ImageBufferInfo{};
        ImageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ImageBufferInfo.imageView = Inputs[0]->View;
        ImageBufferInfo.sampler = Sampler;
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_IBL_IMAGE_INDEX, i, ImageBufferInfo);
    }
};

void FRaytraceTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = GetContext().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, Pipeline);
            auto RayTracingDescriptorSet = Context->DescriptorSetManager->GetSet(Name, RAYTRACE_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, Context->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &RayTracingDescriptorSet, 0, nullptr);
            V::vkCmdTraceRaysKHR(CommandBuffer, &RGenRegion, &RMissRegion, &RHitRegion, &RCallRegion, 1920, 1080, 1);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], "V::RayTracing_Command_Buffer");
    }
};

void FRaytraceTask::Cleanup()
{
    Inputs.clear();
    Outputs.clear();

    vkDestroySampler(LogicalDevice, Sampler, nullptr);

    for (auto& CommandBuffer : CommandBuffers)
    {
        Context->CommandBufferManager->FreeCommandBuffer(CommandBuffer);
    }

    Context->DescriptorSetManager->DestroyPipelineLayout(Name);
    vkDestroyPipeline(LogicalDevice, Pipeline, nullptr);

    Context->DescriptorSetManager->Reset(Name);

    GetResourceAllocator()->DestroyBuffer(SBTBuffer);
};

VkSemaphore FRaytraceTask::Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex)
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
