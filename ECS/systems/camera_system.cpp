#include "components/camera_component.h"
#include "systems/camera_system.h"
#include "coordinator.h"

#include <cassert>

namespace ECS
{
    namespace SYSTEMS
    {
        inline ECS::COMPONENTS::FCameraComponent& FCameraSystem::GetComponent(FEntity CameraEntity)
        {
            assert(Entities.find(CameraEntity) != Entities.end() && "Entity doesn't have camera component");
            auto& Coordinator = GetCoordinator();
            auto& CameraComponent = Coordinator.GetComponent<COMPONENTS::FCameraComponent>(CameraEntity);
            return CameraComponent;
        }

        void FCameraSystem::MoveCameraForward(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent(CameraEntity);
            CameraComponent.Position += CameraComponent.Direction * Value;
        }


        void FCameraSystem::MoveCameraRight(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent(CameraEntity);
            CameraComponent.Position += CameraComponent.Direction * CameraComponent.Up * Value;
        }

        void FCameraSystem::MoveCameraUpward(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent(CameraEntity);
            CameraComponent.Position += CameraComponent.Up * Value;
        }

        void FCameraSystem::LookUp(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent(CameraEntity);
            auto RotationAxis = CameraComponent.Direction * CameraComponent.Up;
            CameraComponent.Direction = CameraComponent.Direction.Rotate(Value, RotationAxis);
            CameraComponent.Up = CameraComponent.Up.Rotate(Value, RotationAxis);
        }

        void FCameraSystem::LookRight(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent(CameraEntity);
            CameraComponent.Direction = CameraComponent.Direction.Rotate(Value, CameraComponent.Up);
        }

        void FCameraSystem::Roll(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent(CameraEntity);
            CameraComponent.Up = CameraComponent.Up.Rotate(Value, CameraComponent.Direction);
        }

        FMatrix4 FCameraSystem::GetProjectionMatrix(FEntity CameraEntity)
        {
            auto& CameraComponent = GetComponent(CameraEntity);
            return GetPerspective(CameraComponent.FOV / 90.f, CameraComponent.Ratio, CameraComponent.ZNear, CameraComponent.ZFar);
        }

        FMatrix4 FCameraSystem::GetViewMatrix(FEntity CameraEntity)
        {
            auto& CameraComponent = GetComponent(CameraEntity);
            return LookAt(CameraComponent.Position, CameraComponent.Position + CameraComponent.Direction, CameraComponent.Up);
        }

        void FCameraSystem::Orthogonize(FEntity CameraEntity)
        {
            auto& CameraComponent = GetComponent(CameraEntity);
            CameraComponent.Direction = CameraComponent.Direction.GetNormalized();
            CameraComponent.Up = CameraComponent.Up.GetNormalized();
            auto Right = CameraComponent.Direction * CameraComponent.Up;
            CameraComponent.Up = Right * CameraComponent.Direction;
        }
    }
}