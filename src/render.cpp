#include "coordinator.h"
#include "systems/mesh_system.h"
#include "systems/transform_system.h"
#include "systems/renderable_system.h"
#include "systems/camera_system.h"
#include "systems/material_system.h"
#include "systems/light_system.h"
#include "components/mesh_component.h"
#include "components/device_mesh_component.h"
#include "components/device_renderable_component.h"
#include "components/transform_component.h"
#include "components/device_camera_component.h"
#include "components/device_transform_component.h"
#include "components/material_component.h"
#include "components/light_component.h"

#include "vk_context.h"

#include "vk_functions.h"
#include "render.h"
#include "texture_manager.h"

#include "logging.h"

int32_t FRender::Index = 0;

FRender::FRender()
{
    /// Create GLFW Window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    Window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME.c_str(), nullptr, nullptr);
    glfwSetWindowPos(Window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    glfwSetCursorPos(Window, 0.f, 0.f);

    auto& Context = GetContext();

    Context.SetWindow(Window);

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
    VulkanContextOptions.AddDeviceExtension(VK_KHR_RAY_QUERY_EXTENSION_NAME, &PhysicalDeviceHostQueryResetFeatures, sizeof(VkPhysicalDeviceHostQueryResetFeatures));

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
    Surface = Context.CreateSurface(Window);
    Context.SetSurface(Surface);

    /// Pick Physical device
    PhysicalDevice = Context.PickPhysicalDevice(VulkanContextOptions, Surface);
    Context.SetPhysicalDevice(PhysicalDevice);

    /// Create Logical device
    LogicalDevice = Context.CreateLogicalDevice(PhysicalDevice, VulkanContextOptions);
    Context.SetLogicalDevice(LogicalDevice);

    Context.GetDeviceQueues(Surface);

    Swapchain = std::make_shared<FSwapchain>(WINDOW_WIDTH, WINDOW_HEIGHT, PhysicalDevice, LogicalDevice, Surface, Context.GetGraphicsQueueIndex(), Context.GetPresentIndex(), VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_PRESENT_MODE_MAILBOX_KHR);
    MAX_FRAMES_IN_FLIGHT = Swapchain->Size();

    Context.InitManagerResources();
    CAMERA_SYSTEM()->Init(MAX_FRAMES_IN_FLIGHT);
    RENDERABLE_SYSTEM()->Init(MAX_FRAMES_IN_FLIGHT);
    MESH_SYSTEM()->Init(MAX_FRAMES_IN_FLIGHT);
    MATERIAL_SYSTEM()->Init(MAX_FRAMES_IN_FLIGHT);
    LIGHT_SYSTEM()->Init(MAX_FRAMES_IN_FLIGHT);
    TRANSFORM_SYSTEM()->Init(MAX_FRAMES_IN_FLIGHT);

    LoadScene("");
    LoadDataToGPU();

    ImGuiFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        ImageAvailableSemaphores.push_back(Context.CreateSemaphore());
        ImagesInFlight.push_back(Context.CreateSignalledFence());
    }

    Context.Init(WINDOW_WIDTH, WINDOW_HEIGHT);

    Init();
}

