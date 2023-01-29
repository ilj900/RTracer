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
        template<typename T>
        inline T& FCameraSystem::GetComponent(FEntity CameraEntity)
        {
            assert(Entities.find(CameraEntity) != Entities.end() && "Entity doesn't have camera component");
            auto& Coordinator = GetCoordinator();
            auto& CameraComponent = Coordinator.GetComponent<T>(CameraEntity);
            return CameraComponent;
        }

        void FCameraSystem::Init(int NumberOfSimultaneousSubmits)
        {
            this->NumberOfSimultaneousSubmits = NumberOfSimultaneousSubmits;
            auto& Coordinator = GetCoordinator();
            auto& Context = GetContext();
            auto DeviceCameraComponentsData = Coordinator.Data<ECS::COMPONENTS::FDeviceCameraComponent>();
            auto DeviceCameraComponentsSize = Coordinator.Size<ECS::COMPONENTS::FDeviceCameraComponent>();

            VkDeviceSize CameraBufferSize = Coordinator.Size<ECS::COMPONENTS::FDeviceCameraComponent>() * NumberOfSimultaneousSubmits;

            DeviceCameraBuffer = GetContext().CreateBuffer(CameraBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "Device_Camera_Buffer");

            for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
            {
                Context.ResourceAllocator->LoadDataToBuffer(DeviceCameraBuffer, DeviceCameraComponentsSize, DeviceCameraComponentsSize * i, DeviceCameraComponentsData);
            }
        }

        void FCameraSystem::Update(int IterationIndex)
        {
            if (bNeedsUpdate)
            {
                auto& Coordinator = GetCoordinator();
                auto& Context = GetContext();
                auto DeviceCameraComponentsData = Coordinator.Data<ECS::COMPONENTS::FDeviceCameraComponent>();
                auto DeviceCameraComponentsSize = Coordinator.Size<ECS::COMPONENTS::FDeviceCameraComponent>();

                Context.ResourceAllocator->LoadDataToBuffer(DeviceCameraBuffer, DeviceCameraComponentsSize, DeviceCameraComponentsSize * IterationIndex, DeviceCameraComponentsData);
            }

            bNeedsUpdate = false;
        }

        void FCameraSystem::UpdateAllDeviceComponentsData()
        {
            auto& Coordinator = GetCoordinator();

            for (auto Entity : Entities)
            {
                auto& DeviceCameraComponent = Coordinator.GetComponent<COMPONENTS::FDeviceCameraComponent>(Entity);
                auto& CameraComponent = Coordinator.GetComponent<COMPONENTS::FCameraComponent>(Entity);
                DeviceCameraComponent.ViewMatrix = LookAt(CameraComponent.Position, CameraComponent.Position + CameraComponent.Direction, CameraComponent.Up);
                DeviceCameraComponent.ProjectionMatrix = GetPerspective(CameraComponent.FOV / 90.f, CameraComponent.Ratio, CameraComponent.ZNear, CameraComponent.ZFar);
            }

            bNeedsUpdate = true;
        }

        void FCameraSystem::UpdateDeviceComponentData(FEntity CameraEntity)
        {
            auto& Coordinator = GetCoordinator();
            auto& DeviceCameraComponent = Coordinator.GetComponent<COMPONENTS::FDeviceCameraComponent>(CameraEntity);
            auto& CameraComponent = Coordinator.GetComponent<COMPONENTS::FCameraComponent>(CameraEntity);
            DeviceCameraComponent.ViewMatrix = LookAt(CameraComponent.Position, CameraComponent.Position + CameraComponent.Direction, CameraComponent.Up);
            DeviceCameraComponent.ProjectionMatrix = GetPerspective(CameraComponent.FOV / 90.f, CameraComponent.Ratio, CameraComponent.ZNear, CameraComponent.ZFar);
            bNeedsUpdate = true;
        }

        void FCameraSystem::MoveCameraForward(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            CameraComponent.Position += CameraComponent.Direction * Value;
        }


        void FCameraSystem::MoveCameraRight(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            CameraComponent.Position += CameraComponent.Direction * CameraComponent.Up * Value;
        }

        void FCameraSystem::MoveCameraUpward(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            CameraComponent.Position += CameraComponent.Up * Value;
        }

        void FCameraSystem::LookUp(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            auto RotationAxis = CameraComponent.Direction * CameraComponent.Up;
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
            auto Right = CameraComponent.Direction * CameraComponent.Up;
            CameraComponent.Up = Right * CameraComponent.Direction;
        }
    }
}