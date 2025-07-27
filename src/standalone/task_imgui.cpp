#include "task_imgui.h"
#include "vk_context.h"
#include "vk_debug.h"

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imGuIZMO.quat/imGuIZMO.quat/imGuIZMOquat.h"
#include "ImGuiProfilerRenderer.h"
#include "L2DFileDialog.h"

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

void FImguiTask::Init(FCompileDefinitions* CompileDefinitions)
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
		ImVec2 Position(50.0f, Height - 20.0f);
		ImDrawList* drawList = ImGui::GetForegroundDrawList();
		drawList->AddText(Position, IM_COL32(0, 255, 0, 255), ("Frames accumulated: " + std::to_string(Render->Counter) + ".").c_str());
	}

	static EOutputType SelectedAOV = EOutputType::Color;
	static EOutputType CurrentAOV = EOutputType::Color;
	static bool bDisplayProfiler = false;
	static bool bDisplayMaterialConfigurator = false;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Windows"))
		{
			if (ImGui::MenuItem("Profiler")) {bDisplayProfiler = true; bFirstCall = true;};
			if (ImGui::MenuItem("Material configurator")) {bDisplayMaterialConfigurator = true;};
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("AOV"))
		{
			if (ImGui::BeginMenu("General"))
			{
				if (ImGui::MenuItem("Color")) { SelectedAOV = EOutputType::Color; };
				if (ImGui::MenuItem("Shading normal")) { SelectedAOV = EOutputType::ShadingNormal; };
				if (ImGui::MenuItem("Geometric normal")) { SelectedAOV = EOutputType::GeometricNormal; };
				if (ImGui::MenuItem("UV")) { SelectedAOV = EOutputType::UV; };
				if (ImGui::MenuItem("World-space position")) { SelectedAOV = EOutputType::WorldSpacePosition; };
				if (ImGui::MenuItem("Opacity")) { SelectedAOV = EOutputType::Opacity; };
				if (ImGui::MenuItem("Depth")) { SelectedAOV = EOutputType::Depth; };
				if (ImGui::MenuItem("Luminance")) { SelectedAOV = EOutputType::Luminance; };
				if (ImGui::MenuItem("MaterialID")) { SelectedAOV = EOutputType::MaterialID; };
				if (ImGui::MenuItem("RenderableID")) { SelectedAOV = EOutputType::RenderableID; };
				if (ImGui::MenuItem("PrimitiveID")) { SelectedAOV = EOutputType::PrimitiveID; };
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Material"))
			{
				if (ImGui::MenuItem("Material base weight")) { SelectedAOV = EOutputType::MaterialBaseWeight; };
				if (ImGui::MenuItem("Material base color")) { SelectedAOV = EOutputType::MaterialBaseColor; };
				if (ImGui::MenuItem("Material diffuse roughness")) { SelectedAOV = EOutputType::MaterialDiffuseRoughness; };
				if (ImGui::MenuItem("Material metalness")) { SelectedAOV = EOutputType::MaterialMetalness; };
				if (ImGui::MenuItem("Material normal")) { SelectedAOV = EOutputType::MaterialNormal; };
				if (ImGui::MenuItem("Material specular weight")) { SelectedAOV = EOutputType::MaterialSpecularWeight; };
				if (ImGui::MenuItem("Material specular color")) { SelectedAOV = EOutputType::MaterialSpecularColor; };
				if (ImGui::MenuItem("Material specular roughness")) { SelectedAOV = EOutputType::MaterialSpecularRoughness; };
				if (ImGui::MenuItem("Material specular ior")) { SelectedAOV = EOutputType::MaterialSpecularIOR; };
				if (ImGui::MenuItem("Material specular anisotropy")) { SelectedAOV = EOutputType::MaterialSpecularAnisotropy; };
				if (ImGui::MenuItem("Material specular rotation")) { SelectedAOV = EOutputType::MaterialSpecularRotation; };
				if (ImGui::MenuItem("Material transmission weight")) { SelectedAOV = EOutputType::MaterialTransmissionWeight; };
				if (ImGui::MenuItem("Material transmission color")) { SelectedAOV = EOutputType::MaterialTransmissionColor; };
				if (ImGui::MenuItem("Material transmission depth")) { SelectedAOV = EOutputType::MaterialTransmissionDepth; };
				if (ImGui::MenuItem("Material transmission scatter")) { SelectedAOV = EOutputType::MaterialTransmissionScatter; };
				if (ImGui::MenuItem("Material transmission anisotropy")) { SelectedAOV = EOutputType::MaterialTransmissionAnisotropy; };
				if (ImGui::MenuItem("Material transmission dispersion")) { SelectedAOV = EOutputType::MaterialTransmissionDispersion; };
				if (ImGui::MenuItem("Material transmission roughness")) { SelectedAOV = EOutputType::MaterialTransmissionRoughness; };
				if (ImGui::MenuItem("Material subsurface weight")) { SelectedAOV = EOutputType::MaterialSubsurfaceWeight; };
				if (ImGui::MenuItem("Material subsurface color")) { SelectedAOV = EOutputType::MaterialSubsurfaceColor; };
				if (ImGui::MenuItem("Material subsurface radius")) { SelectedAOV = EOutputType::MaterialSubsurfaceRadius; };
				if (ImGui::MenuItem("Material subsurface scale")) { SelectedAOV = EOutputType::MaterialSubsurfaceScale; };
				if (ImGui::MenuItem("Material subsurface anisotropy")) { SelectedAOV = EOutputType::MaterialSubsurfaceAnisotropy; };
				if (ImGui::MenuItem("Material sheen weight")) { SelectedAOV = EOutputType::MaterialSheenWeight; };
				if (ImGui::MenuItem("Material sheen color")) { SelectedAOV = EOutputType::MaterialSheenColor; };
				if (ImGui::MenuItem("Material sheen roughness")) { SelectedAOV = EOutputType::MaterialSheenRoughness; };
				if (ImGui::MenuItem("Material coat weight")) { SelectedAOV = EOutputType::MaterialCoatWeight; };
				if (ImGui::MenuItem("Material coat colo")) { SelectedAOV = EOutputType::MaterialCoatColor; };
				if (ImGui::MenuItem("Material coat roughness")) { SelectedAOV = EOutputType::MaterialCoatRoughness; };
				if (ImGui::MenuItem("Material coat anisotropy")) { SelectedAOV = EOutputType::MaterialCoatAnisotropy; };
				if (ImGui::MenuItem("Material coat rotation")) { SelectedAOV = EOutputType::MaterialCoatRotation; };
				if (ImGui::MenuItem("Material coat ior")) { SelectedAOV = EOutputType::MaterialCoatIOR; };
				if (ImGui::MenuItem("Material coat normal")) { SelectedAOV = EOutputType::MaterialCoatNormal; };
				if (ImGui::MenuItem("Material coat affect color")) { SelectedAOV = EOutputType::MaterialCoatAffectColor; };
				if (ImGui::MenuItem("Material coat affect roughness")) { SelectedAOV = EOutputType::MaterialCoatAffectRoughness; };
				if (ImGui::MenuItem("Material thin film thickness")) { SelectedAOV = EOutputType::MaterialThinFilmThickness; };
				if (ImGui::MenuItem("Material thin film ior")) { SelectedAOV = EOutputType::MaterialThinFilmIOR; };
				if (ImGui::MenuItem("Material emission weight")) { SelectedAOV = EOutputType::MaterialEmissionWeight; };
				if (ImGui::MenuItem("Material emission color")) { SelectedAOV = EOutputType::MaterialEmissionColor; };
				if (ImGui::MenuItem("Material opacity")) { SelectedAOV = EOutputType::MaterialOpacity; };
				if (ImGui::MenuItem("Material thin walled")) { SelectedAOV = EOutputType::MaterialThinWalled; };
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Debug"))
			{
				if (ImGui::MenuItem("DebugLayer0")) { SelectedAOV = EOutputType::DebugLayer0; };
				if (ImGui::MenuItem("DebugLayer1")) { SelectedAOV = EOutputType::DebugLayer1; };
				if (ImGui::MenuItem("DebugLayer2")) { SelectedAOV = EOutputType::DebugLayer2; };
				if (ImGui::MenuItem("DebugLayer3")) { SelectedAOV = EOutputType::DebugLayer3; };
				if (ImGui::MenuItem("DebugLayer4")) { SelectedAOV = EOutputType::DebugLayer4; };
				if (ImGui::MenuItem("DebugLayer5")) { SelectedAOV = EOutputType::DebugLayer5; };
				if (ImGui::MenuItem("DebugLayer6")) { SelectedAOV = EOutputType::DebugLayer6; };
				if (ImGui::MenuItem("DebugLayer7")) { SelectedAOV = EOutputType::DebugLayer7; };
				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (CurrentAOV != SelectedAOV)
	{
		CurrentAOV = SelectedAOV;
		Render->SetRenderTarget(SelectedAOV);
	}

	/// Profiler
	static ImGuiUtils::ProfilersWindow ProfilerData;

	if (!bFirstCall)
	{
		if (bDisplayProfiler)
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

			ProfilerData.Render(&bDisplayProfiler);
		}
	}

	/// Material configurator
	if (bDisplayMaterialConfigurator && ImGui::Begin("Material configurator", &bDisplayMaterialConfigurator))
	{
		auto& Materials = Render->GetMaterials();
		std::vector<std::string> Names;
		std::vector<ECS::FEntity> MaterialsOrdered;
		static char* FileDialogBuffer = nullptr;
		static char Path[500];

		for (auto& Material : Materials)
		{
			Names.push_back("Material entity " + std::to_string(Material));
			MaterialsOrdered.push_back(Material);
		}

		static int CurrentMaterialIndex = 0;

		if (!Names.empty())
		{
			if (ImGui::BeginCombo("Material", Names[CurrentMaterialIndex].c_str()))
			{
				for (int i = 0; i < Names.size(); ++i)
				{
					bool IsSelected = (CurrentMaterialIndex == i);
					if (ImGui::Selectable(Names[i].c_str(), IsSelected))
					{
						CurrentMaterialIndex = i;
					}

					if (IsSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			auto CurrentMaterial = MaterialsOrdered[CurrentMaterialIndex];
			auto& Material = Render->GetMaterial(CurrentMaterial);

			float BaseWeight = Material.BaseWeight;
			if (ImGui::SliderFloat("Base weight", &BaseWeight, 0.0f, 1.0f))
			{
				Render->MaterialSetBaseColorWeight(CurrentMaterial, BaseWeight);
			}

			if (Material.BaseColorTexture == UINT32_MAX)
			{
				FVector3 BaseColor = Material.BaseColor;

				static bool bPickerOpened = false;
				if (ImGui::Button("Pick BaseColor"))
				{
					bPickerOpened = true;
				}

				if (bPickerOpened && ImGui::Begin("Base Color", &bPickerOpened))
				{
					if (ImGui::ColorPicker3("Base color", &BaseColor.x))
					{
						Render->MaterialSetBaseColor(CurrentMaterial, BaseColor);
					}

					ImGui::End();
				}
			}
			else
			{
				if (ImGui::Button("Browse##path")) {
					FileDialogBuffer = Path;
					FileDialog::file_dialog_open = true;
					FileDialog::file_dialog_open_type = FileDialog::FileDialogType::OpenFile;
				}

				if (FileDialog::file_dialog_open) {
					FileDialog::ShowFileDialog(&FileDialog::file_dialog_open, FileDialogBuffer, sizeof(FileDialogBuffer), FileDialog::file_dialog_open_type);
					if (!FileDialog::file_dialog_open)
					{
						auto Texture = Render->CreateTexture(FileDialogBuffer);
						Render->MaterialSetBaseColor(CurrentMaterial, Texture);
					}
				}
			}
		}
		ImGui::End();
	}

	/// Axis gizmo
	float GizmoSize = 200.f;
	float GizmoHalfSize = GizmoSize * 0.5f;
	ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
	ImGui::SetNextWindowPos(ImVec2(viewportSize.x - GizmoHalfSize, GizmoHalfSize + ImGui::GetFrameHeight()), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	if (ImGui::Begin("Axes hint", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings ))
	{
		FVector3 Right{};
		FVector3 Up{};
		FVector3 Front{};
		Render->GetCameraPosition(&Right, &Front, &Up, {});
		Right = Cross(Front, Up);
		auto Quat = QuatFromVectors(Right, Up, -Front);
		quat qRot = quat(Quat.W, Quat.X, Quat.Y, Quat.Z);
		ImGui::gizmo3D(" ", qRot, GizmoSize);

		ImGui::End();
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