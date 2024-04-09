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
#include "window_manager.h"

#include "utils.h"

#include "logging.h"

FRender* FRender::RenderInstance = nullptr;

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

    INIT_VK_CONTEXT(WINDOW());
	VK_CONTEXT()->InitManagerResources();
    CAMERA_SYSTEM()->Init(MaxFramesInFlight);
    RENDERABLE_SYSTEM()->Init(MaxFramesInFlight);
    MESH_SYSTEM()->Init();
    LIGHT_SYSTEM()->Init(MaxFramesInFlight);
    TRANSFORM_SYSTEM()->Init(MaxFramesInFlight);
    ACCELERATION_STRUCTURE_SYSTEM()->Init(MaxFramesInFlight);

    LoadScene("");
    ACCELERATION_STRUCTURE_SYSTEM()->Update();
    ACCELERATION_STRUCTURE_SYSTEM()->UpdateTLAS();

    Init();
}

void FRender::SetMaxFramesInFlight(uint32_t MaxFramesInFlightIn)
{
    MaxFramesInFlight = MaxFramesInFlightIn;
}

int FRender::Init()
{
    for (int i = 0; i < MaxFramesInFlight; ++i)
    {
        auto Framebuffer = CreateColorAttachment(Width, Height, "Output Image" + std::to_string(i));
        SetOutput(OutputType(i), Framebuffer);
    }

    for (int i = 0; i < MaxFramesInFlight; ++i)
    {
        ImageAvailableSemaphores.push_back(VK_CONTEXT()->CreateSemaphore());
        ImagesInFlight.push_back(VK_CONTEXT()->CreateSignalledFence());
    }

    GenerateRaysTask = std::make_shared<FGenerateInitialRays>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    GenerateRaysTask->Init();
    GenerateRaysTask->UpdateDescriptorSets();
    GenerateRaysTask->RecordCommands();

    RayTraceTask = std::make_shared<FRaytraceTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);

    RayTraceTask->Init();
    RayTraceTask->UpdateDescriptorSets();
    RayTraceTask->RecordCommands();

    ClearMaterialsCountPerChunkTask = std::make_shared<FClearMaterialsCountPerChunkTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);

    ClearMaterialsCountPerChunkTask->Init();
    ClearMaterialsCountPerChunkTask->UpdateDescriptorSets();
    ClearMaterialsCountPerChunkTask->RecordCommands();

    ClearTotalMaterialsCountTask = std::make_shared<FClearTotalMaterialsCountTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);

    ClearTotalMaterialsCountTask->Init();
    ClearTotalMaterialsCountTask->UpdateDescriptorSets();
    ClearTotalMaterialsCountTask->RecordCommands();

    CountMaterialsPerChunkTask = std::make_shared<FCountMaterialsPerChunkTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);

    CountMaterialsPerChunkTask->Init();
    CountMaterialsPerChunkTask->UpdateDescriptorSets();
    CountMaterialsPerChunkTask->RecordCommands();

    ComputePrefixSumsUpSweepTask = std::make_shared<FComputePrefixSumsUpSweepTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);

    ComputePrefixSumsUpSweepTask->Init();
    ComputePrefixSumsUpSweepTask->UpdateDescriptorSets();
    ComputePrefixSumsUpSweepTask->RecordCommands();

    ComputePrefixSumsZeroOutTask = std::make_shared<FComputePrefixSumsZeroOutTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);

    ComputePrefixSumsZeroOutTask->Init();
    ComputePrefixSumsZeroOutTask->UpdateDescriptorSets();
    ComputePrefixSumsZeroOutTask->RecordCommands();

    ComputePrefixSumsDownSweepTask = std::make_shared<FComputePrefixSumsDownSweepTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);

    ComputePrefixSumsDownSweepTask->Init();
    ComputePrefixSumsDownSweepTask->UpdateDescriptorSets();
    ComputePrefixSumsDownSweepTask->RecordCommands();

    ComputeOffsetsPerMaterialTask = std::make_shared<FComputeOffsetsPerMaterialTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);

    ComputeOffsetsPerMaterialTask->Init();
    ComputeOffsetsPerMaterialTask->UpdateDescriptorSets();
    ComputeOffsetsPerMaterialTask->RecordCommands();

    SortMaterialsTask = std::make_shared<FSortMaterialsTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);

    SortMaterialsTask->Init();
    SortMaterialsTask->UpdateDescriptorSets();
    SortMaterialsTask->RecordCommands();

    ShadeTask = std::make_shared<FShadeTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ShadeTask->Init();
    ShadeTask->UpdateDescriptorSets();
    ShadeTask->RecordCommands();

    MissTask = std::make_shared<FMissTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    SetIBL("../../../resources/brown_photostudio_02_4k.exr");
    MissTask->Init();
    MissTask->UpdateDescriptorSets();
    MissTask->RecordCommands();

    AccumulateTask = std::make_shared<FAccumulateTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    AccumulateTask->Init();
    AccumulateTask->UpdateDescriptorSets();
    AccumulateTask->RecordCommands();

    ClearImageTask = std::make_shared<FClearImageTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    ClearImageTask->Init();
    ClearImageTask->UpdateDescriptorSets();
    ClearImageTask->RecordCommands();

    PassthroughTask = std::make_shared<FPassthroughTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    for (int i = 0; i < MaxFramesInFlight; ++i)
    {
        auto& FramebufferComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FFramebufferComponent>(OutputToFramebufferMap[OutputType(i)]);
        PassthroughTask->RegisterOutput(i, TEXTURE_MANAGER()->GetFramebufferImage(FramebufferComponent.FramebufferImageIndex));
    }
    PassthroughTask->Init();
    PassthroughTask->UpdateDescriptorSets();
    PassthroughTask->RecordCommands();

    ImguiTask = std::make_shared<FImguiTask>(Width, Height, MaxFramesInFlight, VK_CONTEXT()->LogicalDevice);
    for (int i = 0; i < MaxFramesInFlight; ++i)
    {
        auto& FramebufferComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FFramebufferComponent>(OutputToFramebufferMap[OutputType(i)]);
        ImguiTask->RegisterOutput(i, TEXTURE_MANAGER()->GetFramebufferImage(FramebufferComponent.FramebufferImageIndex));
    }
    ImguiTask->Init();
    ImguiTask->RecordCommands();

    RenderFrameIndex = 0;

    return 0;
}

