#include "coordinator.h"

#include "acceleration_structure_system.h"
#include "area_light_system.h"
#include "mesh_system.h"
#include "transform_system.h"
#include "renderable_system.h"
#include "camera_system.h"
#include "point_light_system.h"
#include "directional_light_system.h"
#include "spot_light_system.h"
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
#include "texture_component.h"
#include "material_system.h"

#include "render.h"
#include "texture_manager.h"

#include "utils.h"

#include "renderer_options.h"

#include "logging.h"

FRender::FRender(uint32_t WidthIn, uint32_t HeightIn) : Width(WidthIn), Height(HeightIn)
{
    COORDINATOR().Init();

    /// Register components
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FAccelerationStructureComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FAreaLightComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FDeviceCameraComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FDeviceMeshComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FDeviceRenderableComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FDeviceTransformComponent>();
	COORDINATOR().RegisterComponent<ECS::COMPONENTS::FDirectionalLightComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FMeshComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FMeshInstanceComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FPointLightComponent>();
	COORDINATOR().RegisterComponent<ECS::COMPONENTS::FSpotLightComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FTransformComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FTextureComponent>();
    COORDINATOR().RegisterComponent<ECS::COMPONENTS::FFramebufferComponent>();

    /// Register systems
    auto AreaLightSystem = COORDINATOR().RegisterSystem<ECS::SYSTEMS::FAreaLightSystem>();
    auto CameraSystem = COORDINATOR().RegisterSystem<ECS::SYSTEMS::FCameraSystem>();
    auto TransformSystem = COORDINATOR().RegisterSystem<ECS::SYSTEMS::FTransformSystem>();
    auto RenderableSystem = COORDINATOR().RegisterSystem<ECS::SYSTEMS::FRenderableSystem>();
    auto MaterialSystem = COORDINATOR().RegisterSystem<ECS::SYSTEMS::FMaterialSystem>();
    auto MeshSystem = COORDINATOR().RegisterSystem<ECS::SYSTEMS::FMeshSystem>();
    auto PointLightSystem = COORDINATOR().RegisterSystem<ECS::SYSTEMS::FPointLightSystem>();
	auto DirectionalLightSystem = COORDINATOR().RegisterSystem<ECS::SYSTEMS::FDirectionalLightSystem>();
	auto SpotLightSystem = COORDINATOR().RegisterSystem<ECS::SYSTEMS::FSpotLightSystem>();
    auto AccelerationSystem = COORDINATOR().RegisterSystem<ECS::SYSTEMS::FAccelerationStructureSystem>();
    auto TextureSystem = COORDINATOR().RegisterSystem<ECS::SYSTEMS::FTextureSystem>();

    /// Set camera system signature
    ECS::FSignature CameraSystemSignature;
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
    MaterialSignature.set(COORDINATOR().GetComponentType<ECS::COMPONENTS::FOpenPBRSurfaceComponent>());
    COORDINATOR().SetSystemSignature<ECS::SYSTEMS::FMaterialSystem>(MaterialSignature);

    /// Register Mesh systems signature
    ECS::FSignature MeshSignature;
    MeshSignature.set(COORDINATOR().GetComponentType<ECS::COMPONENTS::FMeshComponent>());
    MeshSignature.set(COORDINATOR().GetComponentType<ECS::COMPONENTS::FDeviceMeshComponent>());
    MeshSignature.set(COORDINATOR().GetComponentType<ECS::COMPONENTS::FAccelerationStructureComponent>());
    COORDINATOR().SetSystemSignature<ECS::SYSTEMS::FMeshSystem>(MeshSignature);

    /// Register Point Light system signature
    ECS::FSignature PointLightSignature;
	PointLightSignature.set(COORDINATOR().GetComponentType<ECS::COMPONENTS::FPointLightComponent>());
    COORDINATOR().SetSystemSignature<ECS::SYSTEMS::FPointLightSystem>(PointLightSignature);

	/// Register Directional Light system signature
	ECS::FSignature DirectionalLightSignature;
	DirectionalLightSignature.set(COORDINATOR().GetComponentType<ECS::COMPONENTS::FDirectionalLightComponent>());
	COORDINATOR().SetSystemSignature<ECS::SYSTEMS::FDirectionalLightSystem>(DirectionalLightSignature);

	/// Register Spot Light system signature
	ECS::FSignature SpotLightSignature;
	SpotLightSignature.set(COORDINATOR().GetComponentType<ECS::COMPONENTS::FSpotLightComponent>());
	COORDINATOR().SetSystemSignature<ECS::SYSTEMS::FSpotLightSystem>(SpotLightSignature);

    /// Register Area Light system signature
    ECS::FSignature AreaLightSignature;
    AreaLightSignature.set(COORDINATOR().GetComponentType<ECS::COMPONENTS::FAreaLightComponent>());
    COORDINATOR().SetSystemSignature<ECS::SYSTEMS::FAreaLightSystem>(AreaLightSignature);

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
	MATERIAL_SYSTEM()->Init();
	POINT_LIGHT_SYSTEM()->Init(MaxFramesInFlight);
	DIRECTIONAL_LIGHT_SYSTEM()->Init(MaxFramesInFlight);
    SPOT_LIGHT_SYSTEM()->Init(MaxFramesInFlight);
    AREA_LIGHT_SYSTEM()->Init(MaxFramesInFlight);
    TRANSFORM_SYSTEM()->Init(MaxFramesInFlight);
    ACCELERATION_STRUCTURE_SYSTEM()->Init(MaxFramesInFlight);

	AllocateIndependentResources();

	Time = std::chrono::high_resolution_clock::now();
	PreviousTime = Time;
}

FRender::~FRender()
{
	Cleanup();

	for (int i = 0; i < ImagesInFlight.size(); ++i)
	{
		vkDestroyFence(VK_CONTEXT()->LogicalDevice, ImagesInFlight[i], nullptr);
		ImagesInFlight[i] = VK_NULL_HANDLE;
	}

	FreeIndependentResources();

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

	/// Free previously registered Framebuffers
	for (int i = 0; i < OutputFramebuffers.size(); ++i)
	{
		auto FramebufferComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FFramebufferComponent>(OutputFramebuffers[i]);
		TEXTURE_MANAGER()->UnregisterAndFreeFramebuffer(FramebufferComponent.FramebufferName);
		OutputFramebuffers[i] = ECS::INVALID_ENTITY;
	}

	OutputFramebuffers.clear();

	MaxFramesInFlight = OutputImagesIn.size();

	/// Copy
	ExternalImageAvailable = ExternalImageIsReadyIn;

	///Register new framebuffers  as outputs
	for (int i = 0; i < OutputImagesIn.size(); ++i)
	{
		OutputFramebuffers.emplace_back(CreateFramebufferFromExternalImage(OutputImagesIn[i]));
	}
}

