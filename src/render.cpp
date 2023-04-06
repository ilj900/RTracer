#include "coordinator.h"
#include "systems/mesh_system.h"
#include "systems/transform_system.h"
#include "systems/renderable_system.h"
#include "systems/camera_system.h"
#include "components/mesh_component.h"
#include "components/device_mesh_component.h"
#include "components/device_renderable_component.h"
#include "components/transform_component.h"
#include "components/device_transform_component.h"

#include "vk_context.h"

#include "vk_functions.h"
#include "render.h"

FRender::FRender()
{
    /// Create GLFW Window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    Window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME.c_str(), nullptr, nullptr);
    glfwSetWindowPos(Window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    glfwSetCursorPos(Window, 0.f, 0.f);

    LoadScene("");

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

    Context.InitManagerResources();

    CAMERA_SYSTEM()->Init(MAX_FRAMES_IN_FLIGHT);
    TRANSFORM_SYSTEM()->Init(MAX_FRAMES_IN_FLIGHT);
    RENDERABLE_SYSTEM()->Init(MAX_FRAMES_IN_FLIGHT);
    MESH_SYSTEM()->Init(MAX_FRAMES_IN_FLIGHT);

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

    Swapchain = std::make_shared<FSwapchain>(WINDOW_WIDTH, WINDOW_HEIGHT, PhysicalDevice, LogicalDevice, Surface, Context.GetGraphicsQueueIndex(), Context.GetPresentIndex(), VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_PRESENT_MODE_MAILBOX_KHR);

    UtilityImageR32 = Context.CreateImage2D(WINDOW_WIDTH, WINDOW_HEIGHT, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL,
                                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                            VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_UtilityImageR32");

    UtilityImageR32->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);


    UtilityImageR8G8B8A8_SRGB = Context.CreateImage2D(WINDOW_WIDTH, WINDOW_HEIGHT, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                                                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                      VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_UtilityImageR8G8B8A8_SRGB");

    auto ColorImage = Context.CreateImage2D(WINDOW_WIDTH, WINDOW_HEIGHT, false, Context.MSAASamples, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                            VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_ColorImage");


    auto NormalsImage = Context.CreateImage2D(WINDOW_WIDTH, WINDOW_HEIGHT, false, Context.MSAASamples, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                              VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_NormalsImage");


    auto RenderableIndexImage = Context.CreateImage2D(WINDOW_WIDTH, WINDOW_HEIGHT, false, Context.MSAASamples, VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL,
                                                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                      VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_RenderableIndexImage");

    auto ResolvedColorImage = Context.CreateImage2D(WINDOW_WIDTH, WINDOW_HEIGHT, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                    VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_ResolvedColorImage");

    auto RTColorImage = Context.CreateImage2D(WINDOW_WIDTH, WINDOW_HEIGHT, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                              VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_RayTracingColorImage");
    RTColorImage->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

//    RenderTask = std::make_shared<FRenderTask>(WINDOW_WIDTH, WINDOW_HEIGHT, &Context, MAX_FRAMES_IN_FLIGHT, LogicalDevice);
//    RenderTask->RegisterOutput(0, ColorImage);
//    RenderTask->RegisterOutput(1, NormalsImage);
//    RenderTask->RegisterOutput(2, RenderableIndexImage);
//    RenderTask->RegisterOutput(3, ResolvedColorImage);
//
//    RenderTask->Init();
//    RenderTask->UpdateDescriptorSets();
//    RenderTask->RecordCommands();

    RayTraceTask = std::make_shared<FRaytraceTask>(WINDOW_WIDTH, WINDOW_HEIGHT, &Context, MAX_FRAMES_IN_FLIGHT, LogicalDevice);
    RayTraceTask->RegisterOutput(0, RTColorImage);
    SetIBL("../resources/brown_photostudio_02_4k.exr");

    RayTraceTask->Init();
    RayTraceTask->UpdateDescriptorSets();
    RayTraceTask->RecordCommands();

    PassthroughTask = std::make_shared<FPassthroughTask>(WINDOW_WIDTH, WINDOW_HEIGHT, &Context, MAX_FRAMES_IN_FLIGHT, LogicalDevice);
    PassthroughTask->RegisterInput(0, RayTraceTask->GetOutput(0));
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

    CAMERA_SYSTEM()->RequestAllUpdate();
    TRANSFORM_SYSTEM()->RequestAllUpdate();
    RENDERABLE_SYSTEM()->RequestAllUpdate();

    return 0;
}

int FRender::Cleanup()
{
    UtilityImageR8G8B8A8_SRGB = nullptr;
    UtilityImageR32 = nullptr;

//    RenderTask->Cleanup();
//    RenderTask = nullptr;
    RayTraceTask->Cleanup();
    RayTraceTask = nullptr;
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

    CAMERA_SYSTEM()->Update();
    TRANSFORM_SYSTEM()->Update();
    RENDERABLE_SYSTEM()->Update();

    auto RenderSignalSemaphore = RayTraceTask->Submit(Context.GetGraphicsQueue(), ImageAvailableSemaphores[CurrentFrame], ImagesInFlight[CurrentFrame], VK_NULL_HANDLE, CurrentFrame);

    auto PassthroughSignalSemaphore = PassthroughTask->Submit(Context.GetGraphicsQueue(), RenderSignalSemaphore, VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

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
    const uint32_t RENDERABLE_SELECTED_BIT = 1 << 5;
    const uint32_t RENDERABLE_HAS_TEXTURE = 1 << 6;
    const uint32_t RENDERABLE_IS_INDEXED = 1 << 7;

    AddMesh({0.9f, 0.6f, 0.0f}, {-3.f, 0.f, -2.f}, Tetrahedron, std::string(), 0);
    AddMesh({0.0f, 0.9f, 0.6f}, {1.f, 0.f, -2.f}, Hexahedron, std::string(), 0);
    AddMesh({0.6f, 0.0f, 0.9f}, {3.f, 0.f, -2.f}, Icosahedron, std::string(), 0);

    AddMesh({0.9f, 0.0f, 0.6f}, {-1.f, 0.f, -2.f}, Model, "../models/viking_room/viking_room.obj", RENDERABLE_HAS_TEXTURE | RENDERABLE_IS_INDEXED);

    return 0;
}

int FRender::SetIBL(const std::string& Path)
{
    ImagePtr IBLImage = GetContext().CreateEXRImageFromFile(Path, "V::IBL_Image");
    IBLImage->Transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    RayTraceTask->RegisterInput(0, IBLImage);

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

int FRender::AddMesh(const FVector3& Color, const FVector3& Position, MeshType Type, const std::string& Path, uint32_t RenderableMask)
{
    auto& Coordinator = ECS::GetCoordinator();
    auto MeshSystem = Coordinator.GetSystem<ECS::SYSTEMS::FMeshSystem>();

    Models.push_back(Coordinator.CreateEntity());
    Coordinator.AddComponent<ECS::COMPONENTS::FMeshComponent>(Models.back(), {});
    Coordinator.AddComponent<ECS::COMPONENTS::FDeviceMeshComponent>(Models.back(), {});
    static uint32_t Index = 0;
    Coordinator.AddComponent<ECS::COMPONENTS::FDeviceRenderableComponent>
            (Models.back(), {FVector3{1.f, 1.f, 1.f}, Index++, RenderableMask});
    Coordinator.AddComponent<ECS::COMPONENTS::FTransformComponent>(Models.back(), {});
    Coordinator.AddComponent<ECS::COMPONENTS::FDeviceTransformComponent>(Models.back(), {});
    switch(Type)
    {
        case Tetrahedron:
            MeshSystem->CreateTetrahedron(Models.back());
            break;
        case Hexahedron:
            MeshSystem->CreateHexahedron(Models.back());
            break;
        case Icosahedron:
            MeshSystem->CreateIcosahedron(Models.back(), 10);
            break;
        case Model:
            MeshSystem->LoadMesh(Models.back(), Path);
            break;
    }
    TRANSFORM_SYSTEM()->SetTransform(Models.back(), Position, {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f});
    RENDERABLE_SYSTEM()->SetRenderableColor(Models.back(), Color.X, Color.Y, Color.Z);
    TRANSFORM_SYSTEM()->UpdateDeviceComponentData(Models.back());

    return 0;
}
