#pragma once

#include "GLFW/glfw3.h"

#include "image.h"
#include "swapchain.h"

#include "maths.h"

#include "entity.h"

#include "task_generate_initial_rays.h"
#include "task_raytrace.h"
#include "task_material_sort_clear_materials_count.h"
#include "task_material_sort_count_materials.h"
#include "task_shade.h"
#include "task_accumulate.h"
#include "task_passthrough.h"
#include "task_imgui.h"
#include "task_clear_image.h"

#include <string>
#include <vector>

class FRender
{
public:
    FRender(uint32_t WidthIn = 1920, uint32_t HeightIn = 1080);

    int Init();
    int Cleanup();
    int Destroy();
    int SetSize(int WidthIn, int HeightIn);

    int Render();
    int Update();

    int LoadScene(const std::string& Path);
    ECS::FEntity CreateMaterial(const FVector3& BaseColor);
    ECS::FEntity ShapeSetMaterial(ECS::FEntity Shape, ECS::FEntity Material);
    ECS::FEntity CreatePlane();
    ECS::FEntity CreatePyramid();
    ECS::FEntity CreateCube();
    ECS::FEntity CreateSphere(int LevelOfComplexity);
    ECS::FEntity CreateModel(const std::string& Path);

    ECS::FEntity CreateInstance(ECS::FEntity BaseModel,  const FVector3& Position);

    int CreateLight(const FVector3& Position);
    int SetIBL(const std::string& Path);

    VkInstance Instance = VK_NULL_HANDLE;
    VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
    VkDevice LogicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR Surface = VK_NULL_HANDLE;

    std::shared_ptr<FSwapchain> Swapchain = nullptr;
    bool bShouldRecreateSwapchain = false;

    int MAX_FRAMES_IN_FLIGHT = 2;
    uint32_t RenderFrameIndex = 0;

    std::vector<ECS::FEntity> Models;
    std::vector<ECS::FEntity> Materials;
    std::vector<ECS::FEntity> Lights;

    std::shared_ptr<FGenerateInitialRays> GenerateRaysTask = nullptr;
    std::shared_ptr<FRaytraceTask> RayTraceTask = nullptr;
    std::shared_ptr<FClearMaterialsCountTask> ClearMaterialsCountTask = nullptr;
    std::shared_ptr<FCountMaterialsTask> CountMaterialsTask = nullptr;
    std::shared_ptr<FShadeTask> ShadeTask = nullptr;
    std::shared_ptr<FAccumulateTask> AccumulateTask = nullptr;
    std::shared_ptr<FPassthroughTask> PassthroughTask = nullptr;
    std::shared_ptr<FImguiTask> ImguiTask = nullptr;
    std::shared_ptr<FClearImageTask> ClearImageTask = nullptr;

    std::vector<VkSemaphore> ImageAvailableSemaphores;
    std::vector<VkFence> ImagesInFlight;

    static FRender* RenderInstance;

private:
    ECS::FEntity CreateEmptyModel();
    int Width;
    int Height;
};

FRender* GetRender(uint32_t WidthIn = 1920, uint32_t HeightIn = 1080);

#define INIT_RENDER(WidthIn, HeightIn) GetRender(WidthIn, HeightIn)
#define RENDER() GetRender()