int FRender::Init()
{
	/// If no external Framebuffers provided Create internal ones
	if (ExternalImageAvailable.empty())
	{
		OutputFramebuffers.resize(MaxFramesInFlight);

		for (int i = 0; i < MaxFramesInFlight; ++i)
		{
			auto Framebuffer = CreateColorAttachment(Width, Height, "Output Image" + std::to_string(i));
			OutputFramebuffers[i] = Framebuffer;
		}
	}

	ImageAvailable.resize(MaxFramesInFlight);

    for (int i = 0; i < MaxFramesInFlight; ++i)
    {
        ImagesInFlight.push_back(VK_CONTEXT()->CreateSignalledFence());
    }

	AllocateDependentResources();

	/// Create all required tasks
	UpdateTLASTask 						= std::make_shared<FUpdateTLASTask>					(Width, Height, 1, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
	ResetRenderIterations				= std::make_shared<FClearBufferTask>				(RENDER_ITERATION_BUFFER, 			Width, Height, 1, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ClearImageTask 						= std::make_shared<FClearImageTask>					("AccumulatorImage", 				Width, Height, 1, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
	ClearCumulativeMaterialColorBuffer	= std::make_shared<FClearBufferTask>				(CUMULATIVE_MATERIAL_COLOR_BUFFER, 		Width, Height, 1, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice, 0x3F800000);
	ResetActiveRayCountTask 			= std::make_shared<FResetActiveRayCountTask>		(Width, Height, 1, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
	std::vector<std::string> BuffersToCleanEachBounce{COUNTED_MATERIALS_PER_CHUNK_BUFFER};
	ClearBuffersEachBounceTask 			= std::make_shared<FClearBufferTask>				(BuffersToCleanEachBounce , Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    RayTraceTask 						= std::make_shared<FRaytraceTask>					(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ClearTotalMaterialsCountTask 		= std::make_shared<FClearTotalMaterialsCountTask>	(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    CountMaterialsPerChunkTask 			= std::make_shared<FCountMaterialsPerChunkTask>		(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ComputePrefixSumsUpSweepTask 		= std::make_shared<FComputePrefixSumsUpSweepTask>	(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ComputePrefixSumsZeroOutTask 		= std::make_shared<FComputePrefixSumsZeroOutTask>	(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ComputePrefixSumsDownSweepTask 		= std::make_shared<FComputePrefixSumsDownSweepTask>	(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ComputeOffsetsPerMaterialTask 		= std::make_shared<FComputeOffsetsPerMaterialTask>	(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    SortMaterialsTask 					= std::make_shared<FSortMaterialsTask>				(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
	MasterShader 						= std::make_shared<FMasterShader>					(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    MissTask 							= std::make_shared<FMissTask>						(Width, Height, RecursionDepth, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    AccumulateTask 						= std::make_shared<FAccumulateTask>					(Width, Height, 1, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    PassthroughTask 					= std::make_shared<FPassthroughTask>				(Width, Height, 1, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
	AdvanceRenderCountTask 				= std::make_shared<FAdvanceRenderCount>				(Width, Height, 1, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);

    for (int i = 0; i < MaxFramesInFlight; ++i)
    {
        auto& FramebufferComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FFramebufferComponent>(OutputFramebuffers[i]);
        PassthroughTask->RegisterOutput("PassthroughOutput" + std::to_string(i), TEXTURE_MANAGER()->GetFramebufferImage(FramebufferComponent.FramebufferName));
    }

    RenderFrameIndex = 0;
	Counter = 0;
    return 0;
}

int FRender::Cleanup()
{
	VK_CONTEXT()->WaitIdle();

	FreeDependentResources();

	UpdateTLASTask 						= nullptr;
	ResetRenderIterations				= nullptr;
    ClearImageTask 						= nullptr;
	ClearCumulativeMaterialColorBuffer	= nullptr;
	ResetActiveRayCountTask 			= nullptr;
	ClearBuffersEachBounceTask			= nullptr;
	ClearBuffersEachBounceTask			= nullptr;
    RayTraceTask 						= nullptr;
    ClearTotalMaterialsCountTask 		= nullptr;
    CountMaterialsPerChunkTask 			= nullptr;
    ComputePrefixSumsUpSweepTask 		= nullptr;
    ComputePrefixSumsZeroOutTask 		= nullptr;
    ComputePrefixSumsDownSweepTask 		= nullptr;
    ComputeOffsetsPerMaterialTask 		= nullptr;
    SortMaterialsTask 					= nullptr;
	MasterShader						= nullptr;
    MissTask 							= nullptr;
    AccumulateTask 						= nullptr;
    PassthroughTask 					= nullptr;
	AdvanceRenderCountTask				= nullptr;

	ExternalTasks.clear();

    ImageAvailable.clear();
    ImagesInFlight.clear();

    return 0;
}

int FRender::SetSize(uint32_t WidthIn, uint32_t HeightIn)
{
    Width = WidthIn;
    Height = HeightIn;
	bWasResized = true;
	Cleanup();
    return 0;
}

void FRender::SetRenderTarget(EOutputType OutputType)
{
	if (RENDER_STATE()->RenderTarget != OutputType)
	{
		RENDER_STATE()->RenderTarget = OutputType;
		AccumulateTask->SetDirty(OUTDATED_DESCRIPTOR_SET | OUTDATED_COMMAND_BUFFER);
		RESOURCE_ALLOCATOR()->LoadDataToBuffer(UTILITY_INFO_BUFFER, sizeof(uint32_t),  offsetof(FUtilityData, AOVIndex), &RENDER_STATE()->RenderTarget);
		bAnyUpdate = true;
	}
}

void FRender::SetAccumulateFrames(bool bAccumulateFrames)
{
	uint32_t Value = bAccumulateFrames ? 1 : 0;
	RESOURCE_ALLOCATOR()->LoadDataToBuffer(UTILITY_INFO_BUFFER, sizeof(uint32_t),  offsetof(FUtilityData, AccumulateFrames), &Value);
	bAnyUpdate = true;
}

void FRender::SetAccumulateBounces(bool bAccumulateBounces)
{
	uint32_t Value = bAccumulateBounces ? 1 : 0;
	RESOURCE_ALLOCATOR()->LoadDataToBuffer(UTILITY_INFO_BUFFER, sizeof(uint32_t),  offsetof(FUtilityData, AccumulateBounces), &Value);
	bAnyUpdate = true;
}

ECS::FEntity FRender::CreateCamera()
{
    ECS::FEntity Camera = COORDINATOR().CreateEntity();
    COORDINATOR().AddComponent<ECS::COMPONENTS::FDeviceCameraComponent>(Camera, {});

    return Camera;
}

ECS::FEntity FRender::CreateFramebuffer(int WidthIn, int HeightIn, const std::string& DebugName)
{
    auto FramebufferImage = TEXTURE_MANAGER()->CreateStorageImage(WidthIn, HeightIn, VK_FORMAT_R32G32B32A32_SFLOAT, DebugName);
    static int Counter = 0;
	std::string FramebufferName = (DebugName.empty()) ? ("Unnamed Framebuffer " + std::to_string(Counter++)) : DebugName;
    TEXTURE_MANAGER()->RegisterFramebuffer(FramebufferImage, FramebufferName);

    ECS::FEntity Framebuffer = COORDINATOR().CreateEntity();
    COORDINATOR().AddComponent<ECS::COMPONENTS::FFramebufferComponent>(Framebuffer, {FramebufferName});

    return Framebuffer;
}

void FRender::DestroyFramebuffer(ECS::FEntity Framebuffer)
{
	auto& FramebufferComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FFramebufferComponent>(Framebuffer);
	TEXTURE_MANAGER()->UnregisterAndFreeFramebuffer(FramebufferComponent.FramebufferName);
	COORDINATOR().DestroyEntity(Framebuffer);
	return;
}

ECS::FEntity FRender::CreateFramebufferFromExternalImage(ImagePtr ImageIn, const std::string& DebugName)
{
    static int Counter = 0;
	std::string FramebufferName = (DebugName.empty()) ? ("Unnamed Framebuffer " + std::to_string(Counter++)) : DebugName;
    TEXTURE_MANAGER()->RegisterFramebuffer(ImageIn, FramebufferName);

    ECS::FEntity Framebuffer = COORDINATOR().CreateEntity();
    COORDINATOR().AddComponent<ECS::COMPONENTS::FFramebufferComponent>(Framebuffer, {FramebufferName});

    return Framebuffer;
}

ECS::FEntity FRender::CreateColorAttachment(int WidthIn, int HeightIn, const std::string& DebugName)
{
	auto FramebufferImage = TEXTURE_MANAGER()->CreateColorAttachment(WidthIn, HeightIn, DebugName);
	static int Counter = 0;
	std::string FramebufferName = (DebugName.empty()) ? ("Unnamed Framebuffer " + std::to_string(Counter++)) : DebugName;
	TEXTURE_MANAGER()->RegisterFramebuffer(FramebufferImage, FramebufferName);

	ECS::FEntity Framebuffer = COORDINATOR().CreateEntity();
	COORDINATOR().AddComponent<ECS::COMPONENTS::FFramebufferComponent>(Framebuffer, {FramebufferName});

	return Framebuffer;
}

void FRender::SetActiveCamera(ECS::FEntity Camera)
{
    ActiveCamera = Camera;
}

void FRender::SetCameraPosition(const FVector3& Position, const std::optional<FVector3>& Direction, const std::optional<FVector3>& Up, const std::optional<ECS::FEntity>& Camera)
{
	if (Camera)
	{
		CAMERA_SYSTEM()->SetPosition(Camera.value(), Position, Direction, Up);
	}
	else
	{
		CAMERA_SYSTEM()->SetPosition(ActiveCamera, Position, Direction, Up);
	}
}

void FRender::SetCameraSensorProperties(const std::optional<float>& SensorSizeX, const std::optional<float>& SensorSizeY, const std::optional<float>& FocalDistance, const std::optional<ECS::FEntity>& Camera)
{
	if (Camera)
	{
		CAMERA_SYSTEM()->SetCameraSensorProperties(Camera.value(), SensorSizeX, SensorSizeY, FocalDistance);
	}
	else
	{
		CAMERA_SYSTEM()->SetCameraSensorProperties(ActiveCamera, SensorSizeX, SensorSizeY, FocalDistance);
	}
}

void FRender::GetCameraPosition(FVector3* Position, FVector3* Direction, FVector3* Up, const std::optional<ECS::FEntity>& Camera)
{
	if (Camera)
	{
		auto& DeviceCameraComponent = CAMERA_SYSTEM()->GetComponent<ECS::COMPONENTS::FDeviceCameraComponent>(Camera.value());
		*Position = DeviceCameraComponent.Origin;
		*Direction = DeviceCameraComponent.Direction;
		*Up = DeviceCameraComponent.Up;
	}
	else
	{
		auto& DeviceCameraComponent = CAMERA_SYSTEM()->GetComponent<ECS::COMPONENTS::FDeviceCameraComponent>(ActiveCamera);
		*Position = DeviceCameraComponent.Origin;
		*Direction = DeviceCameraComponent.Direction;
		*Up = DeviceCameraComponent.Up;
	}
}

void FRender::GetCameraSensorProperties(float* SensorSizeX, float* SensorSizeY, float* FocalDistance, const std::optional<ECS::FEntity>& Camera)
{
	if (Camera)
	{
		auto& DeviceCameraComponent = CAMERA_SYSTEM()->GetComponent<ECS::COMPONENTS::FDeviceCameraComponent>(Camera.value());
		*SensorSizeX = DeviceCameraComponent.SensorSizeX;
		*SensorSizeY = DeviceCameraComponent.SensorSizeY;
		*FocalDistance = DeviceCameraComponent.FocalDistance;
	}
	else
	{
		auto& DeviceCameraComponent = CAMERA_SYSTEM()->GetComponent<ECS::COMPONENTS::FDeviceCameraComponent>(ActiveCamera);
		*SensorSizeX = DeviceCameraComponent.SensorSizeX;
		*SensorSizeY = DeviceCameraComponent.SensorSizeY;
		*FocalDistance = DeviceCameraComponent.FocalDistance;
	}
}

void FRender::SaveFramebufferPng(ECS::FEntity Framebuffer, const std::string& Filename)
{
    auto& FramebufferComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FFramebufferComponent>(Framebuffer);
    VK_CONTEXT()->SaveImagePng(FramebufferComponent.FramebufferName, Filename);
}

void FRender::SaveFramebufferExr(ECS::FEntity Framebuffer, const std::string& Filename)
{
	auto& FramebufferComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FFramebufferComponent>(Framebuffer);
	VK_CONTEXT()->SaveImageExr(FramebufferComponent.FramebufferName, Filename);
}

void FRender::SaveOutputPng(EOutputType OutputType, const std::string& Filename)
{
	std::string ImageName = "ColorImage";

	if (OutputType < EOutputType::Max)
	{
		if (OutputType > EOutputType::Color)
		{
			ImageName = "AOVImage";
		}
	}
	else
	{
		throw std::runtime_error("Requested to save output that didn't exists.");
	}

	VK_CONTEXT()->SaveImagePng(ImageName, Filename);
}

void FRender::SaveOutputExr(EOutputType OutputType, const std::string& Filename)
{
	std::string ImageName = "ColorImage";

	if (OutputType < EOutputType::Max)
	{
		if (OutputType > EOutputType::Color)
		{
			ImageName = "AOVImage";
		}
	}
	else
	{
		throw std::runtime_error("Requested to save output that didn't exists.");
	}

	VK_CONTEXT()->SaveImageExr(ImageName, Filename);
}

void FRender::PrintScreenPng(const std::string& Filename)
{
	VK_CONTEXT()->SaveImagePng("EstimatedImage", Filename);
}

void FRender::PrintScreenExr(const std::string& Filename)
{
	VK_CONTEXT()->SaveImageExr("EstimatedImage", Filename);
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
	ResetRenderIterations->Reload();
	ClearImageTask->Reload();
	ClearCumulativeMaterialColorBuffer->Reload();
	ResetActiveRayCountTask->Reload();
	RayTraceTask->Reload();
	ClearBuffersEachBounceTask->Reload();
	ClearTotalMaterialsCountTask->Reload();
	CountMaterialsPerChunkTask->Reload();
	ComputePrefixSumsUpSweepTask->Reload();
	ComputePrefixSumsZeroOutTask->Reload();
	ComputePrefixSumsDownSweepTask->Reload();
	ComputeOffsetsPerMaterialTask->Reload();
	SortMaterialsTask->Reload();
	FCompileDefinitions CompileDefinitions;
	CompileDefinitions.Push("LAST_BOUNCE", std::to_string(RecursionDepth - 1));
	CompileDefinitions.Push("LAST_DIFFUSE_BOUNCE", std::to_string(DiffuseRecursionDepth - 1));
	CompileDefinitions.Push("LAST_REFLECTION_BOUNCE", std::to_string(ReflectionRecursionDepth - 1));
	CompileDefinitions.Push("LAST_REFRACTION_BOUNCE", std::to_string(RefractionRecursionDepth - 1));
	MasterShader->Reload(&CompileDefinitions);
	MissTask->Reload();
	AccumulateTask->Reload();
	PassthroughTask->Reload();
	AdvanceRenderCountTask->Reload();

	for (const auto & ExternalTask : ExternalTasks)
	{
		ExternalTask->Reload();
	}


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
		SynchronizationPoint = ResetRenderIterations->Submit(PipelineStageFlags, SynchronizationPoint, 0, CurrentFrame);

		SynchronizationPoint = ClearImageTask->Submit(PipelineStageFlags, SynchronizationPoint, 0, CurrentFrame);
	}

	SynchronizationPoint = ClearCumulativeMaterialColorBuffer->Submit(PipelineStageFlags, SynchronizationPoint, 0, CurrentFrame);

	//WaitIdle();
	//auto BufferData = RESOURCE_ALLOCATOR()->DebugGetDataFromBuffer<float>("DebugBuffer");
	//VK_CONTEXT()->SaveEXRWrapper(BufferData.data(), Width, Height, 4, false, "DebugBuffer.exr");

	SynchronizationPoint = ResetActiveRayCountTask->Submit(PipelineStageFlags, SynchronizationPoint, 0, CurrentFrame);

	for (uint32_t i = 0; i < RecursionDepth; ++i)
	{
		SynchronizationPoint = ClearBuffersEachBounceTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);

		SynchronizationPoint = RayTraceTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);

		SynchronizationPoint = ClearTotalMaterialsCountTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);

		SynchronizationPoint = CountMaterialsPerChunkTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);

		SynchronizationPoint = ComputePrefixSumsUpSweepTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);

		SynchronizationPoint = ComputePrefixSumsZeroOutTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);

		SynchronizationPoint = ComputePrefixSumsDownSweepTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);

		SynchronizationPoint = ComputeOffsetsPerMaterialTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);

		SynchronizationPoint = SortMaterialsTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);

		SynchronizationPoint = MissTask->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);

		SynchronizationPoint = MasterShader->Submit(PipelineStageFlags, SynchronizationPoint, i, CurrentFrame);
	}

	SynchronizationPoint = AccumulateTask->Submit(PipelineStageFlags, SynchronizationPoint, 0, CurrentFrame);

	SynchronizationPoint = AdvanceRenderCountTask->Submit(PipelineStageFlags, SynchronizationPoint, 0, CurrentFrame);

	/// If no external work to be done, then use the internal fence in passthrough
	if (ExternalTasks.empty())
	{
		SynchronizationPoint.FencesToSignal.push_back(ImagesInFlight[CurrentFrame]);
	}

	SynchronizationPoint = PassthroughTask->Submit(PipelineStageFlags, SynchronizationPoint, 0, CurrentFrame);

	//if (Counter == 256)
	//{
	//	WaitIdle();
	//	PrintScreenPng("Estimated_" + std::to_string(Counter));
	//	PrintScreenExr("Estimated_" + std::to_string(Counter));
	//}

	//if (Counter == 8)
	//{
	//	WaitIdle();
	//	exit(0);
	//}

	bAnyUpdate = false;

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
	Counter++;

    return SynchronizationPoint;
}

int FRender::Update()
{
	bAnyUpdate |= CAMERA_SYSTEM()->Update();
	bAnyUpdate |= TRANSFORM_SYSTEM()->Update();
	bAnyUpdate |= RENDERABLE_SYSTEM()->Update();
	bAnyUpdate |= POINT_LIGHT_SYSTEM()->Update();
	bAnyUpdate |= SPOT_LIGHT_SYSTEM()->Update();
	bAnyUpdate |= DIRECTIONAL_LIGHT_SYSTEM()->Update();
	bAnyUpdate |= SPOT_LIGHT_SYSTEM()->Update();
    bAnyUpdate |= AREA_LIGHT_SYSTEM()->Update();
	bAnyUpdate |= ACCELERATION_STRUCTURE_SYSTEM()->Update();
	bAnyUpdate |= MATERIAL_SYSTEM()->Update();

	Counter = bAnyUpdate ? 0 : Counter;

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

ECS::FEntity FRender::CreateEmptyMaterial(const std::string& MaterialName)
{
	auto NewMaterial = MATERIAL_SYSTEM()->CreateEmptyMaterial(MaterialName);
	return NewMaterial;
}

ECS::FEntity FRender::CreateMaterial(const std::string& MaterialName)
{
    auto NewMaterial = MATERIAL_SYSTEM()->CreateDefaultMaterial(MaterialName);
    return NewMaterial;
}

ECS::FEntity FRender::CreateDiffuseMaterial(const FVector3& BaseColor, const std::string& MaterialName)
{
	auto NewMaterial = MATERIAL_SYSTEM()->CreateEmptyMaterial(MaterialName);

	MATERIAL_SYSTEM()->SetF(NewMaterial, MATERIAL_BASE_WEIGHT, 1.f);
	MATERIAL_SYSTEM()->Set3F(NewMaterial, MATERIAL_BASE_COLOR, BaseColor);
	MATERIAL_SYSTEM()->Set3F(NewMaterial, MATERIAL_GEOMETRY_NORMAL, {0, 1, 0});

	return NewMaterial;
}

ECS::FEntity FRender::CreateReflectiveMaterial(const FVector3& BaseColor, const std::string& MaterialName)
{
	auto NewMaterial = MATERIAL_SYSTEM()->CreateEmptyMaterial(MaterialName);

	MATERIAL_SYSTEM()->SetF(NewMaterial, MATERIAL_SPECULAR_WEIGHT, 1.f);
	MATERIAL_SYSTEM()->Set3F(NewMaterial, MATERIAL_SPECULAR_COLOR, BaseColor);

	return NewMaterial;
}

ECS::FEntity FRender::CreateRefractiveMaterial(const FVector3& BaseColor, const std::string& MaterialName)
{
	auto NewMaterial = MATERIAL_SYSTEM()->CreateEmptyMaterial(MaterialName);

	MATERIAL_SYSTEM()->SetF(NewMaterial, MATERIAL_TRANSMISSION_WEIGHT, 1.f);
	MATERIAL_SYSTEM()->Set3F(NewMaterial, MATERIAL_TRANSMISSION_COLOR, BaseColor);
	MATERIAL_SYSTEM()->SetF(NewMaterial, MATERIAL_SPECULAR_IOR, 1.45f);

	return NewMaterial;
}


ECS::FEntity FRender::ShapeSetMaterial(ECS::FEntity Shape, ECS::FEntity Material)
{
    RENDERABLE_SYSTEM()->SetMaterial(Shape, Material);

    return Shape;
}

const std::unordered_map<ECS::FEntity, std::string>& FRender::GetMaterials()
{
	return MATERIAL_SYSTEM()->MaterialToName;
}

const ECS::COMPONENTS::FOpenPBRSurfaceComponent& FRender::GetMaterial(ECS::FEntity Material)
{
	return MATERIAL_SYSTEM()->GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(Material);
}

// ---- Base ----
void FRender::MaterialSetBaseWeight(ECS::FEntity MaterialEntity, float Weight)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_BASE_WEIGHT, Weight);
}

void FRender::MaterialSetBaseWeight(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_BASE_WEIGHT, TextureEntity);
}

void FRender::MaterialSetBaseColor(ECS::FEntity MaterialEntity, const FVector3& Color)
{
	MATERIAL_SYSTEM()->Set3F(MaterialEntity, MATERIAL_BASE_COLOR, Color);
}

void FRender::MaterialSetBaseColor(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_BASE_COLOR, TextureEntity);
}

void FRender::MaterialSetBaseMetalness(ECS::FEntity MaterialEntity, float Metalness)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_BASE_METALNESS, Metalness);
}

void FRender::MaterialSetBaseMetalness(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_BASE_METALNESS, TextureEntity);
}

