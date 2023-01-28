#include "coordinator.h"
#include "systems/mesh_system.h"
#include "systems/transform_system.h"
#include "systems/renderable_system.h"
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

    LoadModels("");

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

    VulkanContextOptions.AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    /// Create Vulkan instance
    VkInstance Instance = Context.CreateVkInstance("Hello Triangle", {1, 0, 0}, "No Engine", {1, 0, 0}, VK_API_VERSION_1_0, VulkanContextOptions);
    Context.SetInstance(Instance);

    /// Load Vulkan options
    V::LoadVkFunctions(Instance);

#ifndef NDEBUG
    /// Create debug messenger
    VkDebugUtilsMessengerEXT DebugUtilsMessengerEXT = Context.CreateDebugMessenger(VulkanContextOptions);
    Context.SetDebugUtilsMessengerEXT(DebugUtilsMessengerEXT);
#endif

    /// Create Surface
    VkSurfaceKHR Surface = Context.CreateSurface(Window);
    Context.SetSurface(Surface);

    /// Pick Physical device
    VkPhysicalDevice PhysicalDevice = Context.PickPhysicalDevice(VulkanContextOptions, Surface);
    Context.SetPhysicalDevice(PhysicalDevice);

    /// Create Logical device
    VkDevice LogicalDevice = Context.CreateLogicalDevice(PhysicalDevice, VulkanContextOptions);
    Context.SetLogicalDevice(LogicalDevice);

    Context.GetDeviceQueues(Surface);

    Context.InitManagerResources(WINDOW_WIDTH, WINDOW_HEIGHT, Surface);

    UtilityImageR32 = Context.CreateImage2D(WINDOW_WIDTH, WINDOW_HEIGHT, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL,
                                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                    VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_UtilityImageR32");

    UtilityImageR32->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);


    UtilityImageR8G8B8A8_SRGB = Context.CreateImage2D(WINDOW_WIDTH, WINDOW_HEIGHT, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                              VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_UtilityImageR8G8B8A8_SRGB");

    LoadDataToGPU();

    Context.Init(Window, WINDOW_WIDTH, WINDOW_HEIGHT);
}

int FRender::Cleanup()
{
    UtilityImageR8G8B8A8_SRGB = nullptr;
    UtilityImageR32 = nullptr;

    return 0;
}

FRender::~FRender()
{
    GetContext().WaitIdle();
    Cleanup();
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
    vkWaitForFences(Context.LogicalDevice, 1, &Context.ImagesInFlight[CurrentFrame], VK_TRUE, UINT64_MAX);

    /// Acquire next image from swapchain, also it's index and provide semaphore to signal when image is ready to be used
    uint32_t ImageIndex = 0;
    VkResult Result = Context.Swapchain->GetNextImage(nullptr, Context.ImageAvailableSemaphores[CurrentFrame], ImageIndex);

    /// Run some checks
    if (Result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        Context.RecreateSwapChain(WINDOW_WIDTH, WINDOW_HEIGHT);
        return 1;
    }
    if (Result != VK_SUCCESS && Result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    Context.UpdateUniformBuffer(ImageIndex);

    auto RenderSignalSemaphore = Context.RenderTask-> Submit(Context.GetGraphicsQueue(), Context.ImageAvailableSemaphores[CurrentFrame], Context.ImagesInFlight[CurrentFrame], VK_NULL_HANDLE, CurrentFrame);

    auto PassthroughSignalSemaphore = Context.PassthroughTask->Submit(Context.GetGraphicsQueue(), RenderSignalSemaphore, VK_NULL_HANDLE, VK_NULL_HANDLE, CurrentFrame);

    auto ImguiFinishedSemaphore = Context.ImguiTask->Submit(Context.GetGraphicsQueue(), PassthroughSignalSemaphore, VK_NULL_HANDLE, Context.ImagesInFlight[ImageIndex], CurrentFrame);

    Context.ImGuiFinishedSemaphores[CurrentFrame] = ImguiFinishedSemaphore;

    Context.Present(Context.ImGuiFinishedSemaphores[CurrentFrame], CurrentFrame);

    RenderFrameIndex++;

    glfwPollEvents();

    return 0;
}

int FRender::LoadModels(const std::string& Path)
{
    const uint32_t RENDERABLE_HAS_TEXTURE = 1 << 6;

    AddMesh({0.6f, 0.0f, 0.9f}, {3.f, 0.f, -2.f}, Icosahedron, std::string(), 0);
    AddMesh({0.9f, 0.6f, 0.0f}, {-3.f, 0.f, -2.f}, Tetrahedron, std::string(), 0);
    AddMesh({0.0f, 0.9f, 0.6f}, {1.f, 0.f, -2.f}, Hexahedron, std::string(), 0);

    AddMesh({0.3f, 0.9f, 0.6f}, {-1.f, 0.f, -2.f}, Model, "../models/viking_room/viking_room.obj", RENDERABLE_HAS_TEXTURE);

    return 0;
}

int FRender::LoadDataToGPU()
{
    auto MeshSystem = ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMeshSystem>();

    for(auto Mesh : *MeshSystem)
    {
        MeshSystem->LoadToGPU(Mesh);
    }

    return 0;
}

int FRender::AddMesh(const FVector3& Color, const FVector3& Position, MeshType Type, const std::string& Path, uint32_t RenderableMask)
{
    auto& Coordinator = ECS::GetCoordinator();
    auto MeshSystem = Coordinator.GetSystem<ECS::SYSTEMS::FMeshSystem>();
    auto TransformSystem = Coordinator.GetSystem<ECS::SYSTEMS::FTransformSystem>();

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
    Coordinator.GetSystem<ECS::SYSTEMS::FTransformSystem>()->SetTransform(Models.back(), Position, {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f});
    Coordinator.GetSystem<ECS::SYSTEMS::FRenderableSystem>()->SetRenderableColor(Models.back(), Color.X, Color.Y, Color.Z);
    TransformSystem->UpdateDeviceComponentData(Models.back());

    return 0;
}
