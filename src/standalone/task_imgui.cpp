#include "task_imgui.h"
#include "vk_context.h"
#include "vk_debug.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "ImGuiProfilerRenderer.h"

#include "GLFW/glfw3.h"

#include <iostream>

FImguiTask::FImguiTask(uint32_t WidthIn, uint32_t HeightIn, uint32_t SubmitXIn, uint32_t SubmitYIn, VkDevice LogicalDevice) :
	FExecutableTask(WidthIn, HeightIn, SubmitXIn, SubmitYIn, LogicalDevice)
{
	Name = "Imgui pipeline";

	PipelineStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	QueueFlagsBits = VK_QUEUE_GRAPHICS_BIT;
}

FImguiTask::~FImguiTask()
{
	Render = nullptr;

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	for (auto Framebuffer : ImguiFramebuffers)
	{
		vkDestroyFramebuffer(LogicalDevice, Framebuffer, nullptr);
	}

	vkDestroyRenderPass(LogicalDevice, RenderPass, nullptr);

	vkDestroyDescriptorPool(LogicalDevice, DescriptorPool, nullptr);
}

void FImguiTask::SetGLFWWindow(GLFWwindow* WindowIn)
{
	Window = WindowIn;
}

void FImguiTask::SetRender(std::shared_ptr<FRender> RenderIn)
{
	Render = RenderIn;
}

void FImguiTask::Init()
{
}

void FImguiTask::Init(std::vector<ImagePtr> Images)
{
	bFirstCall = true;

	ExternalImages = std::move(Images);

	FGraphicsPipelineOptions ImguiPipelineOptions;

	ImguiPipelineOptions.RegisterColorAttachment(0, ExternalImages[0], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ATTACHMENT_LOAD_OP_LOAD);
	RenderPass = VK_CONTEXT()->CreateRenderpass(LogicalDevice, ImguiPipelineOptions);

	V::SetName(LogicalDevice, RenderPass, Name);

	ImguiFramebuffers.resize(TotalSize);

	for (int i = 0; i < TotalSize; ++i)
	{
		ImguiFramebuffers[i] = VK_CONTEXT()->CreateFramebuffer(Width, Height, {ExternalImages[i]}, RenderPass, Name);
	}

	std::map<VkDescriptorType, uint32_t> PoolSizes =
		{
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
		};

	DescriptorPool = VK_CONTEXT()->CreateFreeableDescriptorPool(PoolSizes, LogicalDevice, "V_ImGuiDescriptorPool");

	auto CheckResultFunction = [](VkResult Err)
	{
		if (Err == 0)
			return;
		std::cout << "[vulkan] Error: VkResult = " << Err << std::endl;
		if (Err < 0)
			abort();
	};

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& IO = ImGui::GetIO();
	IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(Window, true);
	ImGui_ImplVulkan_InitInfo InitInfo{};
	InitInfo.Instance = VK_CONTEXT()->GetInstance();
	InitInfo.PhysicalDevice = VK_CONTEXT()->GetPhysicalDevice();
	InitInfo.Device = LogicalDevice;
	InitInfo.QueueFamily = VK_CONTEXT()->GetGraphicsQueueIndex();
	InitInfo.Queue = VK_CONTEXT()->GetGraphicsQueue();
	InitInfo.DescriptorPool = DescriptorPool;
	InitInfo.RenderPass = RenderPass;
	InitInfo.MinImageCount = TotalSize;
	InitInfo.ImageCount = TotalSize;
	InitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	InitInfo.CheckVkResultFn = CheckResultFunction;
	ImGui_ImplVulkan_Init(&InitInfo);
}

void FImguiTask::UpdateDescriptorSets()
{
}

void FImguiTask::RecordCommands()
{
	CommandBuffers.resize(TotalSize);
}

FSynchronizationPoint FImguiTask::Submit(VkPipelineStageFlags& PipelineStageFlagsIn, FSynchronizationPoint SynchronizationPoint, uint32_t X, uint32_t Y)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	auto StringToColor = [](const std::string& String)
	{
		static std::hash<std::string> HashingFunction;
		auto Color = (uint32_t)HashingFunction(String);
		Color |= 255 << 24;
		return Color;
	};

	{
		ImVec2 Position(50.0f, 20.0f);
		ImDrawList* drawList = ImGui::GetForegroundDrawList();
		drawList->AddText(Position, IM_COL32(0, 255, 0, 255), ("Frames accumulated: " + std::to_string(Render->Counter) + ".").c_str());
	}

	static ImGuiUtils::ProfilersWindow ProfilerData;

	if (!bFirstCall)
	{
		std::vector<std::string> Names;
		std::vector<std::vector<float>> Timings;
		float DeltaTime = 0;
		Render->GetAllTimings(Names, Timings, DeltaTime, PreviousIterationIndex);
		std::vector<legit::ProfilerTask> GPUTasks(Timings.size());

		float StartTime = 0.f;
		float EndTime = 0.f;

		for (int i = 0; i < Names.size(); ++i)
		{
			float TaskTime = 0;

			for (int j = 0; j < Timings[i].size(); j++)
			{
				TaskTime += Timings[i][j];
			}

			EndTime = StartTime + TaskTime;
			GPUTasks[i] = {StartTime, EndTime, Names[i], StringToColor(Names[i])};
			StartTime = EndTime;
		}

		ProfilerData.gpuGraph.LoadFrameData(GPUTasks.data(), GPUTasks.size());

		ProfilerData.Render();
	}

	{
		std::vector<const char*> Names = {"Color", "Normal", "UV", "World-space position", "DebugLayer"};
		static EOutputType CurrentAOV = EOutputType::Color;

		if (ImGui::BeginCombo("Select AOV", Names[int(CurrentAOV)]))
		{
			for (int i = 0; i < Names.size(); ++i)
			{
				bool IsSelected = i == int(CurrentAOV);
				if (ImGui::Selectable(Names[i], IsSelected))
				{
					CurrentAOV = EOutputType(i);
					Render->SetRenderTarget(CurrentAOV);
				}

				if (IsSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}
	}

	uint32_t SubmitIndex = Y * SubmitX + X;

	CommandBuffers[SubmitIndex] = COMMAND_BUFFER_MANAGER()->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
	{
		{
			ResetQueryPool(CommandBuffer, SubmitIndex);
			FGPUTimer GPUTimer(CommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, QueryPool, SubmitIndex);

			VkRenderPassBeginInfo RenderPassBeginInfo{};
			RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			RenderPassBeginInfo.renderPass = RenderPass;
			RenderPassBeginInfo.framebuffer = ImguiFramebuffers[SubmitIndex % TotalSize];
			/// TODO: find a better way to pass extent
			RenderPassBeginInfo.renderArea.extent = {uint32_t(Width), uint32_t(Height)};
			RenderPassBeginInfo.clearValueCount = 0;
			vkCmdBeginRenderPass(CommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		}

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), CommandBuffer);

		vkCmdEndRenderPass(CommandBuffer);
	}, QueueFlagsBits);

	V::SetName(LogicalDevice, CommandBuffers[SubmitIndex], Name, SubmitIndex);

	if (bFirstCall)
	{
		bFirstCall = false;
	}

	PreviousIterationIndex = SubmitIndex;

	return FExecutableTask::Submit(PipelineStageFlags, SynchronizationPoint, X, Y);
}