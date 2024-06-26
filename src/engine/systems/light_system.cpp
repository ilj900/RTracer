#include "light_component.h"
#include "light_system.h"

namespace ECS
{
    namespace SYSTEMS
    {
        void FLightSystem::Init(uint32_t NumberOfSimultaneousSubmits)
        {
            FGPUBufferableSystem::Init(NumberOfSimultaneousSubmits, sizeof(ECS::COMPONENTS::FLightComponent) * MAX_LIGHTS,
                                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, "Device_Light");
        }

        bool FLightSystem::Update()
        {
			bool bAnyUpdate = false;

			for (auto& Entry : EntitiesToUpdate)
			{
				bAnyUpdate |= !Entry.empty();
			}

			if (bAnyUpdate)
			{
				FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FLightComponent>();
			}

			return bAnyUpdate;
        }

        bool FLightSystem::Update(int Index)
        {
			bool bAnyUpdate = false;

			for (auto& Entry : EntitiesToUpdate)
			{
				bAnyUpdate |= !Entry.empty();
			}

			if (bAnyUpdate)
			{
				FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FLightComponent>(Index);
			}

			return bAnyUpdate;
        }

        FLightSystem& FLightSystem::SetLightPosition(FEntity LightEntity, float X, float Y, float Z)
        {
            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FLightComponent>(LightEntity);
            LightComponent.Position = FVector3(X, Y, Z);
            MarkDirty(LightEntity);

            return *this;
        }

        FLightSystem& FLightSystem::SetLightPosition(FEntity LightEntity, const FVector3& Position)
        {
            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FLightComponent>(LightEntity);
            LightComponent.Position = Position;
            MarkDirty(LightEntity);

            return *this;
        }

        FLightSystem& FLightSystem::SetLightDirection(FEntity LightEntity, const FVector3& Direction)
        {
            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FLightComponent>(LightEntity);
            LightComponent.Direction = Direction;
            MarkDirty(LightEntity);

            return *this;
        }

        FLightSystem& FLightSystem::SetLightColor(FEntity LightEntity, const FVector3& Color)
        {
            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FLightComponent>(LightEntity);
            LightComponent.Color = Color;
            MarkDirty(LightEntity);

            return *this;
        }

        FLightSystem& FLightSystem::SetLightIntensity(FEntity LightEntity, float Intensity)
        {
            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FLightComponent>(LightEntity);
            LightComponent.Intensity = Intensity;
            MarkDirty(LightEntity);

            return *this;
        }

        FEntity FLightSystem::CreatePointLight(const FVector3& Position, const FVector3& Color, float Intensity)
        {
            auto Light = COORDINATOR().CreateEntity();
            COORDINATOR().AddComponent<ECS::COMPONENTS::FLightComponent>(Light, {});

            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FLightComponent>(Light);
            LightComponent.Type = LIGHT_TYPE_POINT_LIGHT;
            LightComponent.Position = Position;
            LightComponent.Color = Color;
            LightComponent.Intensity = Intensity;
            MarkDirty(Light);

            return Light;
        }

        FEntity FLightSystem::CreateDirectionalLight(const FVector3& Direction, const FVector3& Color, float Intensity)
        {
            auto Light = COORDINATOR().CreateEntity();
            COORDINATOR().AddComponent<ECS::COMPONENTS::FLightComponent>(Light, {});

            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FLightComponent>(Light);
            LightComponent.Type = LIGHT_TYPE_DIRECTIONAL_LIGHT;
            LightComponent.Direction = Direction;
            LightComponent.Color = Color;
            LightComponent.Intensity = Intensity;
            MarkDirty(Light);

            return Light;
        }

        FEntity FLightSystem::CreateSpotLight(const FVector3& Position, const FVector3& Color, float Intensity, float OuterAngle, float InnerAngle)
        {
            auto Light = COORDINATOR().CreateEntity();
            COORDINATOR().AddComponent<ECS::COMPONENTS::FLightComponent>(Light, {});

            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FLightComponent>(Light);
            LightComponent.Type = LIGHT_TYPE_SPOT_LIGHT;
            LightComponent.Position = Position;
            LightComponent.Color = Color;
            LightComponent.Intensity = Intensity;
            LightComponent.OuterAngle = OuterAngle;
            LightComponent.InnerAngle = InnerAngle;
            MarkDirty(Light);

            return Light;
        }
    }
}
