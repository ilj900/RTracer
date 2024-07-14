#include "coordinator.h"

#include "acceleration_structure_system.h"
#include "mesh_system.h"
#include "transform_system.h"
#include "renderable_system.h"
#include "camera_system.h"
#include "light_system.h"
#include "texture_system.h"

#include "acceleration_structure_component.h"
#include "mesh_component.h"
#include "framebuffer_component.h"
#include "device_mesh_component.h"
#include "device_renderable_component.h"
#include "transform_component.h"
#include "device_camera_component.h"
#include "device_transform_component.h"
#include "material_component.h"
#include "light_component.h"
#include "camera_component.h"
#include "texture_component.h"
#include "material_system.h"

#include "vk_functions.h"
#include "render.h"
#include "texture_manager.h"

#include "utils.h"

#include "logging.h"

FRender::FRender(uint32_t WidthIn, uint32_t HeightIn) : Width(WidthIn), Height(HeightIn)
{
    COORDINATOR().Init();

    /// Register components
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FAccelerationStructureComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FCameraComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FDeviceCameraComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FDeviceMeshComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FDeviceRenderableComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FDeviceTransformComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FLightComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FMaterialComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FMeshComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FMeshInstanceComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FTransformComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FTextureComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FFramebufferComponent>();

    /// Register systems
    auto CameraSystem = COORDINATOR().RegisterSystem<ECS::SYSTEMS::FCameraSystem>();
    auto TransformSystem = COORDINATOR().RegisterSystem<ECS::SYSTEMS::FTransformSystem>();
    auto RenderableSystem = COORDINATOR().RegisterSystem<ECS::SYSTEMS::FRenderableSystem>();
    auto MaterialSystem = COORDINATOR().RegisterSystem<ECS::SYSTEMS::FMaterialSystem>();
    auto MeshSystem = COORDINATOR().RegisterSystem<ECS::SYSTEMS::FMeshSystem>();
    auto LightSystem = COORDINATOR().RegisterSystem<ECS::SYSTEMS::FLightSystem>();
    auto AccelerationSystem = COORDINATOR().RegisterSystem<ECS::SYSTEMS::FAccelerationStructureSystem>();
    auto TextureSystem = COORDINATOR().RegisterSystem<ECS::SYSTEMS::FTextureSystem>();

    /// Set camera system signature
    ECS::FSignature CameraSystemSignature;
    CameraSystemSignature.set(COORDINATOR().GetComponentType<ECS::COMPONENTS::FCameraComponent>());
    CameraSystemSignature.set(COORDINATOR().GetComponentType<ECS::COMPONENTS::FDeviceCameraComponent>());
    COORDINATOR().SetSystemSignature<ECS::SYSTEMS::FCameraSystem>(CameraSystemSignature);

    /// Register Transform system signature
    ECS::FSignature TransformSystemSignature;
    TransformSystemSignature.set(COORDINATOR().GetComponentType<ECS::COMPONENTS::FTransformComponent>());
    TransformSystemSignature.set(COORDINATOR().GetComponentType<ECS::COMPONENTS::FDeviceTransformComponent>());
    COORDINATOR().SetSystemSignature<ECS::SYSTEMS::FTransformSystem>(TransformSystemSignature);

    /// Register Renderable system signature
    ECS::FSignature RenderableSignature;
    RenderableSignature.set(COORDINATOR().GetComponentType<ECS::COMPONENTS::FDeviceRenderableComponent>());
    COORDINATOR().SetSystemSignature<ECS::SYSTEMS::FRenderableSystem>(RenderableSignature);

    /// Register Material system signature
    ECS::FSignature MaterialSignature;
    MaterialSignature.set(COORDINATOR().GetComponentType<ECS::COMPONENTS::FMaterialComponent>());
    COORDINATOR().SetSystemSignature<ECS::SYSTEMS::FMaterialSystem>(MaterialSignature);

    /// Register Mesh systems signature
    ECS::FSignature MeshSignature;
    MeshSignature.set(COORDINATOR().GetComponentType<ECS::COMPONENTS::FMeshComponent>());
    MeshSignature.set(COORDINATOR().GetComponentType<ECS::COMPONENTS::FDeviceMeshComponent>());
    MeshSignature.set(COORDINATOR().GetComponentType<ECS::COMPONENTS::FAccelerationStructureComponent>());
    COORDINATOR().SetSystemSignature<ECS::SYSTEMS::FMeshSystem>(MeshSignature);

    /// Register Light system signature
    ECS::FSignature LightSignature;
    LightSignature.set(COORDINATOR().GetComponentType<ECS::COMPONENTS::FLightComponent>());
    COORDINATOR().SetSystemSignature<ECS::SYSTEMS::FLightSystem>(LightSignature);

    /// Register Acceleration structure system signature
    ECS::FSignature AccelerationStructureSignature;
    AccelerationStructureSignature.set(COORDINATOR().GetComponentType<ECS::COMPONENTS::FMeshInstanceComponent>());
    COORDINATOR().SetSystemSignature<ECS::SYSTEMS::FAccelerationStructureSystem>(AccelerationStructureSignature);

    ECS::FSignature TextureSignature;
    TextureSignature.set(COORDINATOR().GetComponentType<ECS::COMPONENTS::FTextureComponent>());
    COORDINATOR().SetSystemSignature<ECS::SYSTEMS::FTextureSystem>(TextureSignature);

	VK_CONTEXT()->InitManagerResources();
    CAMERA_SYSTEM()->Init(MaxFramesInFlight);
    RENDERABLE_SYSTEM()->Init(MaxFramesInFlight);
    MESH_SYSTEM()->Init();
    LIGHT_SYSTEM()->Init(MaxFramesInFlight);
    TRANSFORM_SYSTEM()->Init(MaxFramesInFlight);
    ACCELERATION_STRUCTURE_SYSTEM()->Init(MaxFramesInFlight);

	Time = std::chrono::high_resolution_clock::now();
	PreviousTime = Time;
}

