#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

#include "vk_shader_compiler.h"

#include "task_generate_initial_rays.h"

#include "utils.h"

#include "components/device_camera_component.h"
#include "systems/camera_system.h"

#include "common_defines.h"

FGenerateInitialRays::FGenerateInitialRays(uint32_t WidthIn, uint32_t HeightIn, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Generate rays pipeline";

    auto& DescriptorSetManager = VK_CONTEXT().DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, GENERATE_RAYS_LAYOUT_INDEX, CAMERA_RAYS_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, GENERATE_RAYS_LAYOUT_INDEX, CAMERA_POSITION_BUFFER,
                                              {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstants)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    FBuffer InitialRaysBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(sizeof(FRayData) * WidthIn * HeightIn, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Name);
    RESOURCE_ALLOCATOR()->RegisterBuffer(InitialRaysBuffer, "InitialRaysBuffer");

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
}

FGenerateInitialRays::~FGenerateInitialRays()
{
    RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer("InitialRaysBuffer");
};

void FGenerateInitialRays::Init()
{
    TIMING_MANAGER()->RegisterTiming(Name, NumberOfSimultaneousSubmits);

    auto& DescriptorSetManager = VK_CONTEXT().DescriptorSetManager;

    auto RayGenerationShader = FShader("../../../src/shaders/generate_rays.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);
    Pipeline = VK_CONTEXT().CreateComputePipeline(RayGenerationShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, GENERATE_RAYS_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FGenerateInitialRays::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        UpdateDescriptorSet(GENERATE_RAYS_LAYOUT_INDEX, CAMERA_RAYS_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("InitialRaysBuffer"));
        UpdateDescriptorSet(GENERATE_RAYS_LAYOUT_INDEX, CAMERA_POSITION_BUFFER, i, CAMERA_SYSTEM()->DeviceBuffer);
    }
};

void FGenerateInitialRays::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = VK_CONTEXT().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            TIMING_MANAGER()->TimestampStart(Name, CommandBuffer, i);

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto ComputeDescriptorSet = VK_CONTEXT().DescriptorSetManager->GetSet(Name, GENERATE_RAYS_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, VK_CONTEXT().DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &ComputeDescriptorSet, 0, nullptr);

            FPushConstants PushConstants = {Width, Height, 1.f / float(Width), 1.f / float(Height), Width * Height, 0};
            vkCmdPushConstants(CommandBuffer, VK_CONTEXT().DescriptorSetManager->GetPipelineLayout(Name), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstants), &PushConstants);

            vkCmdDispatch(CommandBuffer, CalculateGroupCount(Width * Height, BASIC_CHUNK_SIZE), 1, 1);

            TIMING_MANAGER()->TimestampEnd(Name, CommandBuffer, i);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], Name);
    }
};
