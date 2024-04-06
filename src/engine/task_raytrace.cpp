#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

#include "mesh_component.h"
#include "mesh_system.h"
#include "renderable_system.h"
#include "acceleration_structure_system.h"
#include "vk_shader_compiler.h"

#include "task_raytrace.h"
#include "texture_manager.h"

FRaytraceTask::FRaytraceTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, Context, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "RayTracing pipeline";

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_TLAS_LAYOUT_INDEX,
                                              {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,  VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_RAYS_DATA_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_RENDERABLE_BUFFER_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_HIT_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_MATERIAL_INDEX_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR});

    FBuffer HitsBuffer = Context->ResourceAllocator->CreateBuffer(sizeof(FHit) * WidthIn * HeightIn, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "HitsBuffer");
    Context->ResourceAllocator->RegisterBuffer(HitsBuffer, "HitsBuffer");

    FBuffer MaterialIndicesAOVBuffer = Context->ResourceAllocator->CreateBuffer(sizeof(uint32_t) * WidthIn * HeightIn, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "MaterialIndicesAOVBuffer");
    Context->ResourceAllocator->RegisterBuffer(MaterialIndicesAOVBuffer, "MaterialIndicesAOVBuffer");

    DescriptorSetManager->CreateDescriptorSetLayout({}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;

    CreateSyncObjects();
}

FRaytraceTask::~FRaytraceTask()
{
    FreeSyncObjects();
    Context->ResourceAllocator->UnregisterAndDestroyBuffer("HitsBuffer");
    Context->ResourceAllocator->UnregisterAndDestroyBuffer("MaterialIndicesAOVBuffer");
}

void FRaytraceTask::Init()
{
    Context->TimingManager->RegisterTiming(Name, NumberOfSimultaneousSubmits);

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    auto RayGenerationShader = FShader("../../../src/shaders/raytrace.rgen");
    auto RayClosestHitShader = FShader("../../../src/shaders/raytrace.rchit");
    auto RayMissShader = FShader("../../../src/shaders/raytrace.rmiss");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

    Pipeline = Context->CreateRayTracingPipeline(RayGenerationShader(), RayMissShader(), RayClosestHitShader(), Width, Height, PipelineLayout);

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
};

void FRaytraceTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        Context->DescriptorSetManager->UpdateDescriptorSetInfo(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_LAYOUT_INDEX, i, &ACCELERATION_STRUCTURE_SYSTEM()->TLAS.AccelerationStructure);
        UpdateDescriptorSet(RAYTRACE_LAYOUT_INDEX, RAYTRACE_RAYS_DATA_BUFFER, i, Context->ResourceAllocator->GetBuffer("InitialRaysBuffer"));
        UpdateDescriptorSet(RAYTRACE_LAYOUT_INDEX, RAYTRACE_RENDERABLE_BUFFER_INDEX, i, RENDERABLE_SYSTEM()->DeviceBuffer);
        UpdateDescriptorSet(RAYTRACE_LAYOUT_INDEX, RAYTRACE_HIT_BUFFER, i, Context->ResourceAllocator->GetBuffer("HitsBuffer"));
        UpdateDescriptorSet(RAYTRACE_LAYOUT_INDEX, RAYTRACE_MATERIAL_INDEX_BUFFER, i, Context->ResourceAllocator->GetBuffer("MaterialIndicesAOVBuffer"));
    }
};

void FRaytraceTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = GetContext().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            Context->TimingManager->TimestampStart(Name, CommandBuffer, i);

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, Pipeline);
            auto RayTracingDescriptorSet = Context->DescriptorSetManager->GetSet(Name, RAYTRACE_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, Context->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &RayTracingDescriptorSet, 0, nullptr);
            V::vkCmdTraceRaysKHR(CommandBuffer, &RGenRegion, &RMissRegion, &RHitRegion, &RCallRegion, Width, Height, 1);

            Context->TimingManager->TimestampEnd(Name, CommandBuffer, i);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], "V::RayTracing_Command_Buffer");
    }
};

void FRaytraceTask::Cleanup()
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

    GetResourceAllocator()->DestroyBuffer(SBTBuffer);
};