FRender::~FRender()
{
	Cleanup();

	ACCELERATION_STRUCTURE_SYSTEM()->Terminate();
	MESH_SYSTEM()->Terminate();

	VK_CONTEXT()->CleanUp();
}

void FRender::RegisterExternalOutputs(std::vector<ImagePtr> OutputImagesIn, const std::vector<FSynchronizationPoint>& ExternalImageIsReadyIn)
{
	/// Check whether sizes are the same
	if (OutputImagesIn.size() != ExternalImageIsReadyIn.size())
	{
		throw std::runtime_error("Number of semaphores should be the same as number of images provided!");
	}

	/// Free previous Framebuffers
	for (int i = 0; i < MaxFramesInFlight; ++i)
	{
		if (OutputToFramebufferMap.find(OutputType(i)) != OutputToFramebufferMap.end())
		{
			auto FramebufferComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FFramebufferComponent>(OutputToFramebufferMap[OutputType(i)]);
			TEXTURE_MANAGER()->UnregisterAndFreeFramebuffer(FramebufferComponent.FramebufferImageIndex);
			SetOutput(OutputType(i), ECS::INVALID_ENTITY);
		}
	}

	MaxFramesInFlight = OutputImagesIn.size();

	/// Copy
	ExternalImageAvailable = ExternalImageIsReadyIn;

	///Register new framebuffers  as outputs
	for (int i = 0; i < OutputImagesIn.size(); ++i)
	{
		auto Framebuffer = CreateFramebufferFromExternalImage(OutputImagesIn[i]);
		SetOutput(OutputType(i), Framebuffer);
	}
}

