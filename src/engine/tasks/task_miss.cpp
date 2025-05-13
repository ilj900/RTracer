#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "common_structures.h"

#include "vk_shader_compiler.h"
#include "texture_manager.h"

#include "task_miss.h"

FMissTask::FMissTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
    Name = "Miss pipeline";

    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_RAYS_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_IBL_IMAGE_SAMPLER_LINEAR_INDEX,
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_IBL_IMAGE_SAMPLER_NEAREST_INDEX,
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_MATERIAL_INDEX_MAP,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_MATERIAL_INDEX_AOV_MAP,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
    DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_MATERIALS_OFFSETS,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_PREVIOUS_BOUNCE_NORMAL_AOV_DATA_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_CUMULATIVE_MATERIAL_COLOR_BUFFER_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_THROUGHPUT_BUFFER,
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_OUTPUT_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_SHADING_NORMAL_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_GEOMETRIC_NORMAL_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_UV_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_WORLD_SPACE_POSITION_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_OPACITY_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_DEPTH_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_ALBEDO_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_LUMINANCE_AOV_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_RENDERABLE_INDEX_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_PRIMITIVE_INDEX_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_MATERIAL_INDEX_IMAGE_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_DEBUG_LAYER_IMAGE_0_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_DEBUG_LAYER_IMAGE_1_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_DEBUG_LAYER_IMAGE_2_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});
	DescriptorSetManager->AddDescriptorLayout(Name, COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_DEBUG_LAYER_IMAGE_3_INDEX,
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  VK_SHADER_STAGE_COMPUTE_BIT});

    VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstants)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    PipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    QueueFlagsBits = VK_QUEUE_COMPUTE_BIT;
}

FMissTask::~FMissTask()
{
    vkDestroySampler(LogicalDevice, IBLImageSamplerLinear, nullptr);
	vkDestroySampler(LogicalDevice, IBLImageSamplerNearest, nullptr);
};

void FMissTask::Init(FCompileDefinitions* CompileDefinitions)
{
    auto& DescriptorSetManager = VK_CONTEXT()->DescriptorSetManager;

    auto ShadeShader = FShader("../src/shaders/miss.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);

    Pipeline = VK_CONTEXT()->CreateComputePipeline(ShadeShader(), PipelineLayout);

	IBLImageSamplerLinear = VK_CONTEXT()->CreateTextureSampler(VK_SAMPLE_COUNT_1_BIT, VK_FILTER_LINEAR);
	IBLImageSamplerNearest = VK_CONTEXT()->CreateTextureSampler(VK_SAMPLE_COUNT_1_BIT, VK_FILTER_NEAREST);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, COMPUTE_MISS_LAYOUT_INDEX, TotalSize);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FMissTask::UpdateDescriptorSets()
{
    for (int i = 0; i < TotalSize; ++i)
    {
        UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_RAYS_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(INITIAL_RAYS_BUFFER));
        UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_IBL_IMAGE_SAMPLER_LINEAR_INDEX, i, TEXTURE_MANAGER()->GetTexture("IBL Image"), IBLImageSamplerLinear);
		UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_IBL_IMAGE_SAMPLER_NEAREST_INDEX, i, TEXTURE_MANAGER()->GetTexture("IBL Image"), IBLImageSamplerNearest);
        UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_MATERIAL_INDEX_MAP, i, RESOURCE_ALLOCATOR()->GetBuffer(PIXEL_INDEX_BUFFER));
        UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_MATERIAL_INDEX_AOV_MAP, i, RESOURCE_ALLOCATOR()->GetBuffer(MATERIAL_INDEX_AOV_BUFFER));
        UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_MATERIALS_OFFSETS, i, RESOURCE_ALLOCATOR()->GetBuffer(MATERIALS_OFFSETS_PER_MATERIAL_BUFFER));
		UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_PREVIOUS_BOUNCE_NORMAL_AOV_DATA_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer(NORMAL_AOV_BUFFER));
		UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_CUMULATIVE_MATERIAL_COLOR_BUFFER_INDEX, i, RESOURCE_ALLOCATOR()->GetBuffer(CUMULATIVE_MATERIAL_COLOR_BUFFER));
		UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_THROUGHPUT_BUFFER, i, RESOURCE_ALLOCATOR()->GetBuffer(THROUGHPUT_BUFFER));
        UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_OUTPUT_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("ColorImage"));
		UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_SHADING_NORMAL_AOV_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("ShadingNormalAOVImage"));
		UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_GEOMETRIC_NORMAL_AOV_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("GeometricNormalAOVImage"));
		UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_UV_AOV_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("UVAOVImage"));
		UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_WORLD_SPACE_POSITION_AOV_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("WorldSpacePositionAOVImage"));
		UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_OPACITY_AOV_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("OpacityAOVImage"));
		UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_DEPTH_AOV_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("DepthAOVImage"));
		UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_ALBEDO_AOV_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("AlbedoAOVImage"));
		UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_LUMINANCE_AOV_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("LuminanceAOVImage"));
		UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_RENDERABLE_INDEX_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("RenderableIDAOVImage"));
		UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_PRIMITIVE_INDEX_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("PrimitiveIDAOVImage"));
		UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_MATERIAL_INDEX_IMAGE_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("MaterialIDAOVImage"));
		UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_DEBUG_LAYER_IMAGE_0_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("DebugLayerImage0"));
		UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_DEBUG_LAYER_IMAGE_1_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("DebugLayerImage1"));
		UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_DEBUG_LAYER_IMAGE_2_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("DebugLayerImage2"));
		UpdateDescriptorSet(COMPUTE_MISS_LAYOUT_INDEX, COMPUTE_MISS_DEBUG_LAYER_IMAGE_3_INDEX, i, TEXTURE_MANAGER()->GetFramebufferImage("DebugLayerImage3"));
    }
};

void FMissTask::RecordCommands()
{
    CommandBuffers.resize(TotalSize);
	auto DispatchBuffer = RESOURCE_ALLOCATOR()->GetBuffer(TOTAL_COUNTED_MATERIALS_BUFFER);

    for (uint32_t i = 0; i < TotalSize; ++i)
    {
        CommandBuffers[i] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
			ResetQueryPool(CommandBuffer, i);
			GPU_TIMER();

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto DescriptorSet = VK_CONTEXT()->DescriptorSetManager->GetSet(Name, COMPUTE_MISS_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &DescriptorSet, 0, nullptr);

            uint32_t MaterialIndex = IBL_MATERIAL_INDEX;
            FPushConstants PushConstants = {Width, Height, 1.f / Width, 1.f / Height, Width * Height, MaterialIndex, i % SubmitX};
            vkCmdPushConstants(CommandBuffer, VK_CONTEXT()->DescriptorSetManager->GetPipelineLayout(Name),
                               VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstants), &PushConstants);

            vkCmdDispatchIndirect(CommandBuffer, DispatchBuffer.Buffer, MaterialIndex * 3 * sizeof(uint32_t));
        }, QueueFlagsBits);

        V::SetName(LogicalDevice, CommandBuffers[i], Name, i);
    }
};