int FRender::Init()
{
    auto& Context = GetContext();

    auto RTColorImage = Context.CreateImage2D(WINDOW_WIDTH, WINDOW_HEIGHT, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                              VK_IMAGE_USAGE_STORAGE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                              VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_RayTracingColorImage");
    RTColorImage->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    auto AccumulatorImage = Context.CreateImage2D(WINDOW_WIDTH, WINDOW_HEIGHT, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                                  VK_IMAGE_USAGE_STORAGE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                  VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_AccumulatorImage");
    AccumulatorImage->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    auto EstimatedImage = Context.CreateImage2D(WINDOW_WIDTH, WINDOW_HEIGHT, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                                  VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                  VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_EstimatedImage");
    EstimatedImage->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    GenerateRaysTask = std::make_shared<FGenerateInitialRays>(WINDOW_WIDTH, WINDOW_HEIGHT, &Context, MAX_FRAMES_IN_FLIGHT, LogicalDevice);
    GenerateRaysTask->Init();
    GenerateRaysTask->UpdateDescriptorSets();
    GenerateRaysTask->RecordCommands();

    RayTraceTask = std::make_shared<FRaytraceTask>(WINDOW_WIDTH, WINDOW_HEIGHT, &Context, MAX_FRAMES_IN_FLIGHT, LogicalDevice);

    RayTraceTask->Init();
    RayTraceTask->UpdateDescriptorSets();
    RayTraceTask->RecordCommands();

    ShadeTask = std::make_shared<FShadeTask>(WINDOW_WIDTH, WINDOW_HEIGHT, &Context, MAX_FRAMES_IN_FLIGHT, LogicalDevice);
    ShadeTask->RegisterInput(0, RTColorImage);
    SetIBL("../resources/brown_photostudio_02_4k.exr");
    ShadeTask->Init();
    ShadeTask->UpdateDescriptorSets();
    ShadeTask->RecordCommands();

    AccumulateTask = std::make_shared<FAccumulateTask>(WINDOW_WIDTH, WINDOW_HEIGHT, &Context, MAX_FRAMES_IN_FLIGHT, LogicalDevice);
    AccumulateTask->RegisterInput(0, RTColorImage);
    AccumulateTask->RegisterOutput(0, AccumulatorImage);
    AccumulateTask->RegisterOutput(1, EstimatedImage);

    AccumulateTask->Init();
    AccumulateTask->UpdateDescriptorSets();
    AccumulateTask->RecordCommands();

    ClearImageTask = std::make_shared<FClearImageTask>(WINDOW_WIDTH, WINDOW_HEIGHT, &Context, MAX_FRAMES_IN_FLIGHT, LogicalDevice);
    ClearImageTask->RegisterOutput(0, AccumulatorImage);

    ClearImageTask->Init();
    ClearImageTask->UpdateDescriptorSets();
    ClearImageTask->RecordCommands();

    PassthroughTask = std::make_shared<FPassthroughTask>(WINDOW_WIDTH, WINDOW_HEIGHT, &Context, MAX_FRAMES_IN_FLIGHT, LogicalDevice);
    PassthroughTask->RegisterInput(0, EstimatedImage);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        PassthroughTask->RegisterOutput(i, Swapchain->Images[i]);
    }
    PassthroughTask->Init();
    PassthroughTask->UpdateDescriptorSets();
    PassthroughTask->RecordCommands();

    ImguiTask = std::make_shared<FImguiTask>(WINDOW_WIDTH, WINDOW_HEIGHT, &Context, MAX_FRAMES_IN_FLIGHT, LogicalDevice);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        ImguiTask->RegisterOutput(i, Swapchain->Images[i]);
    }
    ImguiTask->Init();

    RenderFrameIndex = 0;

    return 0;
}

int FRender::Cleanup()
{
    GenerateRaysTask->Cleanup();
    GenerateRaysTask = nullptr;
    RayTraceTask->Cleanup();
    RayTraceTask = nullptr;
    ShadeTask->Cleanup();
    ShadeTask = nullptr;
    AccumulateTask->Cleanup();
    AccumulateTask = nullptr;
    ClearImageTask->Cleanup();
    ClearImageTask = nullptr;
    PassthroughTask->Cleanup();
    PassthroughTask = nullptr;
    ImguiTask->Cleanup();
    ImguiTask = nullptr;

    Swapchain = nullptr;

    return 0;
}

int FRender::SetSize(int Width, int Height)
{
    WINDOW_WIDTH = Width;
    WINDOW_HEIGHT = Height;
    bShouldRecreateSwapchain = true;
    return 0;
}

FRender::~FRender()
{
    GetContext().WaitIdle();
    Cleanup();

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(GetContext().LogicalDevice, ImageAvailableSemaphores[i], nullptr);
        ImageAvailableSemaphores[i] = VK_NULL_HANDLE;
        vkDestroyFence(GetContext().LogicalDevice, ImagesInFlight[i], nullptr);
        ImagesInFlight[i] = VK_NULL_HANDLE;
    }

    ImageAvailableSemaphores.clear();
    ImagesInFlight.clear();

    GetContext().CleanUp();

    glfwDestroyWindow(Window);
    glfwTerminate();
}

