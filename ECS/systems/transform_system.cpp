#include "components/transform_component.h"
#include "components/device_transform_component.h"
#include "systems/transform_system.h"
#include "coordinator.h"

namespace ECS
{
    namespace SYSTEMS
    {
        void FTransformSystem::Update()
        {
            auto& Coordinator = GetCoordinator();

            for (auto Entity : EntitiesToUpdate)
            {
                auto& DeviceTransformComponent = Coordinator.GetComponent<COMPONENTS::FDeviceTransformComponent>(Entity);
                auto& TransformComponent = Coordinator.GetComponent<COMPONENTS::FTransformComponent>(Entity);
                DeviceTransformComponent.ModelMatrix = Transform(TransformComponent.Position, TransformComponent.Direction, TransformComponent.Up, TransformComponent.Scale);
            }
        }

        void FTransformSystem::Update(FEntity Entity)
        {
            auto& Coordinator = GetCoordinator();
            auto& DeviceTransformComponent = Coordinator.GetComponent<COMPONENTS::FDeviceTransformComponent>(Entity);
            auto& TransformComponent = Coordinator.GetComponent<COMPONENTS::FTransformComponent>(Entity);
            DeviceTransformComponent.ModelMatrix = Transform(TransformComponent.Position, TransformComponent.Direction, TransformComponent.Up, TransformComponent.Scale);
        }

        void FTransformSystem::MoveForward(FEntity Entity, float Value)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Position += TransformComponent.Direction * Value;
            EntitiesToUpdate.insert(Entity);
        }

        void FTransformSystem::MoveRight(FEntity Entity, float Value)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Position += Cross(TransformComponent.Direction, TransformComponent.Up) * Value;
            EntitiesToUpdate.insert(Entity);
        }

        void FTransformSystem::SetTransform(FEntity Entity, const FVector3& Position, const FVector3& Direction, const FVector3& Up)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Position = Position;
            TransformComponent.Direction = Direction;
            TransformComponent.Up = Up;
            EntitiesToUpdate.insert(Entity);
        }

        void FTransformSystem::SetLookAt(FEntity Entity, const FVector3& PointOfInterest)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Direction = (PointOfInterest - TransformComponent.Position).GetNormalized();
            EntitiesToUpdate.insert(Entity);
        }

        void FTransformSystem::MoveUpward(FEntity Entity, float Value)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Position += TransformComponent.Up * Value;
            EntitiesToUpdate.insert(Entity);
        }

        void FTransformSystem::Roll(FEntity Entity, float Value)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Up = TransformComponent.Up.Rotate(Value, TransformComponent.Direction);
            EntitiesToUpdate.insert(Entity);
        }

        void FTransformSystem::Pitch(FEntity Entity, float Value)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            auto Axis = Cross(TransformComponent.Direction, TransformComponent.Up);
            TransformComponent.Direction = TransformComponent.Direction.Rotate(Value, Axis);
            TransformComponent.Up = TransformComponent.Up.Rotate(Value, Axis);
            EntitiesToUpdate.insert(Entity);
        }

        void FTransformSystem::Yaw(FEntity Entity, float Value)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Direction = TransformComponent.Direction.Rotate(Value, TransformComponent.Up);
            EntitiesToUpdate.insert(Entity);
        }

        void FTransformSystem::Translate(FEntity Entity, float X, float Y, float Z)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Position += {X, Y, Z};
            EntitiesToUpdate.insert(Entity);
        }

        FMatrix4 FTransformSystem::GetModelMatrix(FEntity Entity)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            return Transform(TransformComponent.Position, TransformComponent.Direction, TransformComponent.Up, TransformComponent.Scale);
        }
    }
}