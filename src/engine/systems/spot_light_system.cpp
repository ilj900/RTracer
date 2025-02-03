#include "named_resources.h"
#include "light_component.h"
#include "spot_light_system.h"

namespace ECS
{
    namespace SYSTEMS
    {
        void FSpotLightSystem::Init(uint32_t NumberOfSimultaneousSubmits)
        {
			UtilitySpotLight.ActiveLightsCount = 0;
            FGPUBufferableSystem::Init(NumberOfSimultaneousSubmits, sizeof(ECS::COMPONENTS::FSpotLightComponent) * MAX_LIGHTS,
                                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, "Device_Spot_Lights");
        }

        bool FSpotLightSystem::Update()
        {
			bool bAnyUpdate = false;

			if (UtilitySpotLight.ActiveLightsCount != CurrentSpotLightsCount)
			{
				RESOURCE_ALLOCATOR()->LoadDataToBuffer(UTILITY_INFO_SPOT_LIGHT_BUFFER, {sizeof(FUtilitySpotLight)}, {0}, {&UtilitySpotLight});
				CurrentSpotLightsCount = UtilitySpotLight.ActiveLightsCount;
			}

			for (auto& Entry : EntitiesToUpdate)
			{
				bAnyUpdate |= !Entry.empty();
			}

			if (bAnyUpdate)
			{
				FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FSpotLightComponent>();
			}

			return bAnyUpdate;
        }

        bool FSpotLightSystem::Update(int Index)
        {
			bool bAnyUpdate = false;

			if (UtilitySpotLight.ActiveLightsCount != CurrentSpotLightsCount)
			{
				RESOURCE_ALLOCATOR()->LoadDataToBuffer(UTILITY_INFO_SPOT_LIGHT_BUFFER, {sizeof(FUtilitySpotLight)}, {0}, {&UtilitySpotLight});
				CurrentSpotLightsCount = UtilitySpotLight.ActiveLightsCount;
			}

			for (auto& Entry : EntitiesToUpdate)
			{
				bAnyUpdate |= !Entry.empty();
			}

			if (bAnyUpdate)
			{
				FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FSpotLightComponent>(Index);
			}

			return bAnyUpdate;
        }

		FSpotLightSystem& FSpotLightSystem::SetLightPosition(FEntity LightEntity, float X, float Y, float Z)
        {
            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FSpotLightComponent>(LightEntity);
            LightComponent.Position = FVector3(X, Y, Z);
            MarkDirty(LightEntity);

            return *this;
        }

		FSpotLightSystem& FSpotLightSystem::SetLightPosition(FEntity LightEntity, const FVector3& Position)
        {
            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FSpotLightComponent>(LightEntity);
            LightComponent.Position = Position;
            MarkDirty(LightEntity);

            return *this;
        }

		FSpotLightSystem& FSpotLightSystem::SetLightDirection(FEntity LightEntity, const FVector3& Direction)
        {
            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FSpotLightComponent>(LightEntity);
            LightComponent.Direction = Direction;
            MarkDirty(LightEntity);

            return *this;
        }

		FSpotLightSystem& FSpotLightSystem::SetLightDirection(FEntity LightEntity, float X, float Y, float Z)
		{
			auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FSpotLightComponent>(LightEntity);
			LightComponent.Direction = {X, Y, Z};
			MarkDirty(LightEntity);

			return *this;
		}

		FSpotLightSystem& FSpotLightSystem::SetLightColor(FEntity LightEntity, const FVector3& Color)
        {
            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FSpotLightComponent>(LightEntity);
            LightComponent.Color = Color;
            MarkDirty(LightEntity);

            return *this;
        }

		FSpotLightSystem& FSpotLightSystem::SetLightIntensity(FEntity LightEntity, float Intensity)
        {
            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FSpotLightComponent>(LightEntity);
            LightComponent.Intensity = Intensity;
            MarkDirty(LightEntity);

            return *this;
        }

		FSpotLightSystem& FSpotLightSystem::SetInnerAngle(FEntity LightEntity, float InnerAngle)
		{
			auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FSpotLightComponent>(LightEntity);
			LightComponent.InnerAngle = InnerAngle;
			MarkDirty(LightEntity);

			return *this;
		}

		FSpotLightSystem& FSpotLightSystem::SetOuterAngle(FEntity LightEntity, float OuterAngle)
		{
			auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FSpotLightComponent>(LightEntity);
			LightComponent.OuterAngle = OuterAngle;
			MarkDirty(LightEntity);

			return *this;
		}

        FEntity FSpotLightSystem::CreateSpotLight(const FVector3& Position, const FVector3& Direction, const FVector3& Color, float Intensity, float OuterAngle, float InnerAngle)
        {
            auto Light = COORDINATOR().CreateEntity();
            COORDINATOR().AddComponent<ECS::COMPONENTS::FSpotLightComponent>(Light, {});

            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FSpotLightComponent>(Light);
			LightComponent.Direction = Direction.GetNormalized();
            LightComponent.Position = Position;
            LightComponent.Color = Color;
            LightComponent.Intensity = Intensity;
            LightComponent.OuterAngle = OuterAngle * 0.5f;
            LightComponent.InnerAngle = InnerAngle * 0.5f;
            MarkDirty(Light);

			UtilitySpotLight.ActiveLightsCount++;

            return Light;
        }
    }
}
