#include "components/camera_component.h"
#include "components/device_camera_component.h"
#include "systems/camera_system.h"
#include "coordinator.h"

#include "vk_context.h"

#include <cassert>

namespace ECS
{
    namespace SYSTEMS
    {
        void FCameraSystem::Init(int NumberOfSimultaneousSubmits)
        {
            FGPUBufferableSystem::Init(NumberOfSimultaneousSubmits, sizeof(ECS::COMPONENTS::FDeviceCameraComponent) * MAX_CAMERAS,
                                       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, "Device_Camera_Buffer");
        }

        void FCameraSystem::Update()
        {
            FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FDeviceCameraComponent>();
        }

        void FCameraSystem::Update(int Index)
        {
            FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FDeviceCameraComponent>(Index);
        }

        void FCameraSystem::UpdateDeviceComponentData(FEntity CameraEntity)
        {
            auto& Coordinator = GetCoordinator();
            auto& DeviceCameraComponent = Coordinator.GetComponent<COMPONENTS::FDeviceCameraComponent>(CameraEntity);
            auto& CameraComponent = Coordinator.GetComponent<COMPONENTS::FCameraComponent>(CameraEntity);
            DeviceCameraComponent.ViewMatrix = LookAt(CameraComponent.Position, CameraComponent.Position + CameraComponent.Direction, CameraComponent.Up);
            DeviceCameraComponent.ProjectionMatrix = GetPerspective(CameraComponent.FOV / 90.f, CameraComponent.Ratio, CameraComponent.ZNear, CameraComponent.ZFar);
        }

        void FCameraSystem::MoveCameraForward(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            CameraComponent.Position += CameraComponent.Direction * Value;
        }


        void FCameraSystem::MoveCameraRight(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            CameraComponent.Position += Cross(CameraComponent.Direction, CameraComponent.Up) * Value;
        }

        void FCameraSystem::MoveCameraUpward(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            CameraComponent.Position += CameraComponent.Up * Value;
        }

        void FCameraSystem::LookUp(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            auto RotationAxis = Cross(CameraComponent.Direction, CameraComponent.Up);
            CameraComponent.Direction = CameraComponent.Direction.Rotate(Value, RotationAxis);
            CameraComponent.Up = CameraComponent.Up.Rotate(Value, RotationAxis);
        }

        void FCameraSystem::LookRight(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            CameraComponent.Direction = CameraComponent.Direction.Rotate(Value, CameraComponent.Up);
        }

        void FCameraSystem::Roll(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            CameraComponent.Up = CameraComponent.Up.Rotate(Value, CameraComponent.Direction);
        }

        void FCameraSystem::SetAspectRatio(FEntity CameraEntity, float AspectRatio)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            CameraComponent.Ratio = AspectRatio;
        }

        FMatrix4 FCameraSystem::GetProjectionMatrix(FEntity CameraEntity)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            return GetPerspective(CameraComponent.FOV / 90.f, CameraComponent.Ratio, CameraComponent.ZNear, CameraComponent.ZFar);
        }

        FMatrix4 FCameraSystem::GetViewMatrix(FEntity CameraEntity)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            return LookAt(CameraComponent.Position, CameraComponent.Position + CameraComponent.Direction, CameraComponent.Up);
        }

        void FCameraSystem::Orthogonize(FEntity CameraEntity)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            CameraComponent.Direction = CameraComponent.Direction.GetNormalized();
            CameraComponent.Up = CameraComponent.Up.GetNormalized();
            auto Right = Cross(CameraComponent.Direction, CameraComponent.Up);
            CameraComponent.Up = Cross(Right, CameraComponent.Direction);
        }
    }
}