void FRender::MaterialSetBaseDiffuseRoughness(ECS::FEntity MaterialEntity, float DiffuseRoughness)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_BASE_DIFFUSE_ROUGHNESS, DiffuseRoughness);
}

void FRender::MaterialSetBaseDiffuseRoughness(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_BASE_DIFFUSE_ROUGHNESS, TextureEntity);
}

// ---- Specular ----
void FRender::MaterialSetSpecularWeight(ECS::FEntity MaterialEntity, float Weight)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_SPECULAR_WEIGHT, Weight);
}

void FRender::MaterialSetSpecularWeight(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_SPECULAR_WEIGHT, TextureEntity);
}

void FRender::MaterialSetSpecularColor(ECS::FEntity MaterialEntity, const FVector3& Color)
{
	MATERIAL_SYSTEM()->Set3F(MaterialEntity, MATERIAL_SPECULAR_COLOR, Color);
}

void FRender::MaterialSetSpecularColor(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_SPECULAR_COLOR, TextureEntity);
}

void FRender::MaterialSetSpecularRoughness(ECS::FEntity MaterialEntity, float Roughness)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_SPECULAR_ROUGHNESS, Roughness);
}

void FRender::MaterialSetSpecularRoughness(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_SPECULAR_ROUGHNESS, TextureEntity);
}

