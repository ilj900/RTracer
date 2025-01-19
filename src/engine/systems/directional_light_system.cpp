#include "light_component.h"
#include "directional_light_system.h"

namespace ECS
{
    namespace SYSTEMS
    {
        void FDirectionalLightSystem::Init(uint32_t NumberOfSimultaneousSubmits)
        {
            FGPUBufferableSystem::Init(NumberOfSimultaneousSubmits, sizeof(ECS::COMPONENTS::FDirectionalLightComponent) * MAX_DIRECTIONAL_LIGHTS,
                                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, "Device_Directional_Lights");
        }

        bool FDirectionalLightSystem::Update()
        {
			bool bAnyUpdate = false;

			for (auto& Entry : EntitiesToUpdate)
			{
				bAnyUpdate |= !Entry.empty();
			}

			if (bAnyUpdate)
			{
				FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FDirectionalLightComponent>();
			}

			return bAnyUpdate;
        }

        bool FDirectionalLightSystem::Update(int Index)
        {
			bool bAnyUpdate = false;

			for (auto& Entry : EntitiesToUpdate)
			{
				bAnyUpdate |= !Entry.empty();
			}

			if (bAnyUpdate)
			{
				FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FDirectionalLightComponent>(Index);
			}

			return bAnyUpdate;
        }

		FDirectionalLightSystem& FDirectionalLightSystem::SetLightPosition(FEntity LightEntity, float X, float Y, float Z)
        {
            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FDirectionalLightComponent>(LightEntity);
            LightComponent.Position = FVector3(X, Y, Z);
            MarkDirty(LightEntity);

            return *this;
        }

		FDirectionalLightSystem& FDirectionalLightSystem::SetLightPosition(FEntity LightEntity, const FVector3& Position)
        {
            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FDirectionalLightComponent>(LightEntity);
            LightComponent.Position = Position;
            MarkDirty(LightEntity);

            return *this;
        }

		FDirectionalLightSystem& FDirectionalLightSystem::SetLightDirection(FEntity LightEntity, const FVector3& Direction)
        {
            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FDirectionalLightComponent>(LightEntity);
            LightComponent.Direction = Direction;
            MarkDirty(LightEntity);

            return *this;
        }

		FDirectionalLightSystem& FDirectionalLightSystem::SetLightDirection(FEntity LightEntity, float X, float Y, float Z)
		{
			auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FDirectionalLightComponent>(LightEntity);
			LightComponent.Direction = {X, Y, Z};
			MarkDirty(LightEntity);

			return *this;
		}

		FDirectionalLightSystem& FDirectionalLightSystem::SetLightColor(FEntity LightEntity, const FVector3& Color)
        {
            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FDirectionalLightComponent>(LightEntity);
            LightComponent.Color = Color;
            MarkDirty(LightEntity);

            return *this;
        }

		FDirectionalLightSystem& FDirectionalLightSystem::SetLightIntensity(FEntity LightEntity, float Intensity)
        {
            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FDirectionalLightComponent>(LightEntity);
            LightComponent.Intensity = Intensity;
            MarkDirty(LightEntity);

            return *this;
        }

        FEntity FDirectionalLightSystem::CreateDirectionalLight(const FVector3& Direction, const FVector3& Color, float Intensity)
        {
            auto Light = COORDINATOR().CreateEntity();
            COORDINATOR().AddComponent<ECS::COMPONENTS::FDirectionalLightComponent>(Light, {});

            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FDirectionalLightComponent>(Light);
            LightComponent.Direction = Direction;
            LightComponent.Color = Color;
            LightComponent.Intensity = Intensity;
            MarkDirty(Light);

            return Light;
        }
    }
}
