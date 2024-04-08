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
    auto& Coordinator = COORDINATOR();
    Coordinator.Init();

    /// Register components
    Coordinator.RegisterComponent<ECS::COMPONENTS::FAccelerationStructureComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FCameraComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FDeviceCameraComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FDeviceMeshComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FDeviceRenderableComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FDeviceTransformComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FLightComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FMaterialComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FMeshComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FMeshInstanceComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FTransformComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FTextureComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FFramebufferComponent>();

    /// Register systems
    auto CameraSystem = Coordinator.RegisterSystem<ECS::SYSTEMS::FCameraSystem>();
    auto TransformSystem = Coordinator.RegisterSystem<ECS::SYSTEMS::FTransformSystem>();
    auto RenderableSystem = Coordinator.RegisterSystem<ECS::SYSTEMS::FRenderableSystem>();
    auto MaterialSystem = Coordinator.RegisterSystem<ECS::SYSTEMS::FMaterialSystem>();
    auto MeshSystem = Coordinator.RegisterSystem<ECS::SYSTEMS::FMeshSystem>();
    auto LightSystem = Coordinator.RegisterSystem<ECS::SYSTEMS::FLightSystem>();
    auto AccelerationSystem = Coordinator.RegisterSystem<ECS::SYSTEMS::FAccelerationStructureSystem>();
    auto TextureSystem = Coordinator.RegisterSystem<ECS::SYSTEMS::FTextureSystem>();

    /// Set camera system signature
    ECS::FSignature CameraSystemSignature;
    CameraSystemSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FCameraComponent>());
    CameraSystemSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FDeviceCameraComponent>());
    Coordinator.SetSystemSignature<ECS::SYSTEMS::FCameraSystem>(CameraSystemSignature);

    /// Register Transform system signature
    ECS::FSignature TransformSystemSignature;
    TransformSystemSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FTransformComponent>());
    TransformSystemSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FDeviceTransformComponent>());
    Coordinator.SetSystemSignature<ECS::SYSTEMS::FTransformSystem>(TransformSystemSignature);

    /// Register Renderable system signature
    ECS::FSignature RenderableSignature;
    RenderableSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FDeviceRenderableComponent>());
    Coordinator.SetSystemSignature<ECS::SYSTEMS::FRenderableSystem>(RenderableSignature);

    /// Register Material system signature
    ECS::FSignature MaterialSignature;
    MaterialSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FMaterialComponent>());
    Coordinator.SetSystemSignature<ECS::SYSTEMS::FMaterialSystem>(MaterialSignature);

    /// Register Mesh systems signature
    ECS::FSignature MeshSignature;
    MeshSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FMeshComponent>());
    MeshSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FDeviceMeshComponent>());
    MeshSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FAccelerationStructureComponent>());
    Coordinator.SetSystemSignature<ECS::SYSTEMS::FMeshSystem>(MeshSignature);

    /// Register Light system signature
    ECS::FSignature LightSignature;
    LightSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FLightComponent>());
    Coordinator.SetSystemSignature<ECS::SYSTEMS::FLightSystem>(LightSignature);

    /// Register Acceleration structure system signature
    ECS::FSignature AccelerationStructureSignature;
    AccelerationStructureSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FMeshInstanceComponent>());
    Coordinator.SetSystemSignature<ECS::SYSTEMS::FAccelerationStructureSystem>(AccelerationStructureSignature);

    ECS::FSignature TextureSignature;
    TextureSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FTextureComponent>());
    Coordinator.SetSystemSignature<ECS::SYSTEMS::FTextureSystem>(TextureSignature);

    auto& Context = GetContext();

    /// Fill in vulkan context creation options
    FVulkanContextOptions VulkanContextOptions;
    VulkanContextOptions.AddInstanceLayer("VK_LAYER_KHRONOS_validation");

#ifndef NDEBUG
    VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo = {};
    DebugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    DebugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    DebugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    DebugCreateInfo.pfnUserCallback = Context.DebugCallback;
    VulkanContextOptions.AddInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, &DebugCreateInfo, sizeof(VkDebugUtilsMessengerCreateInfoEXT));
