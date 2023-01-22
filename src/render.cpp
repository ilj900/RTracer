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

    auto& Context = GetContext();

    LoadModels("");

    Context.Init(Window, WINDOW_WIDTH, WINDOW_HEIGHT);
    Context.CreateImguiContext(Window);

}

FRender::~FRender()
{
    GetContext().WaitIdle();
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
    Context.Render();
    Context.RenderImGui();
    Context.Present();
    glfwPollEvents();

    return 0;
}

int FRender::LoadModels(const std::string& Path)
{
    const uint32_t RENDERABLE_HAS_TEXTURE = 1 << 6;

    std::vector<ECS::FEntity> Models;

    auto& Coordinator = ECS::GetCoordinator();
    auto MeshSystem = Coordinator.GetSystem<ECS::SYSTEMS::FMeshSystem>();
    auto TransformSystem = Coordinator.GetSystem<ECS::SYSTEMS::FTransformSystem>();

    enum MeshType {Tetrahedron, Hexahedron, Icosahedron, Model};

    auto AddMesh = [&Coordinator, &MeshSystem, &TransformSystem, &Models](const FVector3& Color, const FVector3& Position, MeshType Type, const std::string& Path, uint32_t RenderableMask){
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
    };

    AddMesh({0.6f, 0.0f, 0.9f}, {3.f, 0.f, -2.f}, Icosahedron, std::string(), 0);
    AddMesh({0.9f, 0.6f, 0.0f}, {-3.f, 0.f, -2.f}, Tetrahedron, std::string(), 0);
    AddMesh({0.0f, 0.9f, 0.6f}, {1.f, 0.f, -2.f}, Hexahedron, std::string(), 0);

    AddMesh({0.3f, 0.9f, 0.6f}, {-1.f, 0.f, -2.f}, Model, "../models/viking_room/viking_room.obj", RENDERABLE_HAS_TEXTURE);

    return 0;
}

int FRender::AddMesh()
{
    return 0;
}