int FRender::Cleanup()
{
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
    ImguiTask = nullptr;

    for (int i = 0; i < MaxFramesInFlight; ++i)
    {
        vkDestroySemaphore(VK_CONTEXT()->LogicalDevice, ImageAvailableSemaphores[i], nullptr);
        ImageAvailableSemaphores[i] = VK_NULL_HANDLE;
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
    bShouldRecreateSwapchain = true;
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
    auto FramebufferImageIndex = TEXTURE_MANAGER()->RegisterFramebuffer(FramebufferImage, (DebugName == "") ? ("Unnamed Framebuffer " + std::to_string(Counter++)) : DebugName);

    ECS::FEntity Framebuffer = COORDINATOR().CreateEntity();
    COORDINATOR().AddComponent<ECS::COMPONENTS::FFramebufferComponent>(Framebuffer, {FramebufferImageIndex});

    return Framebuffer;
}

ECS::FEntity FRender::CreateFramebufferFromExternalImage(ImagePtr ImageIn, const std::string& DebugName)
{
    static int Counter = 0;
    auto FramebufferImageIndex = TEXTURE_MANAGER()->RegisterFramebuffer(ImageIn, (DebugName == "") ? ("Unnamed Framebuffer From External Image " + std::to_string(Counter++)) : DebugName);

    ECS::FEntity Framebuffer = COORDINATOR().CreateEntity();
    COORDINATOR().AddComponent<ECS::COMPONENTS::FFramebufferComponent>(Framebuffer, {FramebufferImageIndex});

    return Framebuffer;
}

ECS::FEntity FRender::CreateColorAttachment(int WidthIn, int HeightIn, const std::string& DebugName)
{
	auto FramebufferImage = TEXTURE_MANAGER()->CreateColorAttachment(WidthIn, HeightIn, DebugName);
	static int Counter = 0;
	auto FramebufferImageIndex = TEXTURE_MANAGER()->RegisterFramebuffer(FramebufferImage, (DebugName == "") ? ("Unnamed Framebuffer " + std::to_string(Counter++)) : DebugName);

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

void FRender::GetFramebufferData(ECS::FEntity Framebuffer)
{
    auto& FramebufferComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FFramebufferComponent>(Framebuffer);
    auto Image = TEXTURE_MANAGER()->GetFramebufferImage(FramebufferComponent.FramebufferImageIndex);
    std::vector<char> Data;

    VK_CONTEXT()->FetchImageData(*Image, Data);
}

int FRender::Destroy()
{
    VK_CONTEXT()->WaitIdle();
    Cleanup();

    ACCELERATION_STRUCTURE_SYSTEM()->Terminate();
    MESH_SYSTEM()->Terminate();

    VK_CONTEXT()->CleanUp();

    return 0;
}

int FRender::Render()
{
    if (WINDOW_MANAGER()->ShouldClose())
    {
        return 1;
    }

    TIMING_MANAGER()->NewTime();

    uint32_t CurrentFrame = RenderFrameIndex % MaxFramesInFlight;

    /// Previous rendering iteration of the frame might still be in use, so we wait for it
    vkWaitForFences(VK_CONTEXT()->LogicalDevice, 1, &ImagesInFlight[CurrentFrame], VK_TRUE, UINT64_MAX);

    bool NeedUpdate = true;
    CAMERA_SYSTEM()->Update();
    TRANSFORM_SYSTEM()->Update();
    RENDERABLE_SYSTEM()->Update();
    LIGHT_SYSTEM()->Update();
    ACCELERATION_STRUCTURE_SYSTEM()->Update();

	static bool bFirstCall = true;
    auto GenerateRaysSemaphore = GenerateRaysTask->Submit(VK_CONTEXT()->GetComputeQueue(), bFirstCall ? VK_NULL_HANDLE : ImageAvailableSemaphores[(RenderFrameIndex - 1) % MaxFramesInFlight], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, ImagesInFlight[CurrentFrame], VK_NULL_HANDLE, CurrentFrame);
	bFirstCall = false;

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

    auto PassthroughSignalSemaphore = PassthroughTask->Submit(VK_CONTEXT()->GetGraphicsQueue(), AccumulateSignalSemaphore, PipelineStageFlags, VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    ImageAvailableSemaphores[CurrentFrame] = ImguiTask->Submit(VK_CONTEXT()->GetGraphicsQueue(), PassthroughSignalSemaphore, PassthroughTask->GetPipelineStageFlags(), VK_NULL_HANDLE, ImagesInFlight[CurrentFrame], CurrentFrame);

	if (RenderFrameIndex % 100 == 0)
	{
		VK_CONTEXT()->WaitIdle();
		SaveFramebuffer(OutputToFramebufferMap[OutputType(CurrentFrame)], "Color_Output_" + std::to_string(RenderFrameIndex));
	}
    RenderFrameIndex++;

    glfwPollEvents();

    return 0;
}

int FRender::Update()
{
    auto& LightComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FLightComponent>(Lights.back());
    LightComponent.Position.SelfRotateY(0.025f);
    LIGHT_SYSTEM()->SetLightPosition(Lights.back(), LightComponent.Position.X, LightComponent.Position.Y, LightComponent.Position.Z);

    if (bShouldRecreateSwapchain)
    {
        VK_CONTEXT()->WaitIdle();
        Cleanup();
        Init();
        bShouldRecreateSwapchain = false;
    }

    return 0;
}

int FRender::LoadScene(const std::string& Path)
{
    FTimer Timer("Loading scene time: ");
    auto Plane = CreatePlane();
    auto Pyramid = CreatePyramid();
    auto VikingRoom = CreateModel("../../../models/viking_room/viking_room.obj");
    auto Cube = CreateCube();
    auto Sphere = CreateSphere(3);
    auto Shaderball = CreateModel("../../../models/Shaderball.obj");

    auto WoodMaterial = CreateMaterial({1, 0, 1});
    auto YellowMaterial = CreateMaterial({1, 1, 0});
    auto VikingRoomMaterial = CreateMaterial({0, 1, 1});
    auto RedMaterial = CreateMaterial({1, 0, 0});
    auto GreenMaterial = CreateMaterial({0, 1, 0});
    auto BlueMaterial = CreateMaterial({0, 0, 1});

    auto ModelTexture = CreateTexture("../../../models/viking_room/viking_room.png");
    auto WoodAlbedoTexture = CreateTexture("../../../resources/Wood/Wood_8K_Albedo.jpg");
    auto WoodAOTexture = CreateTexture("../../../resources/Wood/Wood_8K_AO.jpg");
    auto WoodRoughnessTexture = CreateTexture("../../../resources/Wood/Wood_8K_Roughness.jpg");
    auto WoodNormalTexture = CreateTexture("../../../resources/Wood/Wood_8K_Normal.jpg");

    MaterialSetBaseColor(WoodMaterial, WoodAlbedoTexture);
    MaterialSetDiffuseRoughness(WoodMaterial, WoodRoughnessTexture);
    MaterialSetNormal(WoodMaterial, WoodNormalTexture);
    MaterialSetBaseColor(VikingRoomMaterial, ModelTexture);

    auto PlaneInstance = CreateInstance(Plane, {-5.f, 0.f, -2.f});
    auto PyramidInstance = CreateInstance(Pyramid, {-3.f, 0.f, -2.f});
    auto VikingRoomInstance = CreateInstance(VikingRoom, {-1.f, 0.f, -2.f});
    auto CubeInstance = CreateInstance(Cube, {1.f, 0.f, -2.f});
    auto ShaderballInstance = CreateInstance(Shaderball, {5.f, -1.f, -2.f});

    for (int i = -10; i < 10; ++i)
    {
        for (int j = -10; j < 10; ++j)
        {
            auto SphereInstance = CreateInstance(Sphere, {2.f * i, -5.f, 2.f * j});
            ShapeSetMaterial(SphereInstance, GreenMaterial);
        }
    }


    ShapeSetMaterial(PlaneInstance, WoodMaterial);
    ShapeSetMaterial(PyramidInstance, YellowMaterial);
    ShapeSetMaterial(VikingRoomInstance, VikingRoomMaterial);
    ShapeSetMaterial(CubeInstance, RedMaterial);
    ShapeSetMaterial(ShaderballInstance, BlueMaterial);

    CreateLight({5, 5, 5});

    TRANSFORM_SYSTEM()->Update();

    return 0;
}

ECS::FEntity FRender::CreateTexture(const std::string& FilePath)
{
    return TEXTURE_SYSTEM()->CreateTextureFromFile(FilePath);
}

ECS::FEntity FRender::CreateMaterial(const FVector3& BaseColor)
{
    auto NewMaterial = MATERIAL_SYSTEM()->CreateMaterial();
    MATERIAL_SYSTEM()->SetBaseColor(NewMaterial, BaseColor.X, BaseColor.Y, BaseColor.Z);
    Materials.push_back(NewMaterial);

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

ECS::FEntity FRender::CreatePlane()
{
    auto NewModel = CreateEmptyModel();

    MESH_SYSTEM()->CreatePlane(NewModel);
    MESH_SYSTEM()->LoadToGPU(NewModel);
    MESH_SYSTEM()->GenerateBLAS(NewModel);

    Models.push_back(NewModel);

    return NewModel;
}

ECS::FEntity FRender::CreateCube()
{
    auto NewModel = CreateEmptyModel();

    MESH_SYSTEM()->CreateHexahedron(NewModel);
    MESH_SYSTEM()->LoadToGPU(NewModel);
    MESH_SYSTEM()->GenerateBLAS(NewModel);

    Models.push_back(NewModel);

    return NewModel;
}

ECS::FEntity FRender::CreateSphere(int LevelOfComplexity)
{
    auto NewModel = CreateEmptyModel();

    MESH_SYSTEM()->CreateIcosahedron(NewModel, LevelOfComplexity, true);
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

    Models.push_back(NewModel);

    return NewModel;
}

ECS::FEntity FRender::CreatePyramid()
{
    auto NewModel = CreateEmptyModel();

    MESH_SYSTEM()->CreateTetrahedron(NewModel);
    MESH_SYSTEM()->LoadToGPU(NewModel);
    MESH_SYSTEM()->GenerateBLAS(NewModel);

    Models.push_back(NewModel);

    return NewModel;
}

ECS::FEntity FRender::CreateInstance(ECS::FEntity BaseModel,  const FVector3& Position)
{
    auto MeshInstance = ACCELERATION_STRUCTURE_SYSTEM()->CreateInstance(BaseModel, Position);
    RENDERABLE_SYSTEM()->SetRenderableDeviceAddress(MeshInstance, MESH_SYSTEM()->GetVertexBufferAddress(MeshInstance), MESH_SYSTEM()->GetIndexBufferAddress(MeshInstance));
    TRANSFORM_SYSTEM()->SetTransform(MeshInstance, Position, {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f});
    RENDERABLE_SYSTEM()->SyncTransform(MeshInstance);
    auto& MeshComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FMeshComponent>(BaseModel);
    if (MeshComponent.Indexed)
    {
        RENDERABLE_SYSTEM()->SetIndexed(MeshInstance);
    }
    return MeshInstance;
}

int FRender::CreateLight(const FVector3& Position)
{
    auto LightSystem = COORDINATOR().GetSystem<ECS::SYSTEMS::FLightSystem>();

    Lights.push_back(COORDINATOR().CreateEntity());
    COORDINATOR().AddComponent<ECS::COMPONENTS::FLightComponent>(Lights.back(), {});
    LIGHT_SYSTEM()->SetLightPosition(Lights.back(), Position.X, Position.Y, Position.Z);

    return 0;
}

ECS::FEntity FRender::CreateEmptyModel()
{
    ECS::FEntity EmptyModel = COORDINATOR().CreateEntity();
    COORDINATOR().AddComponent<ECS::COMPONENTS::FMeshComponent>(EmptyModel, {});
    COORDINATOR().AddComponent<ECS::COMPONENTS::FDeviceMeshComponent>(EmptyModel, {});

    return EmptyModel;
}

FRender* GetRender(uint32_t WidthIn, uint32_t HeightIn)
{
    if (nullptr == FRender::RenderInstance)
    {
        FRender::RenderInstance = new FRender(WidthIn, HeightIn);
    }

    return FRender::RenderInstance;
}