#endif

    // Resolve and add extensions and layers
    uint32_t Counter = 0;
    auto ExtensionsRequiredByGLFW = glfwGetRequiredInstanceExtensions(&Counter);
    for (uint32_t i = 0; i < Counter; ++i)
    {
        VulkanContextOptions.AddInstanceExtension(ExtensionsRequiredByGLFW[i]);
    }

    VkPhysicalDeviceAccelerationStructureFeaturesKHR PhysicalDeviceAccelerationStructureFeatures{};
    PhysicalDeviceAccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    PhysicalDeviceAccelerationStructureFeatures.accelerationStructure = VK_TRUE;
    VulkanContextOptions.AddDeviceExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, &PhysicalDeviceAccelerationStructureFeatures, sizeof(VkPhysicalDeviceAccelerationStructureFeaturesKHR));

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR PhysicalDeviceRayTracingPipelineFeatures{};
    PhysicalDeviceRayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    PhysicalDeviceRayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
    VulkanContextOptions.AddDeviceExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, &PhysicalDeviceRayTracingPipelineFeatures, sizeof(VkPhysicalDeviceRayTracingPipelineFeaturesKHR));

    VkPhysicalDeviceBufferDeviceAddressFeatures PhysicalDeviceBufferDeviceAddressFeatures{};
    PhysicalDeviceBufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    PhysicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
    VulkanContextOptions.AddDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, &PhysicalDeviceBufferDeviceAddressFeatures, sizeof(VkPhysicalDeviceBufferDeviceAddressFeatures));

    VkPhysicalDeviceHostQueryResetFeatures PhysicalDeviceHostQueryResetFeatures{};
    PhysicalDeviceHostQueryResetFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES;
    PhysicalDeviceHostQueryResetFeatures.hostQueryReset = VK_TRUE;
    VulkanContextOptions.AddDeviceExtension(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME, &PhysicalDeviceHostQueryResetFeatures, sizeof(VkPhysicalDeviceHostQueryResetFeatures));

    VkPhysicalDeviceMaintenance4Features PhysicalDeviceMaintenance4Features{};
    PhysicalDeviceMaintenance4Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES;
    PhysicalDeviceMaintenance4Features.maintenance4 = VK_TRUE;
    VulkanContextOptions.AddDeviceExtension(VK_KHR_MAINTENANCE_4_EXTENSION_NAME, &PhysicalDeviceMaintenance4Features, sizeof(PhysicalDeviceMaintenance4Features));

    VulkanContextOptions.AddDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    VulkanContextOptions.AddDeviceExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    VulkanContextOptions.AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    /// Create Vulkan instance
    Instance = Context.CreateVkInstance("Hello Triangle", {1, 3, 0}, "No Engine", {1, 3, 0}, VK_API_VERSION_1_3, VulkanContextOptions);
    Context.SetInstance(Instance);

    /// Load Vulkan options
    V::LoadVkFunctions(Instance);

#ifndef NDEBUG
    /// Create debug messenger
    VkDebugUtilsMessengerEXT DebugUtilsMessengerEXT = Context.CreateDebugMessenger(VulkanContextOptions);
    Context.SetDebugUtilsMessengerEXT(DebugUtilsMessengerEXT);
