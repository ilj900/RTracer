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

void FRender::RegisterExternalOutputs(std::vector<ImagePtr> OutputImagesIn, const std::vector<VkSemaphore>& ExternalImageIsReadySemaphoreIn)
{
	/// Check whether sizes are the same
	if (OutputImagesIn.size() != ExternalImageIsReadySemaphoreIn.size())
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

	/// Copy semaphores
	ExternalImageIsReadySemaphore = ExternalImageIsReadySemaphoreIn;

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
	if (ExternalImageIsReadySemaphore.empty())
	{
		for (int i = 0; i < MaxFramesInFlight; ++i)
		{
			auto Framebuffer = CreateColorAttachment(Width, Height, "Output Image" + std::to_string(i));
			SetOutput(OutputType(i), Framebuffer);
		}

		ImageAvailableSemaphores.resize(MaxFramesInFlight);
	}

    for (int i = 0; i < MaxFramesInFlight; ++i)
    {
        ImagesInFlight.push_back(VK_CONTEXT()->CreateSignalledFence());
    }

    GenerateRaysTask = std::make_shared<FGenerateInitialRays>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    RayTraceTask = std::make_shared<FRaytraceTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ClearMaterialsCountPerChunkTask = std::make_shared<FClearMaterialsCountPerChunkTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ClearTotalMaterialsCountTask = std::make_shared<FClearTotalMaterialsCountTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    CountMaterialsPerChunkTask = std::make_shared<FCountMaterialsPerChunkTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ComputePrefixSumsUpSweepTask = std::make_shared<FComputePrefixSumsUpSweepTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ComputePrefixSumsZeroOutTask = std::make_shared<FComputePrefixSumsZeroOutTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ComputePrefixSumsDownSweepTask = std::make_shared<FComputePrefixSumsDownSweepTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ComputeOffsetsPerMaterialTask = std::make_shared<FComputeOffsetsPerMaterialTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    SortMaterialsTask = std::make_shared<FSortMaterialsTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ShadeTask = std::make_shared<FShadeTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    MissTask = std::make_shared<FMissTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    SetIBL("../../../resources/brown_photostudio_02_4k.exr");
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

	for (int i = 0; i < ImagesInFlight.size(); ++i)
	{
		vkDestroyFence(VK_CONTEXT()->LogicalDevice, ImagesInFlight[i], nullptr);
		ImagesInFlight[i] = VK_NULL_HANDLE;
	}

    ImageAvailableSemaphores.clear();
    ImagesInFlight.clear();

    return 0;
}