void FRender::MaterialSetSpecularRoughnessAnisotropy(ECS::FEntity MaterialEntity, float Anisotropy)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_SPECULAR_ROUGHNESS_ANISOTROPY, Anisotropy);
}

void FRender::MaterialSetSpecularRoughnessAnisotropy(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_SPECULAR_ROUGHNESS_ANISOTROPY, TextureEntity);
}

void FRender::MaterialSetSpecularIor(ECS::FEntity MaterialEntity, float Ior)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_SPECULAR_IOR, Ior);
}

void FRender::MaterialSetSpecularIor(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_SPECULAR_IOR, TextureEntity);
}

// ---- Transmission ----
void FRender::MaterialSetTransmissionWeight(ECS::FEntity MaterialEntity, float Weight)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_TRANSMISSION_WEIGHT, Weight);
}

void FRender::MaterialSetTransmissionWeight(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_TRANSMISSION_WEIGHT, TextureEntity);
}

void FRender::MaterialSetTransmissionColor(ECS::FEntity MaterialEntity, const FVector3& Color)
{
	MATERIAL_SYSTEM()->Set3F(MaterialEntity, MATERIAL_TRANSMISSION_COLOR, Color);
}

void FRender::MaterialSetTransmissionColor(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_TRANSMISSION_COLOR, TextureEntity);
}