#endif
    /// Create Surface
    Surface = Context.CreateSurface(WINDOW());
    Context.SetSurface(Surface);

    /// Pick Physical device
    PhysicalDevice = Context.PickPhysicalDevice(VulkanContextOptions, Surface);
    Context.SetPhysicalDevice(PhysicalDevice);

    /// Create Logical device
    LogicalDevice = Context.CreateLogicalDevice(PhysicalDevice, VulkanContextOptions);
    Context.SetLogicalDevice(LogicalDevice);

    Context.GetDeviceQueues(Surface);

    Context.InitManagerResources();
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
    auto& Context = GetContext();

    Swapchain = std::make_shared<FSwapchain>(Width, Height, PhysicalDevice, LogicalDevice, Surface, Context.GetGraphicsQueueIndex(), Context.GetPresentIndex(), VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_PRESENT_MODE_MAILBOX_KHR);
    SetMaxFramesInFlight(Swapchain->Size());
    for (int i = 0; i < MaxFramesInFlight; ++i)
    {
        auto Framebuffer = CreateFramebufferFromExternalImage(Swapchain->Images[i], "Swapchain Framebuffer Image");
        SetOutput(OutputType(i), Framebuffer);
    }

    for (int i = 0; i < MaxFramesInFlight; ++i)
    {
        ImageAvailableSemaphores.push_back(Context.CreateSemaphore());
        ImagesInFlight.push_back(Context.CreateSignalledFence());
    }

    GenerateRaysTask = std::make_shared<FGenerateInitialRays>(Width, Height, &Context, MaxFramesInFlight, LogicalDevice);
    GenerateRaysTask->Init();
    GenerateRaysTask->UpdateDescriptorSets();
    GenerateRaysTask->RecordCommands();

    RayTraceTask = std::make_shared<FRaytraceTask>(Width, Height, &Context, MaxFramesInFlight, LogicalDevice);

    RayTraceTask->Init();
    RayTraceTask->UpdateDescriptorSets();
    RayTraceTask->RecordCommands();

    ClearMaterialsCountPerChunkTask = std::make_shared<FClearMaterialsCountPerChunkTask>(Width, Height, &Context, MaxFramesInFlight, LogicalDevice);

    ClearMaterialsCountPerChunkTask->Init();
    ClearMaterialsCountPerChunkTask->UpdateDescriptorSets();
    ClearMaterialsCountPerChunkTask->RecordCommands();

    ClearTotalMaterialsCountTask = std::make_shared<FClearTotalMaterialsCountTask>(Width, Height, &Context, MaxFramesInFlight, LogicalDevice);

    ClearTotalMaterialsCountTask->Init();
    ClearTotalMaterialsCountTask->UpdateDescriptorSets();
    ClearTotalMaterialsCountTask->RecordCommands();

    CountMaterialsPerChunkTask = std::make_shared<FCountMaterialsPerChunkTask>(Width, Height, &Context, MaxFramesInFlight, LogicalDevice);

    CountMaterialsPerChunkTask->Init();
    CountMaterialsPerChunkTask->UpdateDescriptorSets();
    CountMaterialsPerChunkTask->RecordCommands();

    ComputePrefixSumsUpSweepTask = std::make_shared<FComputePrefixSumsUpSweepTask>(Width, Height, &Context, MaxFramesInFlight, LogicalDevice);

    ComputePrefixSumsUpSweepTask->Init();
    ComputePrefixSumsUpSweepTask->UpdateDescriptorSets();
    ComputePrefixSumsUpSweepTask->RecordCommands();

    ComputePrefixSumsZeroOutTask = std::make_shared<FComputePrefixSumsZeroOutTask>(Width, Height, &Context, MaxFramesInFlight, LogicalDevice);

    ComputePrefixSumsZeroOutTask->Init();
    ComputePrefixSumsZeroOutTask->UpdateDescriptorSets();
    ComputePrefixSumsZeroOutTask->RecordCommands();

    ComputePrefixSumsDownSweepTask = std::make_shared<FComputePrefixSumsDownSweepTask>(Width, Height, &Context, MaxFramesInFlight, LogicalDevice);

    ComputePrefixSumsDownSweepTask->Init();
    ComputePrefixSumsDownSweepTask->UpdateDescriptorSets();
    ComputePrefixSumsDownSweepTask->RecordCommands();

    ComputeOffsetsPerMaterialTask = std::make_shared<FComputeOffsetsPerMaterialTask>(Width, Height, &Context, MaxFramesInFlight, LogicalDevice);

    ComputeOffsetsPerMaterialTask->Init();
    ComputeOffsetsPerMaterialTask->UpdateDescriptorSets();
    ComputeOffsetsPerMaterialTask->RecordCommands();

    SortMaterialsTask = std::make_shared<FSortMaterialsTask>(Width, Height, &Context, MaxFramesInFlight, LogicalDevice);

    SortMaterialsTask->Init();
    SortMaterialsTask->UpdateDescriptorSets();
    SortMaterialsTask->RecordCommands();

    ShadeTask = std::make_shared<FShadeTask>(Width, Height, &Context, MaxFramesInFlight, LogicalDevice);
    ShadeTask->Init();
    ShadeTask->UpdateDescriptorSets();
    ShadeTask->RecordCommands();

    MissTask = std::make_shared<FMissTask>(Width, Height, &Context, MaxFramesInFlight, LogicalDevice);
    SetIBL("../../../resources/brown_photostudio_02_4k.exr");
    MissTask->Init();
    MissTask->UpdateDescriptorSets();
    MissTask->RecordCommands();

    AccumulateTask = std::make_shared<FAccumulateTask>(Width, Height, &Context, MaxFramesInFlight, LogicalDevice);
    AccumulateTask->Init();
    AccumulateTask->UpdateDescriptorSets();
    AccumulateTask->RecordCommands();

    ClearImageTask = std::make_shared<FClearImageTask>(Width, Height, &Context, MaxFramesInFlight, LogicalDevice);
    ClearImageTask->Init();
    ClearImageTask->UpdateDescriptorSets();
    ClearImageTask->RecordCommands();

    PassthroughTask = std::make_shared<FPassthroughTask>(Width, Height, &Context, MaxFramesInFlight, LogicalDevice);
    for (int i = 0; i < MaxFramesInFlight; ++i)
    {
        auto& FramebufferComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FFramebufferComponent>(OutputToFramebufferMap[OutputType(i)]);
        PassthroughTask->RegisterOutput(i, GetTextureManager()->GetFramebufferImage(FramebufferComponent.FramebufferImageIndex));
    }
    PassthroughTask->Init();
    PassthroughTask->UpdateDescriptorSets();
    PassthroughTask->RecordCommands();

    ImguiTask = std::make_shared<FImguiTask>(Width, Height, &Context, MaxFramesInFlight, LogicalDevice);
    for (int i = 0; i < MaxFramesInFlight; ++i)
    {
        auto& FramebufferComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FFramebufferComponent>(OutputToFramebufferMap[OutputType(i)]);
        ImguiTask->RegisterOutput(i, GetTextureManager()->GetFramebufferImage(FramebufferComponent.FramebufferImageIndex));
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
        vkDestroySemaphore(GetContext().LogicalDevice, ImageAvailableSemaphores[i], nullptr);
        ImageAvailableSemaphores[i] = VK_NULL_HANDLE;
        vkDestroyFence(GetContext().LogicalDevice, ImagesInFlight[i], nullptr);
        ImagesInFlight[i] = VK_NULL_HANDLE;
    }

    ImageAvailableSemaphores.clear();
    ImagesInFlight.clear();

    Swapchain = nullptr;

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
    auto& Coordinator = COORDINATOR();

    ECS::FEntity Camera = Coordinator.CreateEntity();
    Coordinator.AddComponent<ECS::COMPONENTS::FDeviceCameraComponent>(Camera, {});
    Coordinator.AddComponent<ECS::COMPONENTS::FCameraComponent>(Camera, {});

    return Camera;
}

