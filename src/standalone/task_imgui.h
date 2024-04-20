#pragma once

#include "tasks/executable_task.h"
#include "vk_pipeline.h"

class FVulkanContext;
struct GLFWwindow;

class FImguiTask : public FExecutableTask
{
public:
	FImguiTask(uint32_t WidthIn, uint32_t HeightIn, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice);
	~FImguiTask() override;

	void SetGLFWWindow(GLFWwindow* WindowIn);

	void Init(std::vector<ImagePtr> Images);
	void Init() override;
	void UpdateDescriptorSets() override;
	void RecordCommands() override;
	FSynchronizationPoint Submit(VkPipelineStageFlags& PipelineStageFlagsIn, FSynchronizationPoint SynchronizationPoint, uint32_t IterationIndex) override;

	FGraphicsPipelineOptions GraphicsPipelineOptions;

	VkRenderPass RenderPass = VK_NULL_HANDLE;

	VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;

	std::vector<VkFramebuffer> ImguiFramebuffers;
	std::vector<ImagePtr> ExternalImages;

	GLFWwindow* Window = nullptr;
	uint32_t PreviousIterationIndex = 0;
	bool bFirstCall = true;
};