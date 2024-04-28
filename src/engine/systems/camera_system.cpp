#include "camera_component.h"
#include "device_camera_component.h"
#include "camera_system.h"

namespace ECS
{
    namespace SYSTEMS
    {
        void FCameraSystem::Init(uint32_t NumberOfSimultaneousSubmits)
        {
            FGPUBufferableSystem::Init(NumberOfSimultaneousSubmits, sizeof(ECS::COMPONENTS::FDeviceCameraComponent) * MAX_CAMERAS,
                                       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, "Device_Camera");
        }

        bool FCameraSystem::Update()
        {
			bool bAnyUpdate = false;

            for (auto& Entry : EntitiesToUpdate)
            {
				bAnyUpdate |= !Entry.empty();

                for (auto Entity : Entry)
                {
                    auto& DeviceCameraComponent = COORDINATOR().GetComponent<COMPONENTS::FDeviceCameraComponent>(Entity);
                    auto& CameraComponent = COORDINATOR().GetComponent<COMPONENTS::FCameraComponent>(Entity);
                    DeviceCameraComponent.ViewMatrix = LookAt(CameraComponent.Position, CameraComponent.Position + CameraComponent.Direction, CameraComponent.Up);
					DeviceCameraComponent.InverseViewMatrix = DeviceCameraComponent.ViewMatrix.GetInverse();
                    DeviceCameraComponent.ProjectionMatrix = GetPerspective(CameraComponent.FOV / 90.f, CameraComponent.Ratio, CameraComponent.ZNear, CameraComponent.ZFar);
					DeviceCameraComponent.InverseProjectionMatrix = DeviceCameraComponent.ProjectionMatrix.GetInverse();
                    DeviceCameraComponent.FOV = CameraComponent.FOV;
                    DeviceCameraComponent.Origin = CameraComponent.Position;
                    DeviceCameraComponent.ViewDirection = CameraComponent.Direction;
                    DeviceCameraComponent.ZFar = CameraComponent.ZFar;
                    DeviceCameraComponent.ZNear = CameraComponent.ZNear;
                }
            }

			if (bAnyUpdate)
			{
				FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FDeviceCameraComponent>();
			}

			return bAnyUpdate;
        }

        bool FCameraSystem::Update(int Index)
        {
			bool bAnyUpdate = !EntitiesToUpdate.empty();

            for (auto& Entry : EntitiesToUpdate)
            {
				bAnyUpdate |= !Entry.empty();

                for (auto Entity : Entry)
                {
                    auto& DeviceCameraComponent = COORDINATOR().GetComponent<COMPONENTS::FDeviceCameraComponent>(Entity);
                    auto& CameraComponent = COORDINATOR().GetComponent<COMPONENTS::FCameraComponent>(Entity);
                    DeviceCameraComponent.ViewMatrix = LookAt(CameraComponent.Position, CameraComponent.Position + CameraComponent.Direction, CameraComponent.Up);
					DeviceCameraComponent.InverseViewMatrix = DeviceCameraComponent.ViewMatrix.GetInverse();
                    DeviceCameraComponent.ProjectionMatrix = GetPerspective(CameraComponent.FOV / 90.f, CameraComponent.Ratio, CameraComponent.ZNear, CameraComponent.ZFar);
					DeviceCameraComponent.InverseProjectionMatrix = DeviceCameraComponent.ProjectionMatrix.GetInverse();
                    DeviceCameraComponent.FOV = CameraComponent.FOV;
                    DeviceCameraComponent.Origin = CameraComponent.Position;
                    DeviceCameraComponent.ViewDirection = CameraComponent.Direction;
                    DeviceCameraComponent.ZFar = CameraComponent.ZFar;
                    DeviceCameraComponent.ZNear = CameraComponent.ZNear;
                }
            }

			if (bAnyUpdate)
			{
				FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FDeviceCameraComponent>(Index);
			}

			return bAnyUpdate;
        }

        void FCameraSystem::MoveCameraForward(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            CameraComponent.Position += CameraComponent.Direction * Value;
            MarkDirty(CameraEntity);
        }


        void FCameraSystem::MoveCameraRight(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            CameraComponent.Position += Cross(CameraComponent.Direction, CameraComponent.Up) * Value;
            MarkDirty(CameraEntity);
        }

        void FCameraSystem::MoveCameraUpward(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            CameraComponent.Position += CameraComponent.Up * Value;
            MarkDirty(CameraEntity);
        }

        void FCameraSystem::LookUp(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            auto RotationAxis = Cross(CameraComponent.Direction, CameraComponent.Up);
            CameraComponent.Direction = CameraComponent.Direction.Rotate(Value, RotationAxis);
            CameraComponent.Up = CameraComponent.Up.Rotate(Value, RotationAxis);
            MarkDirty(CameraEntity);
        }

        void FCameraSystem::LookRight(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            CameraComponent.Direction = CameraComponent.Direction.Rotate(Value, CameraComponent.Up);
            MarkDirty(CameraEntity);
        }

        void FCameraSystem::Roll(FEntity CameraEntity, float Value)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            CameraComponent.Up = CameraComponent.Up.Rotate(Value, CameraComponent.Direction);
            MarkDirty(CameraEntity);
        }

        void FCameraSystem::SetAspectRatio(FEntity CameraEntity, float AspectRatio)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            CameraComponent.Ratio = AspectRatio;
            MarkDirty(CameraEntity);
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

        void FCameraSystem::Orthogonalize(FEntity CameraEntity)
        {
            auto& CameraComponent = GetComponent<ECS::COMPONENTS::FCameraComponent>(CameraEntity);
            CameraComponent.Direction = CameraComponent.Direction.GetNormalized();
            CameraComponent.Up = CameraComponent.Up.GetNormalized();
            auto Right = Cross(CameraComponent.Direction, CameraComponent.Up);
            CameraComponent.Up = Cross(Right, CameraComponent.Direction);
            MarkDirty(CameraEntity);
        }
    }
}