ECS::FEntity FRender::CreateFramebuffer(int WidthIn, int HeightIn, const std::string& DebugName)
{
    auto FramebufferImage = GetTextureManager()->CreateStorageImage(WidthIn, HeightIn, DebugName);
    static int Counter = 0;
    auto FramebufferImageIndex = GetTextureManager()->RegisterFramebuffer(FramebufferImage, (DebugName == "") ? ("Unnamed Framebuffer " + std::to_string(Counter++)) : DebugName);

    auto& Coordinator = COORDINATOR();
    ECS::FEntity Framebuffer = Coordinator.CreateEntity();
    Coordinator.AddComponent<ECS::COMPONENTS::FFramebufferComponent>(Framebuffer, {FramebufferImageIndex});

    return Framebuffer;
}


ECS::FEntity FRender::CreateFramebufferFromExternalImage(ImagePtr ImageIn, const std::string& DebugName)
{
    static int Counter = 0;
    auto FramebufferImageIndex = GetTextureManager()->RegisterFramebuffer(ImageIn, (DebugName == "") ? ("Unnamed Framebuffer From External Image " + std::to_string(Counter++)) : DebugName);

    auto& Coordinator = COORDINATOR();
    ECS::FEntity Framebuffer = Coordinator.CreateEntity();
    Coordinator.AddComponent<ECS::COMPONENTS::FFramebufferComponent>(Framebuffer, {FramebufferImageIndex});

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

void FRender::SaveFramebuffer(ECS::FEntity Framebuffer)
{
    auto& FramebufferComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FFramebufferComponent>(Framebuffer);
    GetContext().SaveImage(*GetTextureManager()->GetFramebufferImage(FramebufferComponent.FramebufferImageIndex));
}

void FRender::GetFramebufferData(ECS::FEntity Framebuffer)
{
    auto& FramebufferComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FFramebufferComponent>(Framebuffer);
    auto Image = GetTextureManager()->GetFramebufferImage(FramebufferComponent.FramebufferImageIndex);
    std::vector<char> Data;

    GetContext().FetchImageData(*Image, Data);
}

int FRender::Destroy()
{
    GetContext().WaitIdle();
    Cleanup();

    ACCELERATION_STRUCTURE_SYSTEM()->Terminate();
    MESH_SYSTEM()->Terminate();

    GetContext().CleanUp();

    return 0;
}

int FRender::Render()
{
    if (WINDOW_MANAGER()->ShouldClose())
    {
        return 1;
    }

    auto& Context = GetContext();

    Context.TimingManager->NewTime();

    uint32_t CurrentFrame = RenderFrameIndex % MaxFramesInFlight;

    /// Previous rendering iteration of the frame might still be in use, so we wait for it
    vkWaitForFences(Context.LogicalDevice, 1, &ImagesInFlight[CurrentFrame], VK_TRUE, UINT64_MAX);

    /// Acquire next image from swapchain, also it's index and provide semaphore to signal when image is ready to be used
    uint32_t ImageIndex = 0;
    VkResult Result = Swapchain->GetNextImage(nullptr, ImageAvailableSemaphores[CurrentFrame], ImageIndex);

    /// Run some checks
    if (Result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        bShouldRecreateSwapchain = true;
        return 0;
    }
    if (Result != VK_SUCCESS && Result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    bool NeedUpdate = true;
    CAMERA_SYSTEM()->Update();
    TRANSFORM_SYSTEM()->Update();
    RENDERABLE_SYSTEM()->Update();
    LIGHT_SYSTEM()->Update();
    ACCELERATION_STRUCTURE_SYSTEM()->Update();

    auto GenerateRaysSemaphore = GenerateRaysTask->Submit(Context.GetComputeQueue(), ImageAvailableSemaphores[CurrentFrame], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, ImagesInFlight[CurrentFrame], VK_NULL_HANDLE, CurrentFrame);

    auto RayTraceSignalSemaphore = RayTraceTask->Submit(Context.GetGraphicsQueue(), GenerateRaysSemaphore, GenerateRaysTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto ClearMaterialsCountPerChunkSemaphore = ClearMaterialsCountPerChunkTask->Submit(Context.GetComputeQueue(), RayTraceSignalSemaphore, RayTraceTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto ClearTotalMaterialsCountSemaphore = ClearTotalMaterialsCountTask->Submit(Context.GetComputeQueue(), ClearMaterialsCountPerChunkSemaphore, ClearMaterialsCountPerChunkTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto CountMaterialsPerChunkSemaphore = CountMaterialsPerChunkTask->Submit(Context.GetComputeQueue(), ClearTotalMaterialsCountSemaphore, ClearTotalMaterialsCountTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto ComputePrefixSumsUpSweepSemaphore = ComputePrefixSumsUpSweepTask->Submit(Context.GetComputeQueue(), CountMaterialsPerChunkSemaphore, CountMaterialsPerChunkTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto ComputePrefixSumsZeroOutSemaphore = ComputePrefixSumsZeroOutTask->Submit(Context.GetComputeQueue(), ComputePrefixSumsUpSweepSemaphore, ComputePrefixSumsUpSweepTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto ComputePrefixSumsDownSweepSemaphore = ComputePrefixSumsDownSweepTask->Submit(Context.GetComputeQueue(), ComputePrefixSumsZeroOutSemaphore, ComputePrefixSumsZeroOutTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto ComputeOffsetsPerMaterialSemaphore = ComputeOffsetsPerMaterialTask->Submit(Context.GetComputeQueue(), ComputePrefixSumsDownSweepSemaphore, ComputePrefixSumsDownSweepTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto SortMaterialsSemaphore = SortMaterialsTask->Submit(Context.GetComputeQueue(), ComputeOffsetsPerMaterialSemaphore, ComputeOffsetsPerMaterialTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto ShadeSignalSemaphore = ShadeTask->Submit(Context.GetComputeQueue(), SortMaterialsSemaphore, SortMaterialsTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto MissSignalSemaphore = MissTask->Submit(Context.GetComputeQueue(), ShadeSignalSemaphore, ShadeTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    VkSemaphore AccumulateSignalSemaphore = VK_NULL_HANDLE;
    VkPipelineStageFlags PipelineStageFlags = 0;

    if (NeedUpdate)
    {
        auto ClearAccumulatorSemaphore = ClearImageTask->Submit(Context.GetComputeQueue(), MissSignalSemaphore, MissTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);
        AccumulateSignalSemaphore = AccumulateTask->Submit(Context.GetComputeQueue(), ClearAccumulatorSemaphore, ClearImageTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);
        PipelineStageFlags = ClearImageTask->GetPipelineStageFlags();
    }
    else
    {
        AccumulateSignalSemaphore = AccumulateTask->Submit(Context.GetComputeQueue(), MissSignalSemaphore, MissTask->GetPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);
        PipelineStageFlags = AccumulateTask->GetPipelineStageFlags();
    }

    auto PassthroughSignalSemaphore = PassthroughTask->Submit(Context.GetGraphicsQueue(), AccumulateSignalSemaphore, PipelineStageFlags, VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto ImguiFinishedSemaphore = ImguiTask->Submit(Context.GetGraphicsQueue(), PassthroughSignalSemaphore, PassthroughTask->GetPipelineStageFlags(), VK_NULL_HANDLE, ImagesInFlight[ImageIndex], CurrentFrame);

    Result = Context.Present(Swapchain->GetSwapchain(), ImguiFinishedSemaphore, CurrentFrame);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR)
    {
        bShouldRecreateSwapchain = true;
        return 1;
    }

    RenderFrameIndex++;

    glfwPollEvents();

    return 0;
}

int FRender::Update()
{
    auto& Coordinator = COORDINATOR();
    auto& LightComponent = Coordinator.GetComponent<ECS::COMPONENTS::FLightComponent>(Lights.back());
    LightComponent.Position.SelfRotateY(0.025f);
    LIGHT_SYSTEM()->SetLightPosition(Lights.back(), LightComponent.Position.X, LightComponent.Position.Y, LightComponent.Position.Z);

    if (bShouldRecreateSwapchain)
    {
        GetContext().WaitIdle();
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
    ImagePtr IBLImage = GetContext().CreateEXRImageFromFile(Path, "V::IBL_Image");
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
    auto& Coordinator = COORDINATOR();
    auto LightSystem = Coordinator.GetSystem<ECS::SYSTEMS::FLightSystem>();

    Lights.push_back(Coordinator.CreateEntity());
    Coordinator.AddComponent<ECS::COMPONENTS::FLightComponent>(Lights.back(), {});
    LIGHT_SYSTEM()->SetLightPosition(Lights.back(), Position.X, Position.Y, Position.Z);

    return 0;
}

ECS::FEntity FRender::CreateEmptyModel()
{
    auto& Coordinator = COORDINATOR();

    ECS::FEntity EmptyModel = Coordinator.CreateEntity();
    Coordinator.AddComponent<ECS::COMPONENTS::FMeshComponent>(EmptyModel, {});
    Coordinator.AddComponent<ECS::COMPONENTS::FDeviceMeshComponent>(EmptyModel, {});

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