int FRender::SetSize(int WidthIn, int HeightIn)
{
    Width = WidthIn;
    Height = HeightIn;
	bShouldResize = true;
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

int FRender::Render()
{
	return Render(0, nullptr);
}

int FRender::Render(uint32_t OutputImageIndex, VkSemaphore* RenderFinishedSemaphore)
{
    TIMING_MANAGER()->NewTime();

    uint32_t CurrentFrame = RenderFrameIndex % MaxFramesInFlight;

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

    /// Previous rendering iteration of the frame might still be in use, so we wait for it
    vkWaitForFences(VK_CONTEXT()->LogicalDevice, 1, &ImagesInFlight[CurrentFrame], VK_TRUE, UINT64_MAX);

    bool NeedUpdate = true;
	VkSemaphore SemaphoreToWait = VK_NULL_HANDLE;

	/// If we have no external semaphores
	if (ExternalImageIsReadySemaphore.empty())
	{
		/// We need to wait for the internal one, unless it's the first frame
		SemaphoreToWait = (RenderFrameIndex > 0) ? ImageAvailableSemaphores[(RenderFrameIndex - 1) % MaxFramesInFlight] : VK_NULL_HANDLE;
	}
	else
	{
		SemaphoreToWait = ExternalImageIsReadySemaphore[OutputImageIndex];
	}

    auto GenerateRaysSemaphore = GenerateRaysTask->Submit(VK_CONTEXT()->GetComputeQueue(), SemaphoreToWait, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, ImagesInFlight[CurrentFrame], VK_NULL_HANDLE, CurrentFrame);

    auto RayTraceSignalSemaphore = RayTraceTask->Submit(VK_CONTEXT()->GetGraphicsQueue(), GenerateRaysSemaphore, GenerateRaysTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto ClearMaterialsCountPerChunkSemaphore = ClearMaterialsCountPerChunkTask->Submit(VK_CONTEXT()->GetComputeQueue(), RayTraceSignalSemaphore, RayTraceTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto ClearTotalMaterialsCountSemaphore = ClearTotalMaterialsCountTask->Submit(VK_CONTEXT()->GetComputeQueue(), ClearMaterialsCountPerChunkSemaphore, ClearMaterialsCountPerChunkTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto CountMaterialsPerChunkSemaphore = CountMaterialsPerChunkTask->Submit(VK_CONTEXT()->GetComputeQueue(), ClearTotalMaterialsCountSemaphore, ClearTotalMaterialsCountTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto ComputePrefixSumsUpSweepSemaphore = ComputePrefixSumsUpSweepTask->Submit(VK_CONTEXT()->GetComputeQueue(), CountMaterialsPerChunkSemaphore, CountMaterialsPerChunkTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto ComputePrefixSumsZeroOutSemaphore = ComputePrefixSumsZeroOutTask->Submit(VK_CONTEXT()->GetComputeQueue(), ComputePrefixSumsUpSweepSemaphore, ComputePrefixSumsUpSweepTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto ComputePrefixSumsDownSweepSemaphore = ComputePrefixSumsDownSweepTask->Submit(VK_CONTEXT()->GetComputeQueue(), ComputePrefixSumsZeroOutSemaphore, ComputePrefixSumsZeroOutTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto ComputeOffsetsPerMaterialSemaphore = ComputeOffsetsPerMaterialTask->Submit(VK_CONTEXT()->GetComputeQueue(), ComputePrefixSumsDownSweepSemaphore, ComputePrefixSumsDownSweepTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto SortMaterialsSemaphore = SortMaterialsTask->Submit(VK_CONTEXT()->GetComputeQueue(), ComputeOffsetsPerMaterialSemaphore, ComputeOffsetsPerMaterialTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto ShadeSignalSemaphore = ShadeTask->Submit(VK_CONTEXT()->GetComputeQueue(), SortMaterialsSemaphore, SortMaterialsTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto MissSignalSemaphore = MissTask->Submit(VK_CONTEXT()->GetComputeQueue(), ShadeSignalSemaphore, ShadeTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    VkSemaphore AccumulateSignalSemaphore = VK_NULL_HANDLE;
    VkPipelineStageFlags PipelineStageFlags = 0;

    if (NeedUpdate)
    {
        auto ClearAccumulatorSemaphore = ClearImageTask->Submit(VK_CONTEXT()->GetComputeQueue(), MissSignalSemaphore, MissTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);
        AccumulateSignalSemaphore = AccumulateTask->Submit(VK_CONTEXT()->GetComputeQueue(), ClearAccumulatorSemaphore, ClearImageTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);
        PipelineStageFlags = ClearImageTask->GetPipelineStageFlags();
    }
    else
    {
        AccumulateSignalSemaphore = AccumulateTask->Submit(VK_CONTEXT()->GetComputeQueue(), MissSignalSemaphore, MissTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);
        PipelineStageFlags = AccumulateTask->GetPipelineStageFlags();
    }

    auto PassthroughSignalSemaphore = PassthroughTask->Submit(VK_CONTEXT()->GetGraphicsQueue(), AccumulateSignalSemaphore, PipelineStageFlags, VK_NULL_HANDLE, ImagesInFlight[CurrentFrame], CurrentFrame);

	if (RenderFinishedSemaphore != nullptr)
	{
		*RenderFinishedSemaphore = PassthroughSignalSemaphore;
	}
	else
	{
		ImageAvailableSemaphores[CurrentFrame] = PassthroughSignalSemaphore;
	}

    RenderFrameIndex++;

    return 0;
}

int FRender::Update()
{
	CAMERA_SYSTEM()->Update();
	TRANSFORM_SYSTEM()->Update();
	RENDERABLE_SYSTEM()->Update();
	LIGHT_SYSTEM()->Update();
	ACCELERATION_STRUCTURE_SYSTEM()->Update();
	ACCELERATION_STRUCTURE_SYSTEM()->UpdateTLAS();

    if (bShouldResize)
    {
        Cleanup();
        Init();
		bShouldResize = false;
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
    MissTask->RegisterInput(1, IBLImage);

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

ECS::FEntity FRender::CreateIcosahedronSphere(int LevelOfComplexity, bool bJagged)
{
    auto NewModel = CreateEmptyModel();

    MESH_SYSTEM()->CreateIcosahedron(NewModel, LevelOfComplexity, bJagged);
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
