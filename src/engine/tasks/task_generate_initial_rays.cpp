#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"

#include "vk_shader_compiler.h"

#include "task_generate_initial_rays.h"

#include "utils.h"

#include "systems/camera_system.h"

#include "common_defines.h"
#include "common_structures.h"

FGenerateInitialRays::FGenerateInitialRays(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
    Name = "Generate rays pipeline";

    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, GENERATE_RAYS_LAYOUT_INDEX, CAMERA_RAYS_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, GENERATE_RAYS_LAYOUT_INDEX, CAMERA_POSITION_BUFFER,
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, GENERATE_RAYS_LAYOUT_INDEX, GENERATE_RAYS_PIXEL_INDEX_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, GENERATE_RAYS_LAYOUT_INDEX, GENERATE_RAYS_RENDER_ITERATION_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, GENERATE_RAYS_LAYOUT_INDEX, GENERATE_RAYS_THROUGHPUT_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, GENERATE_RAYS_LAYOUT_INDEX, GENERATE_RAYS_DEBUG_CMJ_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstants)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

	FBuffer ThroughputBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(4 * sizeof(float ) * Width * Height, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "ThroughputBuffer");
	RESOURCE_ALLOCATOR()->RegisterBuffer(ThroughputBuffer, "ThroughputBuffer");

    FBuffer InitialRaysBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(sizeof(FRayData) * WidthIn * HeightIn, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "InitialRaysBuffer");
    RESOURCE_ALLOCATOR()->RegisterBuffer(InitialRaysBuffer, "InitialRaysBuffer");

	FBuffer PixelIndexBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(sizeof(uint32_t) * WidthIn * HeightIn, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "PixelIndexBuffer");
	RESOURCE_ALLOCATOR()->RegisterBuffer(PixelIndexBuffer, "PixelIndexBuffer");

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

FGenerateInitialRays::~FGenerateInitialRays()
{
	RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer("ThroughputBuffer");
    RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer("InitialRaysBuffer");
	RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer("PixelIndexBuffer");
};

void FGenerateInitialRays::Init()
{
    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    auto RayGenerationShader = FShader("../../../src/shaders/generate_rays.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);
    Pipeline = VK_CONTEXT()->CreateComputePipeline(RayGenerationShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, GENERATE_RAYS_LAYOUT_INDEX, TotalSize);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FGenerateInitialRays::UpdateDescriptorSets()
{
    for (uint32_t i = 0; i < TotalSize; ++i)
    {
        UpdateDescriptorSet(GENERATE_RAYS_LAYOUT_INDEX, CAMERA_RAYS_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("InitialRaysBuffer"));
        UpdateDescriptorSet(GENERATE_RAYS_LAYOUT_INDEX, CAMERA_POSITION_BUFFER, i, CAMERA_SYSTEM()->DeviceBuffer);
		UpdateDescriptorSet(GENERATE_RAYS_LAYOUT_INDEX, GENERATE_RAYS_PIXEL_INDEX_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("PixelIndexBuffer"));
		UpdateDescriptorSet(GENERATE_RAYS_LAYOUT_INDEX, GENERATE_RAYS_RENDER_ITERATION_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("RenderIterationBuffer"));
		UpdateDescriptorSet(GENERATE_RAYS_LAYOUT_INDEX, GENERATE_RAYS_THROUGHPUT_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("ThroughputBuffer"));
		UpdateDescriptorSet(GENERATE_RAYS_LAYOUT_INDEX, GENERATE_RAYS_DEBUG_CMJ_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer("DebugCMJBuffer"));
    }
};

void FGenerateInitialRays::RecordCommands()
{
    CommandBuffers.resize(TotalSize);

    for (uint32_t i = 0; i < TotalSize; ++i)
    {
        CommandBuffers[i] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
			ResetQueryPool(CommandBuffer, i);
			GPU_TIMER();

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto ComputeDescriptorSet = VK_CONTEXT()->DescriptorSetManager->GetSet(Name, GENERATE_RAYS_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &ComputeDescriptorSet, 0, nullptr);

            FPushConstants PushConstants = {Width, Height, 1.f / float(Width), 1.f / float(Height), Width * Height, 0, 0};
            vkCmdPushConstants(CommandBuffer, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstants), &PushConstants);

            vkCmdDispatch(CommandBuffer, CalculateGroupCount(Width * Height, BASIC_CHUNK_SIZE), 1, 1);
        }, QueueFlagsBits);

        V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
    }
};