void FRender::MaterialSetTransmissionDepth(ECS::FEntity MaterialEntity, float Depth)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_TRANSMISSION_DEPTH, Depth);
}

void FRender::MaterialSetTransmissionDepth(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_TRANSMISSION_DEPTH, TextureEntity);
}

void FRender::MaterialSetTransmissionScatter(ECS::FEntity MaterialEntity, const FVector3& Scatter)
{
	MATERIAL_SYSTEM()->Set3F(MaterialEntity, MATERIAL_TRANSMISSION_SCATTER, Scatter);
}

void FRender::MaterialSetTransmissionScatter(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_TRANSMISSION_SCATTER, TextureEntity);
}

void FRender::MaterialSetTransmissionScatterAnisotropy(ECS::FEntity MaterialEntity, float Anisotropy)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_TRANSMISSION_SCATTER_ANISOTROPY, Anisotropy);
}

void FRender::MaterialSetTransmissionScatterAnisotropy(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_TRANSMISSION_SCATTER_ANISOTROPY, TextureEntity);
}

void FRender::MaterialSetTransmissionDispersionScale(ECS::FEntity MaterialEntity, float DispersionScale)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_TRANSMISSION_DISPERSION_SCALE, DispersionScale);
}

void FRender::MaterialSetTransmissionDispersionScale(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_TRANSMISSION_DISPERSION_SCALE, TextureEntity);
}

void FRender::MaterialSetTransmissionDispersionAbbeNumber(ECS::FEntity MaterialEntity, float AbbeNumber)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_TRANSMISSION_DISPERSION_ABBE_NUMBER, AbbeNumber);
}

void FRender::MaterialSetTransmissionDispersionAbbeNumber(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_TRANSMISSION_DISPERSION_ABBE_NUMBER, TextureEntity);
}

// ---- Subsurface ----
void FRender::MaterialSetSubsurfaceWeight(ECS::FEntity MaterialEntity, float Weight)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_SUBSURFACE_WEIGHT, Weight);
}

void FRender::MaterialSetSubsurfaceWeight(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_SUBSURFACE_WEIGHT, TextureEntity);
}

void FRender::MaterialSetSubsurfaceColor(ECS::FEntity MaterialEntity, const FVector3& Color)
{
	MATERIAL_SYSTEM()->Set3F(MaterialEntity, MATERIAL_SUBSURFACE_COLOR, Color);
}

void FRender::MaterialSetSubsurfaceColor(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_SUBSURFACE_COLOR, TextureEntity);
}

void FRender::MaterialSetSubsurfaceRadius(ECS::FEntity MaterialEntity, float Radius)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_SUBSURFACE_RADIUS, Radius);
}

void FRender::MaterialSetSubsurfaceRadius(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_SUBSURFACE_RADIUS, TextureEntity);
}

