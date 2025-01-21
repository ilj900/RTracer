#include "light_component.h"
#include "point_light_system.h"

namespace ECS
{
    namespace SYSTEMS
    {
        void FPointLightSystem::Init(uint32_t NumberOfSimultaneousSubmits)
        {
			UtilityPointLight.ActiveLightsCount = 0;
            FGPUBufferableSystem::Init(NumberOfSimultaneousSubmits, sizeof(ECS::COMPONENTS::FPointLightComponent) * MAX_POINT_LIGHTS,
                                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, "Device_Point_Lights");
        }

        bool FPointLightSystem::Update()
        {
			bool bAnyUpdate = false;

			if (UtilityPointLight.ActiveLightsCount != CurrentPointLightsCount)
			{
				RESOURCE_ALLOCATOR()->LoadDataToBuffer("UtilityInfoPointLight", {sizeof(FUtilityPointLight)}, {0}, {&UtilityPointLight});
				CurrentPointLightsCount = UtilityPointLight.ActiveLightsCount;
			}

			for (auto& Entry : EntitiesToUpdate)
			{
				bAnyUpdate |= !Entry.empty();
			}

			if (bAnyUpdate)
			{
				FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FPointLightComponent>();
			}

			return bAnyUpdate;
        }

        bool FPointLightSystem::Update(int Index)
        {
			bool bAnyUpdate = false;

			if (UtilityPointLight.ActiveLightsCount != CurrentPointLightsCount)
			{
				RESOURCE_ALLOCATOR()->LoadDataToBuffer("UtilityInfoPointLight", {sizeof(FUtilityPointLight)}, {0}, {&UtilityPointLight});
				CurrentPointLightsCount = UtilityPointLight.ActiveLightsCount;
			}

			for (auto& Entry : EntitiesToUpdate)
			{
				bAnyUpdate |= !Entry.empty();
			}

			if (bAnyUpdate)
			{
				FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FPointLightComponent>(Index);
			}

			return bAnyUpdate;
        }

		FPointLightSystem& FPointLightSystem::SetLightPosition(FEntity LightEntity, float X, float Y, float Z)
        {
            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FPointLightComponent>(LightEntity);
            LightComponent.Position = FVector3(X, Y, Z);
            MarkDirty(LightEntity);

            return *this;
        }

		FPointLightSystem& FPointLightSystem::SetLightPosition(FEntity LightEntity, const FVector3& Position)
        {
            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FPointLightComponent>(LightEntity);
            LightComponent.Position = Position;
            MarkDirty(LightEntity);

            return *this;
        }

		FPointLightSystem& FPointLightSystem::SetLightColor(FEntity LightEntity, const FVector3& Color)
        {
            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FPointLightComponent>(LightEntity);
            LightComponent.Color = Color;
            MarkDirty(LightEntity);

            return *this;
        }

		FPointLightSystem& FPointLightSystem::SetLightIntensity(FEntity LightEntity, float Intensity)
        {
            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FPointLightComponent>(LightEntity);
            LightComponent.Intensity = Intensity;
            MarkDirty(LightEntity);

            return *this;
        }

        FEntity FPointLightSystem::CreatePointLight(const FVector3& Position, const FVector3& Color, float Intensity)
        {
            auto Light = COORDINATOR().CreateEntity();
            COORDINATOR().AddComponent<ECS::COMPONENTS::FPointLightComponent>(Light, {});

            auto& LightComponent = COORDINATOR().GetComponent<COMPONENTS::FPointLightComponent>(Light);
            LightComponent.Position = Position;
            LightComponent.Color = Color;
            LightComponent.Intensity = Intensity;
            MarkDirty(Light);

			UtilityPointLight.ActiveLightsCount++;

            return Light;
        }
    }
}
