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

	UpdateTLASTask = std::make_shared<FUpdateTLASTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    GenerateRaysTask = std::make_shared<FGenerateInitialRays>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    RayTraceTask = std::make_shared<FRaytraceTask>(Width, Height, MaxFramesInFlight * RecursionDepth, VK_CONTEXT()->LogicalDevice);
    ClearMaterialsCountPerChunkTask = std::make_shared<FClearMaterialsCountPerChunkTask>(Width, Height, MaxFramesInFlight * RecursionDepth, VK_CONTEXT()->LogicalDevice);
    ClearTotalMaterialsCountTask = std::make_shared<FClearTotalMaterialsCountTask>(Width, Height, MaxFramesInFlight * RecursionDepth, VK_CONTEXT()->LogicalDevice);
    CountMaterialsPerChunkTask = std::make_shared<FCountMaterialsPerChunkTask>(Width, Height, MaxFramesInFlight * RecursionDepth, VK_CONTEXT()->LogicalDevice);
    ComputePrefixSumsUpSweepTask = std::make_shared<FComputePrefixSumsUpSweepTask>(Width, Height, MaxFramesInFlight * RecursionDepth, VK_CONTEXT()->LogicalDevice);
    ComputePrefixSumsZeroOutTask = std::make_shared<FComputePrefixSumsZeroOutTask>(Width, Height, MaxFramesInFlight * RecursionDepth, VK_CONTEXT()->LogicalDevice);
    ComputePrefixSumsDownSweepTask = std::make_shared<FComputePrefixSumsDownSweepTask>(Width, Height, MaxFramesInFlight * RecursionDepth, VK_CONTEXT()->LogicalDevice);
    ComputeOffsetsPerMaterialTask = std::make_shared<FComputeOffsetsPerMaterialTask>(Width, Height, MaxFramesInFlight * RecursionDepth, VK_CONTEXT()->LogicalDevice);
    SortMaterialsTask = std::make_shared<FSortMaterialsTask>(Width, Height, MaxFramesInFlight * RecursionDepth, VK_CONTEXT()->LogicalDevice);
    ShadeTask = std::make_shared<FShadeTask>(Width, Height, MaxFramesInFlight * RecursionDepth, VK_CONTEXT()->LogicalDevice);
    MissTask = std::make_shared<FMissTask>(Width, Height, MaxFramesInFlight * RecursionDepth, VK_CONTEXT()->LogicalDevice);
    AccumulateTask = std::make_shared<FAccumulateTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ClearImageTask = std::make_shared<FClearImageTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    PassthroughTask = std::make_shared<FPassthroughTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);

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

	UpdateTLASTask = nullptr;
    GenerateRaysTask = nullptr;
    RayTraceTask = nullptr;
    ClearMaterialsCountPerChunkTask = nullptr;
    ClearTotalMaterialsCountTask = nullptr;
    ComputeOffsetsPerMaterialTask = nullptr;
    CountMaterialsPerChunkTask = nullptr;
    SortMaterialsTask = nullptr;
    ComputePrefixSumsUpSweepTask = nullptr;
    ComputePrefixSumsZeroOutTask = nullptr;
    ComputePrefixSumsDownSweepTask = nullptr;
    ShadeTask = nullptr;
    MissTask = nullptr;
    AccumulateTask = nullptr;
    ClearImageTask = nullptr;
    PassthroughTask = nullptr;

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
    TIMING_MANAGER()->NewTime();

    uint32_t CurrentFrame = RenderFrameIndex % MaxFramesInFlight;

	UpdateTLASTask->Reload();
	GenerateRaysTask->Reload();
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
	AccumulateTask->Reload();
	ClearImageTask->Reload();
	PassthroughTask->Reload();

    bool NeedUpdate = true;
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

	SynchronizationPoint = UpdateTLASTask->Submit(PipelineStageFlags, SynchronizationPoint, CurrentFrame);

	SynchronizationPoint = GenerateRaysTask->Submit(PipelineStageFlags, SynchronizationPoint, CurrentFrame);

	for (uint32_t i = 0; i < RecursionDepth; ++i)
	{
		SynchronizationPoint = RayTraceTask->Submit(PipelineStageFlags, SynchronizationPoint, i * MaxFramesInFlight + CurrentFrame);

		SynchronizationPoint = ClearMaterialsCountPerChunkTask->Submit(PipelineStageFlags, SynchronizationPoint, i * MaxFramesInFlight + CurrentFrame);

		SynchronizationPoint = ClearTotalMaterialsCountTask->Submit(PipelineStageFlags, SynchronizationPoint, i * MaxFramesInFlight + CurrentFrame);

		SynchronizationPoint = CountMaterialsPerChunkTask->Submit(PipelineStageFlags, SynchronizationPoint, i * MaxFramesInFlight + CurrentFrame);

		SynchronizationPoint = ComputePrefixSumsUpSweepTask->Submit(PipelineStageFlags, SynchronizationPoint, i * MaxFramesInFlight + CurrentFrame);

		SynchronizationPoint = ComputePrefixSumsZeroOutTask->Submit(PipelineStageFlags, SynchronizationPoint, i * MaxFramesInFlight + CurrentFrame);

		SynchronizationPoint = ComputePrefixSumsDownSweepTask->Submit(PipelineStageFlags, SynchronizationPoint, i * MaxFramesInFlight + CurrentFrame);

		SynchronizationPoint = ComputeOffsetsPerMaterialTask->Submit(PipelineStageFlags, SynchronizationPoint, i * MaxFramesInFlight + CurrentFrame);

		SynchronizationPoint = SortMaterialsTask->Submit(PipelineStageFlags, SynchronizationPoint, i * MaxFramesInFlight + CurrentFrame);

		SynchronizationPoint = ShadeTask->Submit(PipelineStageFlags, SynchronizationPoint, i * MaxFramesInFlight + CurrentFrame);

		SynchronizationPoint = MissTask->Submit(PipelineStageFlags, SynchronizationPoint, i * MaxFramesInFlight + CurrentFrame);
	}

    if (NeedUpdate)
    {
		SynchronizationPoint = ClearImageTask->Submit(PipelineStageFlags, SynchronizationPoint, CurrentFrame);
    }

	SynchronizationPoint = AccumulateTask->Submit(PipelineStageFlags, SynchronizationPoint, CurrentFrame);

	std::vector<VkFence> FencesToSignal;
	/// If no external work to be done, then use the internal fence in passthrough
	if (ExternalTasks.empty())
	{
		SynchronizationPoint.FencesToSignal.push_back(ImagesInFlight[CurrentFrame]);
	}

	SynchronizationPoint = PassthroughTask->Submit(PipelineStageFlags, SynchronizationPoint, CurrentFrame);

	if (!ExternalTasks.empty())
	{
		for (uint32_t i = 0; i < ExternalTasks.size() - 1; ++i)
		{
			SynchronizationPoint = ExternalTasks[i]->Submit(PipelineStageFlags, SynchronizationPoint, CurrentFrame);
		}

		SynchronizationPoint.FencesToSignal.push_back(ImagesInFlight[CurrentFrame]);
		SynchronizationPoint = ExternalTasks.back()->Submit(PipelineStageFlags, SynchronizationPoint, CurrentFrame);
	}

	ImageAvailable[CurrentFrame] = SynchronizationPoint;

    RenderFrameIndex++;

    return SynchronizationPoint;
}

int FRender::Update()
{
	CAMERA_SYSTEM()->Update();
	TRANSFORM_SYSTEM()->Update();
	RENDERABLE_SYSTEM()->Update();
	LIGHT_SYSTEM()->Update();
	ACCELERATION_STRUCTURE_SYSTEM()->Update();

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
