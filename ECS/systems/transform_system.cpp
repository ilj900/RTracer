#include "components/transform_component.h"
#include "components/device_transform_component.h"
#include "systems/transform_system.h"
#include "coordinator.h"

#include <cassert>

namespace ECS
{
    namespace SYSTEMS
    {
        template<typename T>
        inline T& FTransformSystem::GetComponent(FEntity Entity)
        {
            assert(Entities.find(Entity) != Entities.end() && "Entity doesn't have camera component");
            auto& Coordinator = GetCoordinator();
            auto& TransformComponent = Coordinator.GetComponent<T>(Entity);
            return TransformComponent;
        }

        void FTransformSystem::UpdateAllDeviceComponentsData()
        {
            auto& Coordinator = GetCoordinator();

            for (auto Entity : Entities)
            {
                auto& DeviceTransformComponent = Coordinator.GetComponent<COMPONENTS::FDeviceTransformComponent>(Entity);
                auto& TransformComponent = Coordinator.GetComponent<COMPONENTS::FTransformComponent>(Entity);
                DeviceTransformComponent.ModelMatrix = Transform(TransformComponent.Position, TransformComponent.Direction, TransformComponent.Up, TransformComponent.Scale);
            }
        }

        void FTransformSystem::UpdateDeviceComponentData(FEntity Entity)
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
        }

        void FTransformSystem::MoveRight(FEntity Entity, float Value)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Position += TransformComponent.Direction * TransformComponent.Up * Value;
        }

        void FTransformSystem::SetTransform(FEntity Entity, const FVector3& Position, const FVector3& Direction, const FVector3& Up)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Position = Position;
            TransformComponent.Direction = Direction;
            TransformComponent.Up = Up;
        }

        void FTransformSystem::SetLookAt(FEntity Entity, const FVector3& PointOfInterest)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Direction = (PointOfInterest - TransformComponent.Position).GetNormalized();
        }

        void FTransformSystem::MoveUpward(FEntity Entity, float Value)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Position += TransformComponent.Up * Value;
        }

        void FTransformSystem::Roll(FEntity Entity, float Value)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Up = TransformComponent.Up.Rotate(Value, TransformComponent.Direction);
        }

        void FTransformSystem::Pitch(FEntity Entity, float Value)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            auto Axis = TransformComponent.Direction * TransformComponent.Up;
            TransformComponent.Direction = TransformComponent.Direction.Rotate(Value, Axis);
            TransformComponent.Up = TransformComponent.Up.Rotate(Value, Axis);
        }

        void FTransformSystem::Yaw(FEntity Entity, float Value)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Direction = TransformComponent.Direction.Rotate(Value, TransformComponent.Up);
        }

        FMatrix4 FTransformSystem::GetModelMatrix(FEntity Entity)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            return Transform(TransformComponent.Position, TransformComponent.Direction, TransformComponent.Up, TransformComponent.Scale);
        }

    }
}