int FRender::Init()
{
	/// If no external Framebuffers provided Create internal ones
	if (ExternalImageAvailable.empty())
	{
		for (int i = 0; i < MaxFramesInFlight; ++i)
		{
			auto Framebuffer = CreateColorAttachment(Width, Height, "Output Image" + std::to_string(i));
			SetOutput(OutputType(i), Framebuffer);
		}
	}

	ImageAvailable.resize(MaxFramesInFlight);

    for (int i = 0; i < MaxFramesInFlight; ++i)
    {
        ImagesInFlight.push_back(VK_CONTEXT()->CreateSignalledFence());
    }

	UpdateTLASTask 						= std::make_shared<FUpdateTLASTask>					(Width, Height, 1, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
	ResetTask							= std::make_shared<FReset>							(Width, Height, 1, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    GenerateRaysTask 					= std::make_shared<FGenerateInitialRays>			(Width, Height, 1, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
	ResetActiveRayCountTask 			= std::make_shared<FResetActiveRayCountTask>		(Width, Height, 1, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    RayTraceTask 						= std::make_shared<FRaytraceTask>					(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ClearMaterialsCountPerChunkTask 	= std::make_shared<FClearMaterialsCountPerChunkTask>(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ClearTotalMaterialsCountTask 		= std::make_shared<FClearTotalMaterialsCountTask>	(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    CountMaterialsPerChunkTask 			= std::make_shared<FCountMaterialsPerChunkTask>		(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ComputePrefixSumsUpSweepTask 		= std::make_shared<FComputePrefixSumsUpSweepTask>	(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ComputePrefixSumsZeroOutTask 		= std::make_shared<FComputePrefixSumsZeroOutTask>	(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ComputePrefixSumsDownSweepTask 		= std::make_shared<FComputePrefixSumsDownSweepTask>	(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ComputeOffsetsPerMaterialTask 		= std::make_shared<FComputeOffsetsPerMaterialTask>	(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    SortMaterialsTask 					= std::make_shared<FSortMaterialsTask>				(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ShadeTask 							= std::make_shared<FShadeTask>						(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    MissTask 							= std::make_shared<FMissTask>						(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
	AdvanceRenderCountTask 				= std::make_shared<FAdvanceRenderCount>				(Width, Height, 1, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    AccumulateTask 						= std::make_shared<FAccumulateTask>					(Width, Height, 1, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ClearImageTask 						= std::make_shared<FClearImageTask>					(Width, Height, 1, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    PassthroughTask 					= std::make_shared<FPassthroughTask>				(Width, Height, 1, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);

    for (int i = 0; i < MaxFramesInFlight; ++i)
    {
        auto& FramebufferComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FFramebufferComponent>(OutputToFramebufferMap[OutputType(i)]);
        PassthroughTask->RegisterOutput(i, TEXTURE_MANAGER()->GetFramebufferImage(FramebufferComponent.FramebufferImageIndex));
    }

    RenderFrameIndex = 0;

    return 0;
}

int FRender::Cleanup()
{
	VK_CONTEXT()->WaitIdle();

	UpdateTLASTask 					= nullptr;
	ResetTask 						= nullptr;
    GenerateRaysTask 				= nullptr;
	ResetActiveRayCountTask 		= nullptr;
    RayTraceTask 					= nullptr;
    ClearMaterialsCountPerChunkTask = nullptr;
    ClearTotalMaterialsCountTask 	= nullptr;
    ComputeOffsetsPerMaterialTask 	= nullptr;
    CountMaterialsPerChunkTask 		= nullptr;
    SortMaterialsTask 				= nullptr;
    ComputePrefixSumsUpSweepTask 	= nullptr;
    ComputePrefixSumsZeroOutTask 	= nullptr;
    ComputePrefixSumsDownSweepTask 	= nullptr;
    ShadeTask 						= nullptr;
    MissTask 						= nullptr;
	AdvanceRenderCountTask			= nullptr;
    AccumulateTask 					= nullptr;
    ClearImageTask 					= nullptr;
    PassthroughTask 				= nullptr;

	ExternalTasks.clear();

	for (int i = 0; i < ImagesInFlight.size(); ++i)
	{
		vkDestroyFence(VK_CONTEXT()->LogicalDevice, ImagesInFlight[i], nullptr);
		ImagesInFlight[i] = VK_NULL_HANDLE;
	}

    ImageAvailable.clear();
    ImagesInFlight.clear();

    return 0;
}

int FRender::SetSize(int WidthIn, int HeightIn)
{
    Width = WidthIn;
    Height = HeightIn;
	bWasResized = true;
	Cleanup();
	CAMERA_SYSTEM()->SetAspectRatio(ActiveCamera, float(Width) / float(Height));
    return 0;
}

ECS::FEntity FRender::CreateCamera()
{
    ECS::FEntity Camera = COORDINATOR().CreateEntity();
    COORDINATOR().AddComponent<ECS::COMPONENTS::FDeviceCameraComponent>(Camera, {});
    COORDINATOR().AddComponent<ECS::COMPONENTS::FCameraComponent>(Camera, {});

    return Camera;
}

ECS::FEntity FRender::CreateFramebuffer(int WidthIn, int HeightIn, const std::string& DebugName)
{
    auto FramebufferImage = TEXTURE_MANAGER()->CreateStorageImage(WidthIn, HeightIn, DebugName);
    static int Counter = 0;
    auto FramebufferImageIndex = TEXTURE_MANAGER()->RegisterFramebuffer(FramebufferImage, (DebugName.empty()) ? ("Unnamed Framebuffer " + std::to_string(Counter++)) : DebugName);

    ECS::FEntity Framebuffer = COORDINATOR().CreateEntity();
    COORDINATOR().AddComponent<ECS::COMPONENTS::FFramebufferComponent>(Framebuffer, {FramebufferImageIndex});

    return Framebuffer;
}

void FRender::DestroyFramebuffer(ECS::FEntity Framebuffer)
{
	auto& FramebufferComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FFramebufferComponent>(Framebuffer);
	TEXTURE_MANAGER()->UnregisterAndFreeFramebuffer(FramebufferComponent.FramebufferImageIndex);
	COORDINATOR().DestroyEntity(Framebuffer);
	return;
}

ECS::FEntity FRender::CreateFramebufferFromExternalImage(ImagePtr ImageIn, const std::string& DebugName)
{
    static int Counter = 0;
    auto FramebufferImageIndex = TEXTURE_MANAGER()->RegisterFramebuffer(ImageIn, (DebugName.empty()) ? ("Unnamed Framebuffer From External Image " + std::to_string(Counter++)) : DebugName);

    ECS::FEntity Framebuffer = COORDINATOR().CreateEntity();
    COORDINATOR().AddComponent<ECS::COMPONENTS::FFramebufferComponent>(Framebuffer, {FramebufferImageIndex});

    return Framebuffer;
}

ECS::FEntity FRender::CreateColorAttachment(int WidthIn, int HeightIn, const std::string& DebugName)
{
	auto FramebufferImage = TEXTURE_MANAGER()->CreateColorAttachment(WidthIn, HeightIn, DebugName);
	static int Counter = 0;
	auto FramebufferImageIndex = TEXTURE_MANAGER()->RegisterFramebuffer(FramebufferImage, (DebugName.empty()) ? ("Unnamed Framebuffer " + std::to_string(Counter++)) : DebugName);

	ECS::FEntity Framebuffer = COORDINATOR().CreateEntity();
	COORDINATOR().AddComponent<ECS::COMPONENTS::FFramebufferComponent>(Framebuffer, {FramebufferImageIndex});

	return Framebuffer;
}

void FRender::SetActiveCamera(ECS::FEntity Camera)
{
    ActiveCamera = Camera;
}

void FRender::SetOutput(OutputType OutputTypeIn, ECS::FEntity Framebuffer)
{
    OutputToFramebufferMap[OutputTypeIn] = Framebuffer;
}

ECS::FEntity FRender::GetOutput(OutputType OutputTypeIn)
{
    return OutputToFramebufferMap[OutputTypeIn];
}

void FRender::SaveFramebuffer(ECS::FEntity Framebuffer, const std::string& Filename)
{
    auto& FramebufferComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FFramebufferComponent>(Framebuffer);
    VK_CONTEXT()->SaveImage(*TEXTURE_MANAGER()->GetFramebufferImage(FramebufferComponent.FramebufferImageIndex), Filename);
}

void FRender::SaveOutput(OutputType OutputTypeIn, const std::string& Filename)
{
	if (OutputToFramebufferMap.find(OutputTypeIn) == OutputToFramebufferMap.end())
	{
		return;
	}

	auto Framebuffer = OutputToFramebufferMap[OutputTypeIn];
	SaveFramebuffer(Framebuffer, Filename);
}

void FRender::GetFramebufferData(ECS::FEntity Framebuffer)
{
    auto& FramebufferComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FFramebufferComponent>(Framebuffer);
    auto Image = TEXTURE_MANAGER()->GetFramebufferImage(FramebufferComponent.FramebufferImageIndex);
    std::vector<char> Data;

    VK_CONTEXT()->FetchImageData(*Image, Data);
}

void FRender::AddExternalTaskAfterRender(std::shared_ptr<FExecutableTask> Task)
{
	ExternalTasks.push_back(Task);
}

FSynchronizationPoint FRender::Render()
{
	return Render(0);
}

FSynchronizationPoint FRender::Render(uint32_t OutputImageIndex)
{
    uint32_t CurrentFrame = RenderFrameIndex % MaxFramesInFlight;
	PreviousTime = Time;
	Time = std::chrono::high_resolution_clock::now();

	UpdateTLASTask->Reload();
	ResetTask->Reload();
	GenerateRaysTask->Reload();
	ResetActiveRayCountTask->Reload();
	RayTraceTask->Reload();
	ClearMaterialsCountPerChunkTask->Reload();
	ClearTotalMaterialsCountTask->Reload();
	CountMaterialsPerChunkTask->Reload();
	ComputePrefixSumsUpSweepTask->Reload();
	ComputePrefixSumsZeroOutTask->Reload();
	ComputePrefixSumsDownSweepTask->Reload();
	ComputeOffsetsPerMaterialTask->Reload();
	SortMaterialsTask->Reload();
	ShadeTask->Reload();
	MissTask->Reload();
	AdvanceRenderCountTask->Reload();
	AccumulateTask->Reload();
	ClearImageTask->Reload();
	PassthroughTask->Reload();

	FSynchronizationPoint SynchronizationPoint = {{}, {ImagesInFlight[CurrentFrame]}, {}, {}};

	/// If we have no external semaphores
	if (ExternalImageAvailable.empty())
	{
		/// We need to wait for the internal one, unless it's the first frame
		if (RenderFrameIndex > 0)
		{
			SynchronizationPoint += ImageAvailable[(RenderFrameIndex - 1) % MaxFramesInFlight];
		}
	}
	else
	{
		SynchronizationPoint += ExternalImageAvailable[OutputImageIndex];
	}

	VkPipelineStageFlags PipelineStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	SynchronizationPoint = UpdateTLASTask->Submit(PipelineStageFlags, SynchronizationPoint, 0, CurrentFrame);

	if (bAnyUpdate || RenderFrameIndex == 0)
	{
		SynchronizationPoint = ResetTask->Submit(PipelineStageFlags, SynchronizationPoint, 0, CurrentFrame);
	}

	SynchronizationPoint = GenerateRaysTask->Submit(PipelineStageFlags, SynchronizationPoint, 0, CurrentFrame);

	SynchronizationPoint = ResetActiveRayCountTask->Submit(PipelineStageFlags, SynchronizationPoint, 0, CurrentFrame);

	for (uint32_t i = 0; i < RecursionDepth; ++i)
	{
		SynchronizationPoint = RayTraceTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);

		SynchronizationPoint = ClearMaterialsCountPerChunkTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);

		SynchronizationPoint = ClearTotalMaterialsCountTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);

		SynchronizationPoint = CountMaterialsPerChunkTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);

		SynchronizationPoint = ComputePrefixSumsUpSweepTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);

		SynchronizationPoint = ComputePrefixSumsZeroOutTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);

		SynchronizationPoint = ComputePrefixSumsDownSweepTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);

		SynchronizationPoint = ComputeOffsetsPerMaterialTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);

		SynchronizationPoint = SortMaterialsTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);

		SynchronizationPoint = ShadeTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);

		SynchronizationPoint = MissTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);
	}

	SynchronizationPoint = AdvanceRenderCountTask->Submit(PipelineStageFlags, SynchronizationPoint, 0, CurrentFrame);

    if (bAnyUpdate)
    {
		SynchronizationPoint = ClearImageTask->Submit(PipelineStageFlags, SynchronizationPoint, 0, CurrentFrame);
    }

	SynchronizationPoint = AccumulateTask->Submit(PipelineStageFlags, SynchronizationPoint, 0, CurrentFrame);

	/// If no external work to be done, then use the internal fence in passthrough
	if (ExternalTasks.empty())
	{
		SynchronizationPoint.FencesToSignal.push_back(ImagesInFlight[CurrentFrame]);
	}

	SynchronizationPoint = PassthroughTask->Submit(PipelineStageFlags, SynchronizationPoint, 0, CurrentFrame);

	if (!ExternalTasks.empty())
	{
		for (uint32_t i = 0; i < ExternalTasks.size() - 1; ++i)
		{
			SynchronizationPoint = ExternalTasks[i]->Submit(PipelineStageFlags, SynchronizationPoint, 0, CurrentFrame);
		}

		SynchronizationPoint.FencesToSignal.push_back(ImagesInFlight[CurrentFrame]);
		SynchronizationPoint = ExternalTasks.back()->Submit(PipelineStageFlags, SynchronizationPoint, 0, CurrentFrame);
	}

	SynchronizationPoint.FencesToWait.push_back(ImagesInFlight[CurrentFrame]);
	ImageAvailable[CurrentFrame] = SynchronizationPoint;

    RenderFrameIndex++;

    return SynchronizationPoint;
}

int FRender::Update()
{
	bAnyUpdate = false;
	bAnyUpdate |= CAMERA_SYSTEM()->Update();
	bAnyUpdate |= TRANSFORM_SYSTEM()->Update();
	bAnyUpdate |= RENDERABLE_SYSTEM()->Update();
	bAnyUpdate |= LIGHT_SYSTEM()->Update();
	bAnyUpdate |= ACCELERATION_STRUCTURE_SYSTEM()->Update();

    if (bWasResized)
    {
        Init();
		bWasResized = false;
    }

    return 0;
}

void FRender::WaitIdle()
{
	VK_CONTEXT()->WaitIdle();
}

void FRender::Wait(FSynchronizationPoint& SynchronizationPoint)
{
	if(!SynchronizationPoint.FencesToWait.empty())
	{
		vkWaitForFences(VK_CONTEXT()->LogicalDevice, SynchronizationPoint.FencesToWait.size(), SynchronizationPoint.FencesToWait.data(), VK_TRUE, UINT64_MAX);
	}
}

ECS::FEntity FRender::CreateTexture(const std::string& FilePath)
{
    return TEXTURE_SYSTEM()->CreateTextureFromFile(FilePath);
}

ECS::FEntity FRender::CreateMaterial(const FVector3& BaseColor)
{
    auto NewMaterial = MATERIAL_SYSTEM()->CreateMaterial();

    MATERIAL_SYSTEM()->SetBaseColor(NewMaterial, BaseColor.X, BaseColor.Y, BaseColor.Z);

    return NewMaterial;
}

ECS::FEntity FRender::ShapeSetMaterial(ECS::FEntity Shape, ECS::FEntity Material)
{
    RENDERABLE_SYSTEM()->SetMaterial(Shape, Material);

    return Shape;
}

void FRender::MaterialSetBaseColor(ECS::FEntity Material, ECS::FEntity Image)
{
    MATERIAL_SYSTEM()->SetBaseColor(Material, Image);
}

void FRender::MaterialSetBaseColor(ECS::FEntity Material, const FVector3& Value)
{
    MATERIAL_SYSTEM()->SetBaseColor(Material, Value.X, Value.Y, Value.Z);
}

void FRender::MaterialSetDiffuseRoughness(ECS::FEntity Material, ECS::FEntity Image)
{
    MATERIAL_SYSTEM()->SetDiffuseRoughness(Material, Image);
}

void FRender::MaterialSetDiffuseRoughness(ECS::FEntity Material, float Value)
{
    MATERIAL_SYSTEM()->SetDiffuseRoughness(Material, Value);
}

void FRender::MaterialSetNormal(ECS::FEntity Material, const FVector3& Value)
{
    MATERIAL_SYSTEM()->SetNormal(Material, Value);
}

void FRender::MaterialSetNormal(ECS::FEntity Material, ECS::FEntity Image)
{
    MATERIAL_SYSTEM()->SetNormal(Material, Image);
}

int FRender::SetIBL(const std::string& Path)
{
    ImagePtr IBLImage = VK_CONTEXT()->CreateEXRImageFromFile(Path, "V::IBL_Image");
    IBLImage->Transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	TEXTURE_MANAGER()->RegisterTexture(IBLImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, "IBL Image");
	MissTask->SetDirty(OUTDATED_DESCRIPTOR_SET | OUTDATED_COMMAND_BUFFER);

    return 0;
}

void FRender::GetAllTimings(std::vector<std::string>& Names, std::vector<std::vector<float>>& Timings, float& FrameTime, uint32_t FrameIndex)
{
	Names.clear();
	Timings.clear();

	FrameTime = std::chrono::duration<float, std::chrono::milliseconds ::period>(Time - PreviousTime).count();

	Names.push_back(UpdateTLASTask->Name);
	Timings.push_back(UpdateTLASTask->RequestTiming(FrameIndex));

	Names.push_back(GenerateRaysTask->Name);
	Timings.push_back(GenerateRaysTask->RequestTiming(FrameIndex));

	Names.push_back(ResetActiveRayCountTask->Name);
	Timings.push_back(ResetActiveRayCountTask->RequestTiming(FrameIndex));

	Names.push_back(RayTraceTask->Name);
	Timings.push_back(RayTraceTask->RequestTiming(FrameIndex));

	Names.push_back(ClearMaterialsCountPerChunkTask->Name);
	Timings.push_back(ClearMaterialsCountPerChunkTask->RequestTiming(FrameIndex));

	Names.push_back(ClearTotalMaterialsCountTask->Name);
	Timings.push_back(ClearTotalMaterialsCountTask->RequestTiming(FrameIndex));

	Names.push_back(ComputeOffsetsPerMaterialTask->Name);
	Timings.push_back(ComputeOffsetsPerMaterialTask->RequestTiming(FrameIndex));

	Names.push_back(CountMaterialsPerChunkTask->Name);
	Timings.push_back(CountMaterialsPerChunkTask->RequestTiming(FrameIndex));

	Names.push_back(SortMaterialsTask->Name);
	Timings.push_back(SortMaterialsTask->RequestTiming(FrameIndex));

	Names.push_back(ComputePrefixSumsDownSweepTask->Name);
	Timings.push_back(ComputePrefixSumsDownSweepTask->RequestTiming(FrameIndex));

	Names.push_back(ComputePrefixSumsUpSweepTask->Name);
	Timings.push_back(ComputePrefixSumsUpSweepTask->RequestTiming(FrameIndex));

	Names.push_back(ComputePrefixSumsZeroOutTask->Name);
	Timings.push_back(ComputePrefixSumsZeroOutTask->RequestTiming(FrameIndex));

	Names.push_back(ShadeTask->Name);
	Timings.push_back(ShadeTask->RequestTiming(FrameIndex));

	Names.push_back(MissTask->Name);
	Timings.push_back(MissTask->RequestTiming(FrameIndex));

	Names.push_back(AccumulateTask->Name);
	Timings.push_back(AccumulateTask->RequestTiming(FrameIndex));

	Names.push_back(PassthroughTask->Name);
	Timings.push_back(PassthroughTask->RequestTiming(FrameIndex));

	Names.push_back(ClearImageTask->Name);
	Timings.push_back(ClearImageTask->RequestTiming(FrameIndex));

	Names.push_back(ResetTask->Name);
	Timings.push_back(ResetTask->RequestTiming(FrameIndex));

	Names.push_back(AdvanceRenderCountTask->Name);
	Timings.push_back(AdvanceRenderCountTask->RequestTiming(FrameIndex));

	for (auto& Task : ExternalTasks)
	{
		Names.push_back(Task->Name);
		Timings.push_back(Task->RequestTiming(FrameIndex));
	}
}

ECS::FEntity FRender::CreatePlane(const FVector2& Size)
{
    auto NewModel = CreateEmptyModel();

    MESH_SYSTEM()->CreatePlane(NewModel, Size);
    MESH_SYSTEM()->LoadToGPU(NewModel);
    MESH_SYSTEM()->GenerateBLAS(NewModel);

    return NewModel;
}

ECS::FEntity FRender::CreateCube()
{
    auto NewModel = CreateEmptyModel();

    MESH_SYSTEM()->CreateHexahedron(NewModel);
    MESH_SYSTEM()->LoadToGPU(NewModel);
    MESH_SYSTEM()->GenerateBLAS(NewModel);

    return NewModel;
}

ECS::FEntity FRender::CreateIcosahedronSphere(float Radius, int LevelOfComplexity, bool bJagged)
{
    auto NewModel = CreateEmptyModel();

    MESH_SYSTEM()->CreateIcosahedron(NewModel, Radius, LevelOfComplexity, bJagged);
    MESH_SYSTEM()->LoadToGPU(NewModel);
    MESH_SYSTEM()->GenerateBLAS(NewModel);

    return NewModel;
}

ECS::FEntity FRender::CreateUVSphere(uint32_t LongitudeCount, uint32_t LatitudeCount)
{
	auto NewModel = CreateEmptyModel();

	MESH_SYSTEM()->CreateUVSphere(NewModel, LongitudeCount, LatitudeCount);
	MESH_SYSTEM()->LoadToGPU(NewModel);
	MESH_SYSTEM()->GenerateBLAS(NewModel);

	return NewModel;
}

ECS::FEntity FRender::CreateModel(const std::string& Path)
{
    auto NewModel = CreateEmptyModel();

    MESH_SYSTEM()->LoadMesh(NewModel, Path);
    MESH_SYSTEM()->LoadToGPU(NewModel);
    MESH_SYSTEM()->GenerateBLAS(NewModel);

    return NewModel;
}

ECS::FEntity FRender::CreatePyramid()
{
    auto NewModel = CreateEmptyModel();

    MESH_SYSTEM()->CreateTetrahedron(NewModel);
    MESH_SYSTEM()->LoadToGPU(NewModel);
    MESH_SYSTEM()->GenerateBLAS(NewModel);

    return NewModel;
}

ECS::FEntity FRender::CreateInstance(ECS::FEntity BaseModel, const FVector3& Position, const FVector3& Direction, const FVector3& Up)
{
    auto MeshInstance = ACCELERATION_STRUCTURE_SYSTEM()->CreateInstance(BaseModel, Position, Direction, Up);
    RENDERABLE_SYSTEM()->SetRenderableDeviceAddress(MeshInstance, MESH_SYSTEM()->GetVertexBufferAddress(MeshInstance), MESH_SYSTEM()->GetIndexBufferAddress(MeshInstance));
	TRANSFORM_SYSTEM()->SetTransform(MeshInstance, Position, Direction, Up);
    RENDERABLE_SYSTEM()->SyncTransform(MeshInstance);
    auto& MeshComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FMeshComponent>(BaseModel);

    if (MeshComponent.Indexed)
    {
        RENDERABLE_SYSTEM()->SetIndexed(MeshInstance);
    }

    return MeshInstance;
}

void FRender::SetInstancePosition(ECS::FEntity Instance, const FVector3& Position)
{
	auto& TransformComponent = TRANSFORM_SYSTEM()->GetComponent<ECS::COMPONENTS::FTransformComponent>(Instance);
	TRANSFORM_SYSTEM()->SetTransform(Instance, Position, TransformComponent.Direction, TransformComponent.Up);
	ACCELERATION_STRUCTURE_SYSTEM()->UpdateInstancePosition(Instance);
}

FVector3 FRender::GetInstancePosition(ECS::FEntity Instance)
{
	return TRANSFORM_SYSTEM()->GetComponent<ECS::COMPONENTS::FTransformComponent>(Instance).Position;
}

ECS::FEntity FRender::CreateLight(const FVector3& Position)
{
    auto Light = COORDINATOR().CreateEntity();
    COORDINATOR().AddComponent<ECS::COMPONENTS::FLightComponent>(Light, {});
    LIGHT_SYSTEM()->SetLightPosition(Light, Position.X, Position.Y, Position.Z);

    return Light;
}

void FRender::SetLightPosition(ECS::FEntity Light, const FVector3& Position)
{
	LIGHT_SYSTEM()->SetLightPosition(Light, Position);
}

FVector3 FRender::GetLightPosition(ECS::FEntity Light)
{
	return COORDINATOR().GetComponent<ECS::COMPONENTS::FLightComponent>(Light).Position;
}

ECS::FEntity FRender::CreateEmptyModel()
{
    ECS::FEntity EmptyModel = COORDINATOR().CreateEntity();
    COORDINATOR().AddComponent<ECS::COMPONENTS::FMeshComponent>(EmptyModel, {});
    COORDINATOR().AddComponent<ECS::COMPONENTS::FDeviceMeshComponent>(EmptyModel, {});

    return EmptyModel;
}
