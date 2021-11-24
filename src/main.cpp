#include "coordinator.h"

#include "components/camera_component.h"
#include "components/device_camera_component.h"
#include "systems/camera_system.h"

#include "components/transform_component.h"
#include "components/device_transform_component.h"
#include "systems/transform_system.h"

#include "components/device_renderable_component.h"
#include "systems/renderable_system.h"

#include "components/mesh_component.h"
#include "components/device_mesh_component.h"
#include "systems/mesh_system.h"

#include "GLFW/glfw3.h"
#include "context.h"

#include <chrono>
#include <string>

const uint32_t WINDOW_WIDTH = 1920;
const uint32_t WINDOW_HEIGHT = 1080;
const std::string WINDOW_NAME = "RTracer";

std::vector<ECS::FEntity> Models;

void InitECS()
{
    auto& Coordinator = ECS::GetCoordinator();
    Coordinator.Init();

    /// Register Camera сomponents and system
    Coordinator.RegisterComponent<ECS::COMPONENTS::FCameraComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FDeviceCameraComponent>();
    auto CameraSystem = Coordinator.RegisterSystem<ECS::SYSTEMS::FCameraSystem>();

    ECS::FSignature CameraSystemSignature;
    CameraSystemSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FCameraComponent>());
    CameraSystemSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FDeviceCameraComponent>());
    Coordinator.SetSystemSignature<ECS::SYSTEMS::FCameraSystem>(CameraSystemSignature);

    /// Register Transform components and system
    Coordinator.RegisterComponent<ECS::COMPONENTS::FTransformComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FDeviceTransformComponent>();
    auto TransformSystem = Coordinator.RegisterSystem<ECS::SYSTEMS::FTransformSystem>();

    ECS::FSignature TransformSystemSignature;
    TransformSystemSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FTransformComponent>());
    TransformSystemSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FDeviceTransformComponent>());
    Coordinator.SetSystemSignature<ECS::SYSTEMS::FTransformSystem>(TransformSystemSignature);

    /// Register Renderable components and system
    Coordinator.RegisterComponent<ECS::COMPONENTS::FDeviceRenderableComponent>();
    auto RenderableSystem = Coordinator.RegisterSystem<ECS::SYSTEMS::FRenderableSystem>();

    ECS::FSignature RenderablesSignature;
    RenderablesSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FDeviceRenderableComponent>());
    Coordinator.SetSystemSignature<ECS::SYSTEMS::FRenderableSystem>(RenderablesSignature);

    /// Register Mesh component and Systems
    Coordinator.RegisterComponent<ECS::COMPONENTS::FMeshComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FDeviceMeshComponent>();
    auto MeshSystem = Coordinator.RegisterSystem<ECS::SYSTEMS::FMeshSystem>();

    ECS::FSignature MeshSignature;
    MeshSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FMeshComponent>());
    MeshSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FDeviceMeshComponent>());
    Coordinator.SetSystemSignature<ECS::SYSTEMS::FMeshSystem>(MeshSignature);
}

void LoadModels()
{
    auto& Coordinator = ECS::GetCoordinator();
    auto MeshSystem = Coordinator.GetSystem<ECS::SYSTEMS::FMeshSystem>();
    auto TransformSystem = Coordinator.GetSystem<ECS::SYSTEMS::FTransformSystem>();

    enum MeshType {Tetrahedron, Hexahedron, Icosahedron, Model};

    auto AddMesh = [&Coordinator, &MeshSystem, &TransformSystem](const FVector3& Color, const FVector3& Position, MeshType Type, const std::string& Path, uint32_t RenderableMask){
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
            MeshSystem->CreateIcosahedron(Models.back(), 6);
            break;
        case Model:
            MeshSystem->LoadMesh(Models.back(), Path);
            break;
        }
        Coordinator.GetSystem<ECS::SYSTEMS::FTransformSystem>()->SetTransform(Models.back(), Position, {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f});
        Coordinator.GetSystem<ECS::SYSTEMS::FRenderableSystem>()->SetRenderableColor(Models.back(), Color.X, Color.Y, Color.Z);
        TransformSystem->UpdateDeviceComponentData(Models.back());
    };

//    AddMesh({0.9f, 0.6f, 0.3f}, {-2.f, 0.f, -2.f}, Tetrahedron, std::string(), 0);
//    AddMesh({0.6f, 0.3f, 0.9f}, {0.f, 0.f, -2.f}, Hexahedron, std::string(), ECS::COMPONENTS::RENDERABLE_SELECTED_BIT);
//    AddMesh({0.3f, 0.9f, 0.6f}, {2.f, 0.f, -2.f}, Icosahedron, std::string(), 0);
    AddMesh({0.3f, 0.9f, 0.6f}, {2.f, 0.f, -2.f}, Model, "../models/viking_room/viking_room.obj", 0);
}

int main()
{
    system("powershell.exe F:\\Work\\Projects\\_C++_Home_Projects\\RTracer\\shaders\\compile.ps1");
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    GLFWwindow* Window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME.c_str(), nullptr, nullptr);
    glfwSetWindowPos(Window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    glfwSetCursorPos(Window, 0.f, 0.f);

    InitECS();

    FController Controller(Window);

    LoadModels();

    auto& Context = GetContext();
    Context.Init(Window, &Controller);

    while (!glfwWindowShouldClose(Window))
    {
        static auto StartTime = std::chrono::high_resolution_clock::now();

        auto CurrentTime = std::chrono::high_resolution_clock::now();
        float Time = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - StartTime).count();
        StartTime = CurrentTime;

        Controller.Update(Time);
        Context.Render();
        Context.RenderImGui();
        Context.Present();
        glfwPollEvents();
    }
    Context.WaitIdle();
    Context.CleanUp();

    glfwDestroyWindow(Window);
    glfwTerminate();
    return 0;
}