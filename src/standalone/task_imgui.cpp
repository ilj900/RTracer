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
	constexpr uint32_t MaxNumberOfMaterialWindows = 3;
	static std::vector<char> OpenedMaterialEditorWindows(MaxNumberOfMaterialWindows, 0);
	static std::vector<char*> FileDialogBuffer(MaxNumberOfMaterialWindows);
	static char Path[MaxNumberOfMaterialWindows][500];
	static bool bDisplayRenderSettings = false;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Windows"))
		{
			if (ImGui::MenuItem("Profiler")) {bDisplayProfiler = true; bFirstCall = true;};
			if (ImGui::MenuItem("Material configurator")) { for (int i = 0; i < OpenedMaterialEditorWindows.size(); ++i) if (OpenedMaterialEditorWindows[i] == false) {OpenedMaterialEditorWindows[i] = true; break;} ; };
			if (ImGui::MenuItem("Render settings")) {bDisplayRenderSettings = true;};
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
				if (ImGui::MenuItem("Bounce heatmap")) { SelectedAOV = EOutputType::BounceHeatmap; };
				if (ImGui::MenuItem("MaterialID")) { SelectedAOV = EOutputType::MaterialID; };
				if (ImGui::MenuItem("RenderableID")) { SelectedAOV = EOutputType::RenderableID; };
				if (ImGui::MenuItem("PrimitiveID")) { SelectedAOV = EOutputType::PrimitiveID; };
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Material"))
			{
			    if (ImGui::BeginMenu("Base"))
			    {
			        if (ImGui::MenuItem("Weight")) { SelectedAOV = EOutputType::MaterialBaseWeight; };
			        if (ImGui::MenuItem("Color")) { SelectedAOV = EOutputType::MaterialBaseColor; };
			        if (ImGui::MenuItem("Metalness")) { SelectedAOV = EOutputType::MaterialBaseMetalness; };
			        if (ImGui::MenuItem("Diffuse roughness")) { SelectedAOV = EOutputType::MaterialBaseDiffuseRoughness; };
			        ImGui::EndMenu();
			    }
			    if (ImGui::BeginMenu("Specular"))
			    {
			        if (ImGui::MenuItem("Weight")) { SelectedAOV = EOutputType::MaterialSpecularWeight; };
			        if (ImGui::MenuItem("Color")) { SelectedAOV = EOutputType::MaterialSpecularColor; };
			        if (ImGui::MenuItem("Roughness")) { SelectedAOV = EOutputType::MaterialSpecularRoughness; };
			        if (ImGui::MenuItem("Roughness anisotropy")) { SelectedAOV = EOutputType::MaterialSpecularRoughnessAnisotropy; };
			        if (ImGui::MenuItem("IOR")) { SelectedAOV = EOutputType::MaterialSpecularIOR; };
			        ImGui::EndMenu();
			    }
			    if (ImGui::BeginMenu("Transmission"))
			    {
			        if (ImGui::MenuItem("Weight")) { SelectedAOV = EOutputType::MaterialTransmissionWeight; };
			        if (ImGui::MenuItem("Color")) { SelectedAOV = EOutputType::MaterialTransmissionColor; };
			        if (ImGui::MenuItem("Depth")) { SelectedAOV = EOutputType::MaterialTransmissionDepth; };
			        if (ImGui::MenuItem("Scatter")) { SelectedAOV = EOutputType::MaterialTransmissionScatter; };
			        if (ImGui::MenuItem("Scatter anisotropy")) { SelectedAOV = EOutputType::MaterialTransmissionScatterAnisotropy; };
			        if (ImGui::MenuItem("Dispersion scale")) { SelectedAOV = EOutputType::MaterialTransmissionDispersionScale; };
			        if (ImGui::MenuItem("Dispersion Abbe number")) { SelectedAOV = EOutputType::MaterialTransmissionDispersionAbbeNumber; };
			        ImGui::EndMenu();
			    }
			    if (ImGui::BeginMenu("Subsurface"))
			    {
			        if (ImGui::MenuItem("Weight")) { SelectedAOV = EOutputType::MaterialSubsurfaceWeight; };
			        if (ImGui::MenuItem("Color")) { SelectedAOV = EOutputType::MaterialSubsurfaceColor; };
			        if (ImGui::MenuItem("Radius")) { SelectedAOV = EOutputType::MaterialSubsurfaceRadius; };
			        if (ImGui::MenuItem("Radius scale")) { SelectedAOV = EOutputType::MaterialSubsurfaceRadiusScale; };
			        if (ImGui::MenuItem("Scatter anisotropy")) { SelectedAOV = EOutputType::MaterialSubsurfaceScatterAnisotropy; };
			        ImGui::EndMenu();
			    }
			    if (ImGui::BeginMenu("Coat"))
			    {
			        if (ImGui::MenuItem("Weight")) { SelectedAOV = EOutputType::MaterialCoatWeight; };
			        if (ImGui::MenuItem("Color")) { SelectedAOV = EOutputType::MaterialCoatColor; };
			        if (ImGui::MenuItem("Roughness")) { SelectedAOV = EOutputType::MaterialCoatRoughness; };
			        if (ImGui::MenuItem("Roughness anisotropy")) { SelectedAOV = EOutputType::MaterialCoatRoughnessAnisotropy; };
			        if (ImGui::MenuItem("IOR")) { SelectedAOV = EOutputType::MaterialCoatIOR; };
			        if (ImGui::MenuItem("Darkening")) { SelectedAOV = EOutputType::MaterialCoatDarkening; };
			        ImGui::EndMenu();
			    }
					if (ImGui::BeginMenu("Fuzz"))
			    {
			        if (ImGui::MenuItem("Weight")) { SelectedAOV = EOutputType::MaterialFuzzWeight; };
			        if (ImGui::MenuItem("Color")) { SelectedAOV = EOutputType::MaterialFuzzColor; };
			        if (ImGui::MenuItem("Roughness")) { SelectedAOV = EOutputType::MaterialFuzzRoughness; };
			        ImGui::EndMenu();
			    }
			    if (ImGui::BeginMenu("Emission"))
			    {
			        if (ImGui::MenuItem("Luminance")) { SelectedAOV = EOutputType::MaterialEmissionLuminance; };
			        if (ImGui::MenuItem("Color")) { SelectedAOV = EOutputType::MaterialEmissionColor; };
			        ImGui::EndMenu();
			    }
			    if (ImGui::BeginMenu("Thin film"))
			    {
			        if (ImGui::MenuItem("Weight")) { SelectedAOV = EOutputType::MaterialThinFilmWeight; };
			        if (ImGui::MenuItem("Thickness")) { SelectedAOV = EOutputType::MaterialThinFilmThickness; };
			        if (ImGui::MenuItem("IOR")) { SelectedAOV = EOutputType::MaterialThinFilmIOR; };
			        ImGui::EndMenu();
			    }
			    if (ImGui::BeginMenu("Geometry"))
			    {
			        if (ImGui::MenuItem("Opacity")) { SelectedAOV = EOutputType::MaterialGeometryOpacity; };
			        if (ImGui::MenuItem("Thin walled")) { SelectedAOV = EOutputType::MaterialGeometryThinWalled; };
			        if (ImGui::MenuItem("Normal")) { SelectedAOV = EOutputType::MaterialGeometryNormal; };
			        if (ImGui::MenuItem("Tangent")) { SelectedAOV = EOutputType::MaterialGeometryTangent; };
			        if (ImGui::MenuItem("Coat normal")) { SelectedAOV = EOutputType::MaterialGeometryCoatNormal; };
			        if (ImGui::MenuItem("Coat tangent")) { SelectedAOV = EOutputType::MaterialGeometryCoatTangent; };
			        ImGui::EndMenu();
			    }
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
	for (int i = 0; i < OpenedMaterialEditorWindows.size(); ++i)
	{
		if (!OpenedMaterialEditorWindows[i])
		{
			continue;
		}
		if (ImGui::Begin(("Material configurator " + std::to_string(i)).c_str(), reinterpret_cast<bool*>(&OpenedMaterialEditorWindows[i]), ImGuiWindowFlags_NoCollapse ))
		{
			const auto& Materials = Render->GetMaterials();

			static std::vector<ECS::FEntity> CurrentMaterialEntities(3, Materials.begin()->first);

			if (!Materials.empty())
			{
				if (ImGui::BeginCombo("Material", Materials.find(CurrentMaterialEntities[i])->second.c_str()))
				{
					for (auto& Material : Materials)
					{
						bool IsSelected = (Material.first == CurrentMaterialEntities[i]);
						if (ImGui::Selectable(Material.second.c_str(), IsSelected))
						{
							CurrentMaterialEntities[i] = Material.first;
						}

						if (IsSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				auto& Material = Render->GetMaterial(CurrentMaterialEntities[i]);

				auto PlotSlider = [&](const std::string& ElementName, const uint32_t* TextureIndex, void* MaterialData,
					const std::function<void(ECS::FEntity, float)>& CallbackForFloat,
					const std::function<void(ECS::FEntity, ECS::FEntity)>& CallbackForTexture, float Min, float Max)
				{
					if (*TextureIndex == UINT32_MAX)
					{
						float Value = *static_cast<float*>(MaterialData);
						if (ImGui::SliderFloat(ElementName.c_str(), &Value, Min, Max))
						{
							CallbackForFloat(CurrentMaterialEntities[i], Value);
						}
					}
					else
					{
						if (ImGui::Button((ElementName + "Texture##path").c_str())) {
							FileDialogBuffer[i] = Path[i];
							FileDialog::file_dialog_open = true;
							FileDialog::file_dialog_open_type = FileDialog::FileDialogType::OpenFile;
						}

						if (FileDialog::file_dialog_open) {
							FileDialog::ShowFileDialog(&FileDialog::file_dialog_open, FileDialogBuffer[i], sizeof(FileDialogBuffer), FileDialog::file_dialog_open_type);
							if (!FileDialog::file_dialog_open)
							{
								auto Texture = Render->CreateTexture(FileDialogBuffer[i]);
								CallbackForTexture(CurrentMaterialEntities[i], Texture);
							}
						}
					}
				};

				auto PlotVec3 = [&](const std::string& ElementName, const uint32_t* TextureIndex, void* MaterialData,
				const std::function<void(ECS::FEntity, const FVector3&)>& CallbackForVec3,
					const std::function<void(ECS::FEntity, ECS::FEntity)>& CallbackForTexture)
				{
					if (*TextureIndex == UINT32_MAX)
					{
						FVector3 BaseColor = *static_cast<FVector3*>(MaterialData);

						if (ImGui::ColorEdit3(ElementName.c_str(), &BaseColor.x))
						{
							CallbackForVec3(CurrentMaterialEntities[i], BaseColor);
						}
					}
					else
					{
						if (ImGui::Button((ElementName + "Texture##path").c_str())) {
							FileDialogBuffer[i] = Path[i];
							FileDialog::file_dialog_open = true;
							FileDialog::file_dialog_open_type = FileDialog::FileDialogType::OpenFile;
						}

						if (FileDialog::file_dialog_open) {
							FileDialog::ShowFileDialog(&FileDialog::file_dialog_open, FileDialogBuffer[i], sizeof(FileDialogBuffer), FileDialog::file_dialog_open_type);
							if (!FileDialog::file_dialog_open)
							{
								auto Texture = Render->CreateTexture(FileDialogBuffer[i]);
								CallbackForTexture(CurrentMaterialEntities[i], Texture);
							}
						}
					}
				};

				enum class UIElementType {FloatSlider01, FloatSlider05, FloatSlider020, FloatSlider0360, FloatSlider070, Float3Color};

				auto AddMaterialPropertyElement = [&](const std::string& ElementName, UIElementType ElementType, const uint32_t* TextureIndex, void* MaterialData,
					const std::function<void(ECS::FEntity, float)>& CallbackForFloat,
					const std::function<void(ECS::FEntity, const FVector3&)>& CallbackForVec3,
					const std::function<void(ECS::FEntity, ECS::FEntity)>& CallbackForTexture)
				{
					switch (ElementType)
					{
						case UIElementType::FloatSlider01:
						{
							PlotSlider(ElementName, TextureIndex, MaterialData, CallbackForFloat, CallbackForTexture, 0.f, 1.f);
							break;
						}
						case UIElementType::FloatSlider05:
						{
							PlotSlider(ElementName, TextureIndex, MaterialData, CallbackForFloat, CallbackForTexture, 0.f, 5.f);
							break;
						}
						case UIElementType::FloatSlider020:
						{
							PlotSlider(ElementName, TextureIndex, MaterialData, CallbackForFloat, CallbackForTexture, 0.f, 20.f);
							break;
						}
						case UIElementType::FloatSlider0360:
						{
							PlotSlider(ElementName, TextureIndex, MaterialData, CallbackForFloat, CallbackForTexture, 0.f, 360.f);
							break;
						}
						case UIElementType::FloatSlider070:
						{
							PlotSlider(ElementName, TextureIndex, MaterialData, CallbackForFloat, CallbackForTexture, 0.f, 70.f);
							break;
						}
						case UIElementType::Float3Color:
						{
							PlotVec3(ElementName, TextureIndex, MaterialData, CallbackForVec3, CallbackForTexture);
							break;
						}
					}
				};

				static const auto DummyLambdaFloat = [](ECS::FEntity, float){};
				static const auto DummyLambdaVec3 = [](ECS::FEntity, const FVector3&){};

// ---- Base ----
				AddMaterialPropertyElement("Base weight",				UIElementType::FloatSlider01,	&Material.BaseWeightTextureIndex,			(void*)&Material.BaseWeight,			static_cast<void(*)(ECS::FEntity, float)>(&FRender::MaterialSetBaseWeight),				DummyLambdaVec3,																		static_cast<void(*)(ECS::FEntity, ECS::FEntity)>(&FRender::MaterialSetBaseWeight));
				AddMaterialPropertyElement("Base color",					UIElementType::Float3Color,		&Material.BaseColorTextureIndex,			(void*)&Material.BaseColor,				DummyLambdaFloat,																		static_cast<void(*)(ECS::FEntity, const FVector3&)>(&FRender::MaterialSetBaseColor),	static_cast<void(*)(ECS::FEntity, ECS::FEntity)>(&FRender::MaterialSetBaseColor));
				AddMaterialPropertyElement("Base metalness",				UIElementType::FloatSlider01,	&Material.BaseMetalnessTextureIndex,		(void*)&Material.BaseMetalness,			static_cast<void(*)(ECS::FEntity, float)>(&FRender::MaterialSetBaseMetalness),			DummyLambdaVec3,																		static_cast<void(*)(ECS::FEntity, ECS::FEntity)>(&FRender::MaterialSetBaseMetalness));
				AddMaterialPropertyElement("Base diffuse roughness",		UIElementType::FloatSlider01,	&Material.BaseDiffuseRoughnessTextureIndex,	(void*)&Material.BaseDiffuseRoughness,	static_cast<void(*)(ECS::FEntity, float)>(&FRender::MaterialSetBaseDiffuseRoughness),	DummyLambdaVec3,																		static_cast<void(*)(ECS::FEntity, ECS::FEntity)>(&FRender::MaterialSetBaseDiffuseRoughness));

				// ---- Specular ----
				AddMaterialPropertyElement("Specular weight",			UIElementType::FloatSlider01,				&Material.SpecularWeightTextureIndex,					(void*)&Material.SpecularWeight,					static_cast<void(*)(ECS::FEntity, float)>(&FRender::MaterialSetSpecularWeight),						DummyLambdaVec3,																					static_cast<void(*)(ECS::FEntity, ECS::FEntity)>(&FRender::MaterialSetSpecularWeight));
				AddMaterialPropertyElement("Specular color",				UIElementType::Float3Color,					&Material.SpecularColorTextureIndex,					(void*)&Material.SpecularColor,						DummyLambdaFloat,																					static_cast<void(*)(ECS::FEntity, const FVector3&)>(&FRender::MaterialSetSpecularColor),				static_cast<void(*)(ECS::FEntity, ECS::FEntity)>(&FRender::MaterialSetSpecularColor));
				AddMaterialPropertyElement("Specular roughness",			UIElementType::FloatSlider01,				&Material.SpecularRoughnessTextureIndex,				(void*)&Material.SpecularRoughness,					static_cast<void(*)(ECS::FEntity, float)>(&FRender::MaterialSetSpecularRoughness),					DummyLambdaVec3,																					static_cast<void(*)(ECS::FEntity, ECS::FEntity)>(&FRender::MaterialSetSpecularRoughness));
				AddMaterialPropertyElement("Specular anisotropy",		UIElementType::FloatSlider01,				&Material.SpecularRoughnessAnisotropyTextureIndex,		(void*)&Material.SpecularRoughnessAnisotropy,		static_cast<void(*)(ECS::FEntity, float)>(&FRender::MaterialSetSpecularRoughnessAnisotropy),		DummyLambdaVec3,																					static_cast<void(*)(ECS::FEntity, ECS::FEntity)>(&FRender::MaterialSetSpecularRoughnessAnisotropy));
				AddMaterialPropertyElement("Specular IOR",				UIElementType::FloatSlider05,				&Material.SpecularIorTextureIndex,						(void*)&Material.SpecularIor,						static_cast<void(*)(ECS::FEntity, float)>(&FRender::MaterialSetSpecularIor),							DummyLambdaVec3,																					static_cast<void(*)(ECS::FEntity, ECS::FEntity)>(&FRender::MaterialSetSpecularIor));

				// ---- Transmission ----
				AddMaterialPropertyElement("Transmission weight",			UIElementType::FloatSlider01,				&Material.TransmissionWeightTextureIndex,				(void*)&Material.TransmissionWeight,				static_cast<void(*)(ECS::FEntity, float)>(&FRender::MaterialSetTransmissionWeight),					DummyLambdaVec3,																					static_cast<void(*)(ECS::FEntity, ECS::FEntity)>(&FRender::MaterialSetTransmissionWeight));
				AddMaterialPropertyElement("Transmission color",				UIElementType::Float3Color,					&Material.TransmissionColorTextureIndex,				(void*)&Material.TransmissionColor,					DummyLambdaFloat,																					static_cast<void(*)(ECS::FEntity, const FVector3&)>(&FRender::MaterialSetTransmissionColor),			static_cast<void(*)(ECS::FEntity, ECS::FEntity)>(&FRender::MaterialSetTransmissionColor));
				AddMaterialPropertyElement("Transmission depth",				UIElementType::FloatSlider020,				&Material.TransmissionDepthTextureIndex,				(void*)&Material.TransmissionDepth,					static_cast<void(*)(ECS::FEntity, float)>(&FRender::MaterialSetTransmissionDepth),					DummyLambdaVec3,																					static_cast<void(*)(ECS::FEntity, ECS::FEntity)>(&FRender::MaterialSetTransmissionDepth));
				AddMaterialPropertyElement("Transmission scatter",			UIElementType::Float3Color,					&Material.TransmissionScatterTextureIndex,				(void*)&Material.TransmissionScatter,				DummyLambdaFloat,																					static_cast<void(*)(ECS::FEntity, const FVector3&)>(&FRender::MaterialSetTransmissionScatter),		static_cast<void(*)(ECS::FEntity, ECS::FEntity)>(&FRender::MaterialSetTransmissionScatter));
				AddMaterialPropertyElement("Transmission anisotropy",		UIElementType::FloatSlider01,		&Material.TransmissionScatterAnisotropyTextureIndex,	(void*)&Material.TransmissionScatterAnisotropy,		static_cast<void(*)(ECS::FEntity, float)>(&FRender::MaterialSetTransmissionScatterAnisotropy),		DummyLambdaVec3,																					static_cast<void(*)(ECS::FEntity, ECS::FEntity)>(&FRender::MaterialSetTransmissionScatterAnisotropy));
				AddMaterialPropertyElement("Transmission dispersion scale",	UIElementType::FloatSlider01,				&Material.TransmissionDispersionScaleTextureIndex,		(void*)&Material.TransmissionDispersionScale,		static_cast<void(*)(ECS::FEntity, float)>(&FRender::MaterialSetTransmissionDispersionScale),			DummyLambdaVec3,																					static_cast<void(*)(ECS::FEntity, ECS::FEntity)>(&FRender::MaterialSetTransmissionDispersionScale));
				AddMaterialPropertyElement("Transmission Abbe number",		UIElementType::FloatSlider020,		&Material.TransmissionDispersionAbbeNumberTextureIndex,	(void*)&Material.TransmissionDispersionAbbeNumber,	static_cast<void(*)(ECS::FEntity, float)>(&FRender::MaterialSetTransmissionDispersionAbbeNumber),	DummyLambdaVec3,																					static_cast<void(*)(ECS::FEntity, ECS::FEntity)>(&FRender::MaterialSetTransmissionDispersionAbbeNumber));
			}
			ImGui::End();
		}
	}

	/// Render settings
	{
		if (bDisplayRenderSettings && ImGui::Begin("Render Settings", &bDisplayRenderSettings, ImGuiWindowFlags_NoCollapse ))
		{
			static bool bAccumulateFrames = true;
			if (ImGui::Checkbox("Accumulate frames", &bAccumulateFrames))
			{
				Render->SetAccumulateFrames(bAccumulateFrames);
			}

			static bool bAccumulateBounces = false;
			if (ImGui::Checkbox("Accumulate bounces", &bAccumulateBounces))
			{
				Render->SetAccumulateBounces(bAccumulateBounces);
			}

			static char Buffer[16] = "";
			static int Index = -1;
			if (ImGui::InputText("Save frame at index: ", Buffer, sizeof(Buffer), ImGuiInputTextFlags_CharsDecimal))
			{
				Index = std::stoi(Buffer);
			}

			if (Render->Counter == Index)
			{
				static int SaveImageCounter = 0;
				Render->PrintScreenExr("Autosave_" + std::to_string(SaveImageCounter++));
			}

			ImGui::End();
		}
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