void FRender::MaterialSetSubsurfaceRadiusScale(ECS::FEntity MaterialEntity, const FVector3& RadiusScale)
{
	MATERIAL_SYSTEM()->Set3F(MaterialEntity, MATERIAL_SUBSURFACE_RADIUS_SCALE, RadiusScale);
}

void FRender::MaterialSetSubsurfaceRadiusScale(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_SUBSURFACE_RADIUS_SCALE, TextureEntity);
}

void FRender::MaterialSetSubsurfaceScatterAnisotropy(ECS::FEntity MaterialEntity, float Anisotropy)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_SUBSURFACE_SCATTER_ANISOTROPY, Anisotropy);
}

void FRender::MaterialSetSubsurfaceScatterAnisotropy(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_SUBSURFACE_SCATTER_ANISOTROPY, TextureEntity);
}

// ---- Coat ----
void FRender::MaterialSetCoatWeight(ECS::FEntity MaterialEntity, float Weight)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_COAT_WEIGHT, Weight);
}

void FRender::MaterialSetCoatWeight(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_COAT_WEIGHT, TextureEntity);
}

void FRender::MaterialSetCoatColor(ECS::FEntity MaterialEntity, const FVector3& Color)
{
	MATERIAL_SYSTEM()->Set3F(MaterialEntity, MATERIAL_COAT_COLOR, Color);
}

void FRender::MaterialSetCoatColor(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_COAT_COLOR, TextureEntity);
}

void FRender::MaterialSetCoatRoughness(ECS::FEntity MaterialEntity, float Roughness)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_COAT_ROUGHNESS, Roughness);
}

void FRender::MaterialSetCoatRoughness(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_COAT_ROUGHNESS, TextureEntity);
}

void FRender::MaterialSetCoatRoughnessAnisotropy(ECS::FEntity MaterialEntity, float Anisotropy)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_COAT_ROUGHNESS_ANISOTROPY, Anisotropy);
}

void FRender::MaterialSetCoatRoughnessAnisotropy(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_COAT_ROUGHNESS_ANISOTROPY, TextureEntity);
}

void FRender::MaterialSetCoatIor(ECS::FEntity MaterialEntity, float Ior)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_COAT_IOR, Ior);
}

void FRender::MaterialSetCoatIor(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_COAT_IOR, TextureEntity);
}

void FRender::MaterialSetCoatDarkening(ECS::FEntity MaterialEntity, float Darkening)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_COAT_DARKENING, Darkening);
}

void FRender::MaterialSetCoatDarkening(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_COAT_DARKENING, TextureEntity);
}

// ---- Fuzz ----
void FRender::MaterialSetFuzzWeight(ECS::FEntity MaterialEntity, float Weight)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_FUZZ_WEIGHT, Weight);
}

void FRender::MaterialSetFuzzWeight(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_FUZZ_WEIGHT, TextureEntity);
}

void FRender::MaterialSetFuzzColor(ECS::FEntity MaterialEntity, const FVector3& Color)
{
	MATERIAL_SYSTEM()->Set3F(MaterialEntity, MATERIAL_FUZZ_COLOR, Color);
}

void FRender::MaterialSetFuzzColor(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_FUZZ_COLOR, TextureEntity);
}

void FRender::MaterialSetFuzzRoughness(ECS::FEntity MaterialEntity, float Roughness)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_FUZZ_ROUGHNESS, Roughness);
}

void FRender::MaterialSetFuzzRoughness(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_FUZZ_ROUGHNESS, TextureEntity);
}

// ---- Emission ----
void FRender::MaterialSetEmissionLuminance(ECS::FEntity MaterialEntity, float Luminance)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_EMISSION_LUMINANCE, Luminance);
}

void FRender::MaterialSetEmissionLuminance(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_EMISSION_LUMINANCE, TextureEntity);
}

void FRender::MaterialSetEmissionColor(ECS::FEntity MaterialEntity, const FVector3& Color)
{
	MATERIAL_SYSTEM()->Set3F(MaterialEntity, MATERIAL_EMISSION_COLOR, Color);
}

void FRender::MaterialSetEmissionColor(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_EMISSION_COLOR, TextureEntity);
}

// ---- Thin film ----
void FRender::MaterialSetThinFilmWeight(ECS::FEntity MaterialEntity, float Weight)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_THIN_FILM_WEIGHT, Weight);
}

void FRender::MaterialSetThinFilmWeight(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_THIN_FILM_WEIGHT, TextureEntity);
}

void FRender::MaterialSetThinFilmThickness(ECS::FEntity MaterialEntity, float Thickness)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_THIN_FILM_THICKNESS, Thickness);
}

void FRender::MaterialSetThinFilmThickness(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_THIN_FILM_THICKNESS, TextureEntity);
}

void FRender::MaterialSetThinFilmIor(ECS::FEntity MaterialEntity, float Ior)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_THIN_FILM_IOR, Ior);
}

void FRender::MaterialSetThinFilmIor(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_THIN_FILM_IOR, TextureEntity);
}

// ---- Geometry ----
void FRender::MaterialSetGeometryOpacity(ECS::FEntity MaterialEntity, float Opacity)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_GEOMETRY_OPACITY, Opacity);
}

void FRender::MaterialSetGeometryOpacity(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_GEOMETRY_OPACITY, TextureEntity);
}

void FRender::MaterialSetGeometryThinWalled(ECS::FEntity MaterialEntity, bool bThinWalled)
{
	MATERIAL_SYSTEM()->SetF(MaterialEntity, MATERIAL_GEOMETRY_THIN_WALLED, bThinWalled ? 1.f : 0.f);
}

void FRender::MaterialSetGeometryThinWalled(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_GEOMETRY_THIN_WALLED, TextureEntity);
}

void FRender::MaterialSetGeometryNormal(ECS::FEntity MaterialEntity, const FVector3& Normal)
{
	MATERIAL_SYSTEM()->Set3F(MaterialEntity, MATERIAL_GEOMETRY_NORMAL, Normal);
}

void FRender::MaterialSetGeometryNormal(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_GEOMETRY_NORMAL, TextureEntity);
}

void FRender::MaterialSetGeometryTangent(ECS::FEntity MaterialEntity, const FVector3& Tangent)
{
	MATERIAL_SYSTEM()->Set3F(MaterialEntity, MATERIAL_GEOMETRY_TANGENT, Tangent);
}

void FRender::MaterialSetGeometryTangent(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_GEOMETRY_TANGENT, TextureEntity);
}

void FRender::MaterialSetGeometryCoatNormal(ECS::FEntity MaterialEntity, const FVector3& Normal)
{
	MATERIAL_SYSTEM()->Set3F(MaterialEntity, MATERIAL_GEOMETRY_COAT_NORMAL, Normal);
}

void FRender::MaterialSetGeometryCoatNormal(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_GEOMETRY_COAT_NORMAL, TextureEntity);
}

void FRender::MaterialSetGeometryCoatTangent(ECS::FEntity MaterialEntity, const FVector3& Tangent)
{
	MATERIAL_SYSTEM()->Set3F(MaterialEntity, MATERIAL_GEOMETRY_COAT_TANGENT, Tangent);
}

