#pragma once

#include "GLFW/glfw3.h"

#include "image.h"
#include "swapchain.h"

#include "maths.h"

#include "entities/entity.h"

#include "task_render.h"
#include "task_passthrough.h"
#include "task_imgui.h"

#include <string>
#include <vector>

enum MeshType {Tetrahedron, Hexahedron, Icosahedron, Model};

class FRender
{
public:
    FRender();
    ~FRender();

    int Init();
    int Cleanup();
    int SetSize(int Width, int Height);

    int Render();
    int Update();

    int LoadDataToGPU();
    int LoadScene(const std::string& Path);
    int AddMesh(const FVector3& Color, const FVector3& Position, MeshType Type, const std::string& Path, uint32_t RenderableMask);

    VkInstance Instance = VK_NULL_HANDLE;
    VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
    VkDevice LogicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR Surface = VK_NULL_HANDLE;

    GLFWwindow* Window;

    std::shared_ptr<FSwapchain> Swapchain = nullptr;
    bool bShouldRecreateSwapchain = false;

    uint32_t WINDOW_WIDTH = 1920;
    uint32_t WINDOW_HEIGHT = 1080;
    const std::string WINDOW_NAME = "RTracer";
    const int MAX_FRAMES_IN_FLIGHT = 2;
    uint32_t RenderFrameIndex = 0;

    ImagePtr UtilityImageR32 = nullptr;
    ImagePtr UtilityImageR8G8B8A8_SRGB = nullptr;

    std::vector<ECS::FEntity> Models;

    std::shared_ptr<FRenderTask> RenderTask = nullptr;
    std::shared_ptr<FPassthroughTask> PassthroughTask = nullptr;
    std::shared_ptr<FImguiTask> ImguiTask = nullptr;

    std::vector<VkSemaphore> ImageAvailableSemaphores;
    std::vector<VkSemaphore> ImGuiFinishedSemaphores;
    std::vector<VkFence> ImagesInFlight;
};