int FRender::Render()
{
    if (glfwWindowShouldClose(Window))
    {
        return 1;
    }

    auto& Context = GetContext();

    uint32_t CurrentFrame = RenderFrameIndex % MAX_FRAMES_IN_FLIGHT;

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
    MATERIAL_SYSTEM()->Update();
    LIGHT_SYSTEM()->Update();

    std::vector<ECS::COMPONENTS::FDeviceCameraComponent> Data2 = Context.ResourceAllocator->DebugGetDataFromBuffer<ECS::COMPONENTS::FDeviceCameraComponent>(CAMERA_SYSTEM()->DeviceBuffer, CAMERA_SYSTEM()->GetTotalSize(), 0);

    auto GenerateRaysSemaphore = GenerateRaysTask->Submit(Context.GetComputeQueue(), ImageAvailableSemaphores[CurrentFrame], ImagesInFlight[CurrentFrame], VK_NULL_HANDLE, CurrentFrame);

    auto RenderSignalSemaphore = RayTraceTask->Submit(Context.GetGraphicsQueue(), GenerateRaysSemaphore, VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto ShadeSignalSemaphore = ShadeTask->Submit(Context.GetComputeQueue(), RenderSignalSemaphore, VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    VkSemaphore AccumulateSignalSemaphore = VK_NULL_HANDLE;

    if (NeedUpdate)
    {
        auto ClearAccumulatorSemaphore = ClearImageTask->Submit(Context.GetComputeQueue(), ShadeSignalSemaphore, VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);
        AccumulateSignalSemaphore = AccumulateTask->Submit(Context.GetComputeQueue(), ClearAccumulatorSemaphore, VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);
    }
    else
    {
        AccumulateSignalSemaphore = AccumulateTask->Submit(Context.GetComputeQueue(), ShadeSignalSemaphore, VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);
    }

    auto PassthroughSignalSemaphore = PassthroughTask->Submit(Context.GetGraphicsQueue(), AccumulateSignalSemaphore, VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto ImguiFinishedSemaphore = ImguiTask->Submit(Context.GetGraphicsQueue(), PassthroughSignalSemaphore, VK_NULL_HANDLE, ImagesInFlight[ImageIndex], CurrentFrame);

    ImGuiFinishedSemaphores[CurrentFrame] = ImguiFinishedSemaphore;

    Result = Context.Present(Swapchain->GetSwapchain(), ImGuiFinishedSemaphores[CurrentFrame], CurrentFrame);

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
    auto& Coordinator = ECS::GetCoordinator();
    auto LightComponent = Coordinator.GetComponent<ECS::COMPONENTS::FLightComponent>(Lights.back());
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
    Models.push_back(AddCube({0.0f, 0.9f, 0.6f}, {1.f, 0.f, -2.f}));
    Models.push_back(AddModel({0.9f, 0.0f, 0.6f}, {-1.f, 0.f, -2.f}, "../models/viking_room/viking_room.obj"));
    Models.push_back(AddSphere({0.6f, 0.0f, 0.9f}, {3.f, 0.f, -2.f}, 10));
    Models.push_back(AddPyramid({0.9f, 0.6f, 0.0f}, {-3.f, 0.f, -2.f}));
    Models.push_back(AddPlane({0.6f, 0.9f, 0.0f}, {-5.f, 0.f, -2.f}));
    Models.push_back(AddModel({0.9f, 0.0f, 0.6f}, {5.f, -1.f, -2.f}, "../models/Shaderball.obj"));

    AddLight({5, 5, 5});

    TRANSFORM_SYSTEM()->Update();

    return 0;
}

int FRender::SetIBL(const std::string& Path)
{
    ImagePtr IBLImage = GetContext().CreateEXRImageFromFile(Path, "V::IBL_Image");
    IBLImage->Transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    ShadeTask->RegisterInput(1, IBLImage);

    return 0;
}

int FRender::LoadDataToGPU()
{
    auto MeshSystem = MESH_SYSTEM();

    for(auto Mesh : *MeshSystem)
    {
        MeshSystem->LoadToGPU(Mesh);
    }

    auto RenderableSystem = RENDERABLE_SYSTEM();

    for (auto Renderable : *RenderableSystem)
    {
        RenderableSystem->SetRenderableDeviceAddress(Renderable, MeshSystem->GetVertexBufferAddress(Renderable), MeshSystem->GetIndexBufferAddress(Renderable));
    }

    return 0;
}

ECS::FEntity FRender::AddPlane(const FVector3& Color, const FVector3& Position)
{
    auto NewModel = CreateEmptyModel();

    MESH_SYSTEM()->CreatePlane(NewModel);
    TRANSFORM_SYSTEM()->SetTransform(NewModel, Position, {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f});
    RENDERABLE_SYSTEM()->SyncTransform(NewModel);
    RENDERABLE_SYSTEM()->SetRenderableColor(NewModel, Color.X, Color.Y, Color.Z);
    RENDERABLE_SYSTEM()->SetIndexed(NewModel);
    MATERIAL_SYSTEM()->SetBaseColor(NewModel, 1, 0, 1);

    return NewModel;
}

ECS::FEntity FRender::AddCube(const FVector3& Color, const FVector3& Position)
{
    auto NewModel = CreateEmptyModel();

    MESH_SYSTEM()->CreateHexahedron(NewModel);
    TRANSFORM_SYSTEM()->SetTransform(NewModel, Position, {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f});
    RENDERABLE_SYSTEM()->SyncTransform(NewModel);
    RENDERABLE_SYSTEM()->SetRenderableColor(NewModel, Color.X, Color.Y, Color.Z);
    MATERIAL_SYSTEM()->SetBaseColor(NewModel, 1, 0, 1);

    return NewModel;
}

ECS::FEntity FRender::AddSphere(const FVector3& Color, const FVector3& Position, int LevelOfComplexity)
{
    auto NewModel = CreateEmptyModel();

    MESH_SYSTEM()->CreateIcosahedron(NewModel, LevelOfComplexity, true);
    TRANSFORM_SYSTEM()->SetTransform(NewModel, Position, {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f});
    RENDERABLE_SYSTEM()->SyncTransform(NewModel);
    RENDERABLE_SYSTEM()->SetRenderableColor(NewModel, Color.X, Color.Y, Color.Z);
    MATERIAL_SYSTEM()->SetBaseColor(NewModel, 0, 1, 1);

    return NewModel;
}

ECS::FEntity FRender::AddModel(const FVector3& Color, const FVector3& Position, const std::string& Path)
{
    auto NewModel = CreateEmptyModel();

    MESH_SYSTEM()->LoadMesh(NewModel, Path);
    TRANSFORM_SYSTEM()->SetTransform(NewModel, Position, {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f});
    RENDERABLE_SYSTEM()->SyncTransform(NewModel);
    RENDERABLE_SYSTEM()->SetRenderableColor(NewModel, Color.X, Color.Y, Color.Z);
    RENDERABLE_SYSTEM()->SetIndexed(NewModel);
    RENDERABLE_SYSTEM()->SetRenderableHasTexture(NewModel);
    MATERIAL_SYSTEM()->SetBaseColor(NewModel, 1, 1, 1);

    return NewModel;
}

ECS::FEntity FRender::AddPyramid(const FVector3& Color, const FVector3& Position)
{
    auto NewModel = CreateEmptyModel();

    MESH_SYSTEM()->CreateTetrahedron(NewModel);
    TRANSFORM_SYSTEM()->SetTransform(NewModel, Position, {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f});
    RENDERABLE_SYSTEM()->SyncTransform(NewModel);
    RENDERABLE_SYSTEM()->SetRenderableColor(NewModel, Color.X, Color.Y, Color.Z);
    MATERIAL_SYSTEM()->SetBaseColor(NewModel, 1, 1, 0);

    return NewModel;
}

int FRender::AddLight(const FVector3& Position)
{
    auto& Coordinator = ECS::GetCoordinator();
    auto LightSystem = Coordinator.GetSystem<ECS::SYSTEMS::FLightSystem>();

    Lights.push_back(Coordinator.CreateEntity());
    Coordinator.AddComponent<ECS::COMPONENTS::FLightComponent>(Lights.back(), {});
    LIGHT_SYSTEM()->SetLightPosition(Lights.back(), Position.X, Position.Y, Position.Z);

    return 0;
}

ECS::FEntity FRender::CreateEmptyModel()
{
    auto& Coordinator = ECS::GetCoordinator();

    ECS::FEntity EmptyModel = Coordinator.CreateEntity();
    Coordinator.AddComponent<ECS::COMPONENTS::FMeshComponent>(EmptyModel, {});
    Coordinator.AddComponent<ECS::COMPONENTS::FDeviceMeshComponent>(EmptyModel, {});
    Coordinator.AddComponent<ECS::COMPONENTS::FDeviceRenderableComponent> (EmptyModel, {FVector3{1.f, 1.f, 1.f},Index++, 0, 0, 0});
    Coordinator.AddComponent<ECS::COMPONENTS::FTransformComponent>(EmptyModel, {});
    Coordinator.AddComponent<ECS::COMPONENTS::FDeviceTransformComponent>(EmptyModel, {});
    Coordinator.AddComponent<ECS::COMPONENTS::FMaterialComponent>(EmptyModel, {});

    return EmptyModel;
}
