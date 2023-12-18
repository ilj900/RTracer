#pragma once

#include <chrono>

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

#include "components/material_component.h"
#include "systems/material_system.h"

#include "components/light_component.h"
#include "systems/light_system.h"

#include "application.h"
#include "render.h"

FApplication::FApplication()
{
    auto& Coordinator = ECS::GetCoordinator();
    Coordinator.Init();

    /// Register components
    Coordinator.RegisterComponent<ECS::COMPONENTS::FCameraComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FDeviceCameraComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FTransformComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FDeviceTransformComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FDeviceRenderableComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FMaterialComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FDeviceMeshComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FMeshComponent>();
    Coordinator.RegisterComponent<ECS::COMPONENTS::FLightComponent>();

    /// Register systems
    auto CameraSystem = Coordinator.RegisterSystem<ECS::SYSTEMS::FCameraSystem>();
    auto TransformSystem = Coordinator.RegisterSystem<ECS::SYSTEMS::FTransformSystem>();
    auto RenderableSystem = Coordinator.RegisterSystem<ECS::SYSTEMS::FRenderableSystem>();
    auto MaterialSystem = Coordinator.RegisterSystem<ECS::SYSTEMS::FMaterialSystem>();
    auto MeshSystem = Coordinator.RegisterSystem<ECS::SYSTEMS::FMeshSystem>();
    auto LightSystem = Coordinator.RegisterSystem<ECS::SYSTEMS::FLightSystem>();

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
    Coordinator.SetSystemSignature<ECS::SYSTEMS::FMeshSystem>(MeshSignature);

    /// Register Light system signature
    ECS::FSignature LightSignature;
    LightSignature.set(Coordinator.GetComponentType<ECS::COMPONENTS::FLightComponent>());
    Coordinator.SetSystemSignature<ECS::SYSTEMS::FLightSystem>(LightSignature);

    Render = std::make_shared<FRender>();
    Controller = std::make_shared<FController>();

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
        i += Render->Render();
    }

    return 0;
}