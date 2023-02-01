#pragma once

#include "GLFW/glfw3.h"
#include "vk_context.h"

#include <chrono>
#include <string>

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

#include "application.h"
#include "render.h"

FApplication::FApplication()
{
    auto& Coordinator = ECS::GetCoordinator();
    Coordinator.Init();

    /// Register Camera components and system
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

    system("powershell.exe ..\\shaders\\compile.ps1");

    Controller = std::make_shared<FController>();
    Render = std::make_shared<FRender>();

    Controller->SetWindow(Render->Window);
    Controller->SetRender(Render);
    Controller->UpdateCallbacks();
}

FApplication::~FApplication()
{
    Controller = nullptr;
    Render = nullptr;
}

int FApplication::Run()
{
    int i = 0;
    while (0 == i) {
        static auto StartTime = std::chrono::high_resolution_clock::now();

        auto CurrentTime = std::chrono::high_resolution_clock::now();
        float Time = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - StartTime).count();
        StartTime = CurrentTime;

        Controller->Update(Time);
        i = Render->Update();
        i = Render->Render();
    }

    return 0;
}