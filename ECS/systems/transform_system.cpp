#include "components/transform_component.h"
#include "components/device_transform_component.h"
#include "systems/transform_system.h"
#include "coordinator.h"

#include "vk_context.h"

#include <cassert>

namespace ECS
{
    namespace SYSTEMS
    {
        void FTransformSystem::Init(int NumberOfSimultaneousSubmits)
        {
            this->NumberOfSimultaneousSubmits = NumberOfSimultaneousSubmits;
            auto& Coordinator = GetCoordinator();
            auto& Context = GetContext();
            auto DeviceTransformComponentsData = Coordinator.Data<ECS::COMPONENTS::FDeviceTransformComponent>();
            auto DeviceTransformComponentsSize = Coordinator.Size<ECS::COMPONENTS::FDeviceTransformComponent>();

            VkDeviceSize TransformBufferSize = DeviceTransformComponentsSize * NumberOfSimultaneousSubmits;

            DeviceTransformBuffer = GetContext().CreateBuffer(TransformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "Device_Transform_Buffer");

            for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
            {
                Context.ResourceAllocator->LoadDataToBuffer(DeviceTransformBuffer, DeviceTransformComponentsSize, DeviceTransformComponentsSize * i, DeviceTransformComponentsData);
            }

            BufferPartThatNeedsUpdate.resize(NumberOfSimultaneousSubmits);
        }

        void FTransformSystem::Update()
        {
            for (int i = 0; i < BufferPartThatNeedsUpdate.size(); ++i)
            {
                if (true == BufferPartThatNeedsUpdate[i])
                {
                    auto& Coordinator = GetCoordinator();
                    auto& Context = GetContext();
                    auto DeviceTransformComponentsData = Coordinator.Data<ECS::COMPONENTS::FDeviceTransformComponent>();
                    auto DeviceTransformComponentsSize = Coordinator.Size<ECS::COMPONENTS::FDeviceTransformComponent>();

                    Context.ResourceAllocator->LoadDataToBuffer(DeviceTransformBuffer,
                                                                DeviceTransformComponentsSize,
                                                                DeviceTransformComponentsSize * i,
                                                                DeviceTransformComponentsData);
                    BufferPartThatNeedsUpdate[i] = false;
                }
            }
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
            TransformComponent.Position += Cross(TransformComponent.Direction, TransformComponent.Up) * Value;
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
            auto Axis = Cross(TransformComponent.Direction, TransformComponent.Up);
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

        void FTransformSystem::RequestAllUpdate()
        {
            for(int i = 0; i < NumberOfSimultaneousSubmits; ++i)
            {
                BufferPartThatNeedsUpdate[i] = true;
            }
        }

        void FTransformSystem::RequestUpdate(int FrameIndex)
        {
            BufferPartThatNeedsUpdate[FrameIndex] = true;
        }

    }
}