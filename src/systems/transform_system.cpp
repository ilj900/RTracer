#include "components/transform_component.h"
#include "components/device_transform_component.h"
#include "systems/transform_system.h"
#include "coordinator.h"

namespace ECS
{
    namespace SYSTEMS
    {
        void FTransformSystem::Init(int NumberOfSimultaneousSubmits)
        {
            FGPUBufferableSystem::Init(NumberOfSimultaneousSubmits, sizeof(ECS::COMPONENTS::FDeviceTransformComponent) * MAX_ENTITIES,
                                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, "Device_Transform_Buffer");
        }

        void FTransformSystem::Update()
        {
            for (int i = 0; i < NumberOfSimultaneousSubmits; ++i)
            {
                for (auto Entity: EntitiesToUpdate[i])
                {
                    auto DeviceTransformComponent = GetComponent<ECS::COMPONENTS::FDeviceTransformComponent>(Entity);
                    DeviceTransformComponent.ModelMatrix = GetModelMatrix(Entity);
                }
            }

            FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FDeviceTransformComponent>();
        }

        void FTransformSystem::Update(int Index)
        {
            for (auto Entity: EntitiesToUpdate[Index])
            {
                auto DeviceTransformComponent = GetComponent<ECS::COMPONENTS::FDeviceTransformComponent>(Entity);
                DeviceTransformComponent.ModelMatrix = GetModelMatrix(Entity);
            }

            FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FDeviceTransformComponent>(Index);
        }

        void FTransformSystem::MoveForward(FEntity Entity, float Value)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Position += TransformComponent.Direction * Value;
            MarkDirty(Entity);
        }

        void FTransformSystem::MoveRight(FEntity Entity, float Value)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Position += Cross(TransformComponent.Direction, TransformComponent.Up) * Value;
            MarkDirty(Entity);
        }

        void FTransformSystem::SetTransform(FEntity Entity, const FVector3& Position, const FVector3& Direction, const FVector3& Up)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Position = Position;
            TransformComponent.Direction = Direction;
            TransformComponent.Up = Up;
            MarkDirty(Entity);
        }

        void FTransformSystem::SetLookAt(FEntity Entity, const FVector3& PointOfInterest)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Direction = (PointOfInterest - TransformComponent.Position).GetNormalized();
            MarkDirty(Entity);
        }

        void FTransformSystem::MoveUpward(FEntity Entity, float Value)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Position += TransformComponent.Up * Value;
            MarkDirty(Entity);
        }

        void FTransformSystem::Roll(FEntity Entity, float Value)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Up = TransformComponent.Up.Rotate(Value, TransformComponent.Direction);
            MarkDirty(Entity);
        }

        void FTransformSystem::Pitch(FEntity Entity, float Value)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            auto Axis = Cross(TransformComponent.Direction, TransformComponent.Up);
            TransformComponent.Direction = TransformComponent.Direction.Rotate(Value, Axis);
            TransformComponent.Up = TransformComponent.Up.Rotate(Value, Axis);
            MarkDirty(Entity);
        }

        void FTransformSystem::Yaw(FEntity Entity, float Value)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Direction = TransformComponent.Direction.Rotate(Value, TransformComponent.Up);
            MarkDirty(Entity);
        }

        void FTransformSystem::Translate(FEntity Entity, float X, float Y, float Z)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            TransformComponent.Position += {X, Y, Z};
            MarkDirty(Entity);
        }

        FMatrix4 FTransformSystem::GetModelMatrix(FEntity Entity)
        {
            auto& TransformComponent = GetComponent<ECS::COMPONENTS::FTransformComponent>(Entity);
            return Transform(TransformComponent.Position, TransformComponent.Direction, TransformComponent.Up, TransformComponent.Scale);
        }
    }
}