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

FRaytraceTask::FRaytraceTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
    Name = "RayTracing pipeline";

    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_TLAS_LAYOUT_INDEX,
                                              {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,  VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_RAYS_DATA_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
	DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_PIXEL_INDEX_BUFFER,
											   {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_RAYGEN_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_RENDERABLE_BUFFER_INDEX,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_HIT_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR});
    DescriptorSetManager->AddDescriptorLayout(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_MATERIAL_INDEX_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR});

    FBuffer HitsBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(sizeof(FHit) * WidthIn * HeightIn, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "HitsBuffer");
    RESOURCE_ALLOCATOR()->RegisterBuffer(HitsBuffer, "HitsBuffer");

    FBuffer MaterialIndicesAOVBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(sizeof(uint32_t) * WidthIn * HeightIn, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "MaterialIndicesAOVBuffer");
    RESOURCE_ALLOCATOR()->RegisterBuffer(MaterialIndicesAOVBuffer, "MaterialIndicesAOVBuffer");

    DescriptorSetManager->CreateDescriptorSetLayout({}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

FRaytraceTask::~FRaytraceTask()
{
    RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer("HitsBuffer");
    RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer("MaterialIndicesAOVBuffer");
    GetResourceAllocator()->DestroyBuffer(SBTBuffer);
};

void FRaytraceTask::Init()
{
    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    auto RayGenerationShader = FShader("../../../src/shaders/raytrace.rgen");
    auto RayClosestHitShader = FShader("../../../src/shaders/raytrace.rchit");
    auto RayMissShader = FShader("../../../src/shaders/raytrace.rmiss");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

    Pipeline = VK_CONTEXT()->CreateRayTracingPipeline(RayGenerationShader(), RayMissShader(), RayClosestHitShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, RAYTRACE_LAYOUT_INDEX, TotalSize);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);

    auto RTProperties = VK_CONTEXT()->GetRTProperties();

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
    SBTBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(SBTSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "V::SBT_Buffer");

    auto SBTBufferAddress = VK_CONTEXT()->GetBufferDeviceAddressInfo(SBTBuffer);
    RGenRegion.deviceAddress = SBTBufferAddress;
    RMissRegion.deviceAddress = RGenRegion.deviceAddress + RGenRegion.size;
    RHitRegion.deviceAddress = RMissRegion.deviceAddress + RMissRegion.size;

    auto GetHandle = [&](int i)
    {
        return Handles.data() + i * HandleSize;
    };

    auto* SBTBufferPtr = reinterpret_cast<uint8_t*>(RESOURCE_ALLOCATOR()->Map(SBTBuffer));
    uint8_t* DataPtr{nullptr};
    uint32_t HandleIndex{0};

    DataPtr = SBTBufferPtr;
    memcpy(DataPtr, GetHandle(HandleIndex++), HandleSize);

    DataPtr = SBTBufferPtr + RGenRegion.size;
    memcpy(DataPtr, GetHandle(HandleIndex++), HandleSize);

    DataPtr = SBTBufferPtr + RGenRegion.size + RMissRegion.size;
    memcpy(DataPtr, GetHandle(HandleIndex++), HandleSize);

    RESOURCE_ALLOCATOR()->Unmap(SBTBuffer);
};

void FRaytraceTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < TotalSize; ++i)
    {
        VK_CONTEXT()->DescriptorSetManager->UpdateDescriptorSetInfo(Name, RAYTRACE_LAYOUT_INDEX, RAYTRACE_LAYOUT_INDEX, i, &ACCELERATION_STRUCTURE_SYSTEM()->TLAS.AccelerationStructure);
        UpdateDescriptorSet(RAYTRACE_LAYOUT_INDEX, RAYTRACE_RAYS_DATA_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("InitialRaysBuffer"));
		UpdateDescriptorSet(RAYTRACE_LAYOUT_INDEX, RAYTRACE_PIXEL_INDEX_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("PixelIndexBuffer"));
        UpdateDescriptorSet(RAYTRACE_LAYOUT_INDEX, RAYTRACE_RENDERABLE_BUFFER_INDEX, i, RENDERABLE_SYSTEM()->DeviceBuffer);
        UpdateDescriptorSet(RAYTRACE_LAYOUT_INDEX, RAYTRACE_HIT_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("HitsBuffer"));
        UpdateDescriptorSet(RAYTRACE_LAYOUT_INDEX, RAYTRACE_MATERIAL_INDEX_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("MaterialIndicesAOVBuffer"));
    }
};

void FRaytraceTask::RecordCommands()
{
    CommandBuffers.resize(TotalSize);
	auto ActiveRayCountBufferDeviceAddress = VK_CONTEXT()->GetBufferDeviceAddressInfo(RESOURCE_ALLOCATOR()->GetBuffer("ActiveRayCountBuffer"));

    for (std::size_t i = 0; i < TotalSize; ++i)
    {
        CommandBuffers[i] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
			ResetQueryPool(CommandBuffer, i);
			GPU_TIMER();

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, Pipeline);
            auto RayTracingDescriptorSet = VK_CONTEXT()->DescriptorSetManager->GetSet(Name, RAYTRACE_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &RayTracingDescriptorSet, 0, nullptr);
            V::vkCmdTraceRaysIndirectKHR(CommandBuffer, &RGenRegion, &RMissRegion, &RHitRegion, &RCallRegion, ActiveRayCountBufferDeviceAddress);
        }, QueueFlagsBits);

        V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
    }
};