void FRender::MaterialSetGeometryCoatTangent(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity)
{
	MATERIAL_SYSTEM()->SetTexture(MaterialEntity, MATERIAL_GEOMETRY_COAT_TANGENT, TextureEntity);
}

int FRender::SetIBL(const std::string& Path)
{
	auto IBLImage = VK_CONTEXT()->CreateEXRImageFromFile(Path, "V::IBL_Image");

    IBLImage->Transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	TEXTURE_MANAGER()->RegisterIBL(IBLImage);

	MissTask->SetDirty(OUTDATED_DESCRIPTOR_SET | OUTDATED_COMMAND_BUFFER);
	MasterShader->SetDirty(OUTDATED_DESCRIPTOR_SET | OUTDATED_COMMAND_BUFFER);
	bAnyUpdate = true;

    return 0;
}

void FRender::GetAllTimings(std::vector<std::string>& Names, std::vector<std::vector<float>>& Timings, float& FrameTime, uint32_t FrameIndex)
{
	Names.clear();
	Timings.clear();

	FrameTime = std::chrono::duration<float, std::chrono::milliseconds ::period>(Time - PreviousTime).count();

	auto TimingFillingLambda = [&NamesIn = Names, &TimingsIn = Timings](const std::shared_ptr<FExecutableTask>& Task, uint32_t FrameIndex)
	{
		NamesIn.push_back(Task->Name);
		TimingsIn.push_back(Task->RequestTiming(FrameIndex));
	};

	TimingFillingLambda(UpdateTLASTask, FrameIndex);
	TimingFillingLambda(ResetRenderIterations, FrameIndex);
	TimingFillingLambda(ClearImageTask, FrameIndex);
	TimingFillingLambda(ClearCumulativeMaterialColorBuffer, FrameIndex);
	TimingFillingLambda(ResetActiveRayCountTask, FrameIndex);
	TimingFillingLambda(ClearBuffersEachBounceTask, FrameIndex);
	TimingFillingLambda(RayTraceTask, FrameIndex);
	TimingFillingLambda(ClearTotalMaterialsCountTask, FrameIndex);
	TimingFillingLambda(CountMaterialsPerChunkTask, FrameIndex);
	TimingFillingLambda(ComputePrefixSumsUpSweepTask, FrameIndex);
	TimingFillingLambda(ComputePrefixSumsZeroOutTask, FrameIndex);
	TimingFillingLambda(ComputePrefixSumsDownSweepTask, FrameIndex);
	TimingFillingLambda(ComputeOffsetsPerMaterialTask, FrameIndex);
	TimingFillingLambda(SortMaterialsTask, FrameIndex);
	TimingFillingLambda(MasterShader, FrameIndex);
	TimingFillingLambda(MissTask, FrameIndex);
	TimingFillingLambda(AccumulateTask, FrameIndex);
	TimingFillingLambda(AdvanceRenderCountTask, FrameIndex);
	TimingFillingLambda(PassthroughTask, FrameIndex);

	for (auto& Task : ExternalTasks)
	{
		TimingFillingLambda(Task, FrameIndex);
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

ECS::FEntity FRender::CreateUVSphere(uint32_t LongitudeCount, uint32_t LatitudeCount, float Radius)
{
	auto NewModel = CreateEmptyModel();

	MESH_SYSTEM()->CreateUVSphere(NewModel, LongitudeCount, LatitudeCount, Radius);
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

ECS::FEntity FRender::CreateInstance(ECS::FEntity BaseModel, const FVector3& Position, const FVector3& Direction, const FVector3& Up, const FVector3& Scale)
{
    auto MeshInstance = ACCELERATION_STRUCTURE_SYSTEM()->CreateInstance(BaseModel, Position, Direction, Up, Scale);
    RENDERABLE_SYSTEM()->SetRenderableDeviceAddress(MeshInstance, MESH_SYSTEM()->GetVertexBufferAddress(MeshInstance), MESH_SYSTEM()->GetIndexBufferAddress(MeshInstance));
    RENDERABLE_SYSTEM()->SyncTransform(MeshInstance);
    auto& MeshComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FMeshComponent>(BaseModel);

    if (MeshComponent.Indexed)
    {
        RENDERABLE_SYSTEM()->SetIndexed(MeshInstance);
    }

    return MeshInstance;
}

void FRender::SetInstancePosition(ECS::FEntity Instance, const FVector3& Position,  const std::optional<FVector3>& Direction, const std::optional<FVector3>& Up)
{
	auto& TransformComponent = TRANSFORM_SYSTEM()->GetComponent<ECS::COMPONENTS::FTransformComponent>(Instance);

	if (Direction)
	{
		TransformComponent.Direction = Direction.value();
	}

	if (Up)
	{
		TransformComponent.Up = Up.value();
	}

	TRANSFORM_SYSTEM()->SetTransform(Instance, Position, TransformComponent.Direction, TransformComponent.Up);
	ACCELERATION_STRUCTURE_SYSTEM()->UpdateInstancePosition(Instance);
}

FVector3 FRender::GetInstancePosition(ECS::FEntity Instance)
{
	return TRANSFORM_SYSTEM()->GetComponent<ECS::COMPONENTS::FTransformComponent>(Instance).Position;
}

ECS::FEntity FRender::CreateDirectionalLight(const FVector3& Direction, const FVector3& Color, float Intensity)
{
	return DIRECTIONAL_LIGHT_SYSTEM()->CreateDirectionalLight(Direction, Color, Intensity);
}

ECS::FEntity FRender::CreatePointLight(const FVector3& Position, const FVector3& Color, float Intensity)
{
	return POINT_LIGHT_SYSTEM()->CreatePointLight(Position, Color, Intensity);
}

ECS::FEntity FRender::CreateSpotLight(const FVector3& Position, const FVector3& Direction, const FVector3& Color, float Intensity, float OuterAngle, float InnerAngle)
{
	return SPOT_LIGHT_SYSTEM()->CreateSpotLight(Position, Direction, Color, Intensity, OuterAngle, InnerAngle);
}

ECS::FEntity FRender::CreateAreaLight(ECS::FEntity Renderable)
{
    return AREA_LIGHT_SYSTEM()->CreateAreaLightInstance(Renderable);
}

void FRender::SetLightPosition(ECS::FEntity Light, const FVector3& Position)
{
	POINT_LIGHT_SYSTEM()->SetLightPosition(Light, Position);
}

FVector3 FRender::GetLightPosition(ECS::FEntity Light)
{
	return COORDINATOR().GetComponent<ECS::COMPONENTS::FPointLightComponent>(Light).Position;
}

ECS::FEntity FRender::CreateEmptyModel()
{
    ECS::FEntity EmptyModel = COORDINATOR().CreateEntity();
    COORDINATOR().AddComponent<ECS::COMPONENTS::FMeshComponent>(EmptyModel, {});
    COORDINATOR().AddComponent<ECS::COMPONENTS::FDeviceMeshComponent>(EmptyModel, {});

    return EmptyModel;
}

void FRender::AllocateDependentResources()
{
	/// Create internal buffers
	std::vector<FBufferDescription> BufferDescriptions = {
		{THROUGHPUT_BUFFER, 					sizeof(FVector4) * Width * Height, 0},
		{INITIAL_RAYS_BUFFER, 				sizeof(FRayData) * Width * Height, 0},
		{HITS_BUFFER, 						sizeof(FHit) * Width * Height, 	0},
		{PIXEL_INDEX_BUFFER, 					sizeof(uint32_t) * Width * Height, 0},
		{MATERIAL_INDEX_AOV_BUFFER,			sizeof(uint32_t) * Width * Height, 0},
		{COUNTED_MATERIALS_PER_CHUNK_BUFFER,	sizeof(uint32_t) * TOTAL_MATERIALS * CalculateMaxGroupCount(Width * Height, BASIC_CHUNK_SIZE),	VK_BUFFER_USAGE_TRANSFER_DST_BIT},
		{CUMULATIVE_MATERIAL_COLOR_BUFFER, 	sizeof(FVector4) * Width * Height, VK_BUFFER_USAGE_TRANSFER_DST_BIT},
	};

	CreateAndRegisterBufferShortcut(BufferDescriptions);

	/// Create internal images
	std::vector<FImageDescription> ImageDescriptions = {
		{"ColorImage", 		Width, Height, VK_FORMAT_R32G32B32A32_SFLOAT},
		{"AOVImage", 		Width, Height, VK_FORMAT_R32G32B32A32_SFLOAT},
	};

	CreateRegisterAndTransitionImageShortcut(ImageDescriptions);

	auto AccumulatorImage = TEXTURE_MANAGER()->CreateClearableStorageImage(Width, Height,VK_FORMAT_R32G32B32A32_SFLOAT, "AccumulatorImage");
	TEXTURE_MANAGER()->RegisterFramebuffer(AccumulatorImage, "AccumulatorImage");
	AccumulatorImage->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	auto EstimatedImage = TEXTURE_MANAGER()->CreateSampledStorageImage(Width, Height, VK_FORMAT_R32G32B32A32_SFLOAT, "EstimatedImage");
	TEXTURE_MANAGER()->RegisterFramebuffer(EstimatedImage, "EstimatedImage");
	EstimatedImage->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
}

void FRender::AllocateIndependentResources()
{
	/// Allocate buffers that doesn't require recreation
	std::vector<FBufferDescription> BufferDescriptions = {
		{RENDER_ITERATION_BUFFER,				sizeof(uint32_t),							VK_BUFFER_USAGE_TRANSFER_DST_BIT},
		{TOTAL_COUNTED_MATERIALS_BUFFER,		sizeof(uint32_t) * TOTAL_MATERIALS * 3,	VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT},
		{ACTIVE_RAY_COUNT_BUFFER, 				sizeof(uint32_t) * 3,						VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT},
		{MATERIALS_OFFSETS_PER_MATERIAL_BUFFER,	sizeof(uint32_t) * TOTAL_MATERIALS,	0},
		{POINT_LIGHTS_IMPORTANCE_BUFFER, 		sizeof(FAliasTableEntry) * POINT_LIGHT_SYSTEM()->MAX_POINT_LIGHTS, 0},
		{DIRECTIONAL_LIGHTS_IMPORTANCE_BUFFER,	sizeof(FAliasTableEntry) * DIRECTIONAL_LIGHT_SYSTEM()->MAX_DIRECTIONAL_LIGHTS, 0},
		{SPOT_LIGHTS_IMPORTANCE_BUFFER,			sizeof(FAliasTableEntry) * SPOT_LIGHT_SYSTEM()->MAX_SPOT_LIGHTS, 0},
        {AREA_LIGHTS_IMPORTANCE_BUFFER, 		sizeof(uint64_t) * AREA_LIGHT_SYSTEM()->MAX_AREA_LIGHTS, 0},
	};

	CreateAndRegisterBufferShortcut(BufferDescriptions);

	auto UtilityInfo = RESOURCE_ALLOCATOR()->CreateBuffer(sizeof(FUtilityData),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, UTILITY_INFO_BUFFER);
	RESOURCE_ALLOCATOR()->RegisterBuffer(UtilityInfo, UTILITY_INFO_BUFFER);

	FUtilityData UtilityData{};
	UtilityData.AccumulateFrames = true;
	RESOURCE_ALLOCATOR()->LoadDataToBuffer(UTILITY_INFO_BUFFER, sizeof(FUtilityData), 0, &UtilityData);
}

void FRender::FreeDependentResources()
{
	/// Free buffers
	RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer(THROUGHPUT_BUFFER);
	RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer(INITIAL_RAYS_BUFFER);
	RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer(HITS_BUFFER);
	RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer(PIXEL_INDEX_BUFFER);
	RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer(MATERIAL_INDEX_AOV_BUFFER);
	RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer(CUMULATIVE_MATERIAL_COLOR_BUFFER);
	RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer(COUNTED_MATERIALS_PER_CHUNK_BUFFER);

	/// Free images
	TEXTURE_MANAGER()->UnregisterAndFreeFramebuffer("ColorImage");
	TEXTURE_MANAGER()->UnregisterAndFreeFramebuffer("AOVImage");
}

void FRender::FreeIndependentResources()
{
	/// Free buffers
	RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer(RENDER_ITERATION_BUFFER);
	RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer(TOTAL_COUNTED_MATERIALS_BUFFER);
	RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer(MATERIALS_OFFSETS_PER_MATERIAL_BUFFER);
	RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer(UTILITY_INFO_BUFFER);
	RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer(ACTIVE_RAY_COUNT_BUFFER);
	RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer(POINT_LIGHTS_IMPORTANCE_BUFFER);
	RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer(DIRECTIONAL_LIGHTS_IMPORTANCE_BUFFER);
	RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer(SPOT_LIGHTS_IMPORTANCE_BUFFER);
    RESOURCE_ALLOCATOR()->UnregisterAndDestroyBuffer(AREA_LIGHTS_IMPORTANCE_BUFFER);
}

void FRender::CreateAndRegisterBufferShortcut(const std::vector<FBufferDescription>& BufferDescriptions)
{
	for (auto & BufferDescription : BufferDescriptions)
	{
		FBuffer Buffer = RESOURCE_ALLOCATOR()->CreateBuffer(BufferDescription.Size,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | BufferDescription.Flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, BufferDescription.Name);
		RESOURCE_ALLOCATOR()->RegisterBuffer(Buffer, BufferDescription.Name);
	}
};

void FRender::CreateRegisterAndTransitionImageShortcut(const std::vector<FImageDescription>& ImageDescriptions)
{
	for (auto & ImageDescription : ImageDescriptions)
	{
		auto Image = TEXTURE_MANAGER()->CreateStorageImage(ImageDescription.Width, ImageDescription.Height, ImageDescription.Format, ImageDescription.Name);
		TEXTURE_MANAGER()->RegisterFramebuffer(Image, ImageDescription.Name);
		Image->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	}
}
