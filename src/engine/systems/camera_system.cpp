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
            }

			if (bAnyUpdate)
			{
				FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FDeviceCameraComponent>(Index);
			}

			return bAnyUpdate;
        }

		void FCameraSystem::SetPosition(FEntity CameraEntity, const FVector3& Position, const std::optional<FVector3>& Direction, const std::optional<FVector3>& Up)
		{
			auto& DeviceCameraComponent = GetComponent<ECS::COMPONENTS::FDeviceCameraComponent>(CameraEntity);
			DeviceCameraComponent.Origin = Position;

			if (Direction)
			{
				DeviceCameraComponent.Direction = Direction.value().GetNormalized();
			}

			if (Up)
			{
				DeviceCameraComponent.Up = Up.value().GetNormalized();
			}

			DeviceCameraComponent.Right = Cross(DeviceCameraComponent.Direction, DeviceCameraComponent.Up);

			MarkDirty(CameraEntity);
		}

		void FCameraSystem::SetCameraSensorProperties(FEntity Camera, const std::optional<float>& SensorSizeX, const std::optional<float>& SensorSizeY, const std::optional<float>& FocalDistance)
		{
			auto& DeviceCameraComponent = GetComponent<ECS::COMPONENTS::FDeviceCameraComponent>(Camera);

			if (SensorSizeX)
			{
				DeviceCameraComponent.SensorSizeX = SensorSizeX.value();
			}

			if (SensorSizeY)
			{
				DeviceCameraComponent.SensorSizeY = SensorSizeY.value();
			}

			if (FocalDistance)
			{
				DeviceCameraComponent.FocalDistance = FocalDistance.value();
			}

			MarkDirty(Camera);
		}

        void FCameraSystem::MoveCameraForward(FEntity CameraEntity, float Value)
        {
            auto& DeviceCameraComponent = GetComponent<ECS::COMPONENTS::FDeviceCameraComponent>(CameraEntity);
			DeviceCameraComponent.Origin += DeviceCameraComponent.Direction * Value;
            MarkDirty(CameraEntity);
        }


        void FCameraSystem::MoveCameraRight(FEntity CameraEntity, float Value)
        {
            auto& DeviceCameraComponent = GetComponent<ECS::COMPONENTS::FDeviceCameraComponent>(CameraEntity);
			DeviceCameraComponent.Origin += DeviceCameraComponent.Right * Value;
            MarkDirty(CameraEntity);
        }

        void FCameraSystem::MoveCameraUpward(FEntity CameraEntity, float Value)
        {
            auto& DeviceCameraComponent = GetComponent<ECS::COMPONENTS::FDeviceCameraComponent>(CameraEntity);
			DeviceCameraComponent.Origin += DeviceCameraComponent.Up * Value;
            MarkDirty(CameraEntity);
        }

        void FCameraSystem::LookUp(FEntity CameraEntity, float Value)
        {
            auto& DeviceCameraComponent = GetComponent<ECS::COMPONENTS::FDeviceCameraComponent>(CameraEntity);
			DeviceCameraComponent.Direction = DeviceCameraComponent.Direction.Rotate(-Value, DeviceCameraComponent.Right);
			DeviceCameraComponent.Up = DeviceCameraComponent.Up.Rotate(-Value, DeviceCameraComponent.Right);
            MarkDirty(CameraEntity);
        }

        void FCameraSystem::LookRight(FEntity CameraEntity, float Value)
        {
            auto& DeviceCameraComponent = GetComponent<ECS::COMPONENTS::FDeviceCameraComponent>(CameraEntity);
			DeviceCameraComponent.Direction = DeviceCameraComponent.Direction.Rotate(-Value, DeviceCameraComponent.Up);
			DeviceCameraComponent.Right = DeviceCameraComponent.Right.Rotate(-Value, DeviceCameraComponent.Up);
            MarkDirty(CameraEntity);
        }

        void FCameraSystem::Roll(FEntity CameraEntity, float Value)
        {
            auto& DeviceCameraComponent = GetComponent<ECS::COMPONENTS::FDeviceCameraComponent>(CameraEntity);
			DeviceCameraComponent.Up = DeviceCameraComponent.Up.Rotate(Value, DeviceCameraComponent.Direction);
			DeviceCameraComponent.Right = DeviceCameraComponent.Right.Rotate(Value, DeviceCameraComponent.Direction);
            MarkDirty(CameraEntity);
        }
    }
}