#include "named_resources.h"
#include "light_component.h"
#include "point_light_system.h"

namespace ECS
{
    namespace SYSTEMS
    {
        void FPointLightSystem::Init(uint32_t NumberOfSimultaneousSubmits)
        {
			LoadedPointLightsCount = 0;
			CurrentPointLightsCount = 0;

            FGPUBufferableSystem::Init(NumberOfSimultaneousSubmits, sizeof(ECS::COMPONENTS::FPointLightComponent) * MAX_POINT_LIGHTS,
                                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, "Device_Point_Lights");
        }

        bool FPointLightSystem::Update()
        {
			bool bAnyUpdate = false;

			if (LoadedPointLightsCount != CurrentPointLightsCount)
			{
				RESOURCE_ALLOCATOR()->LoadDataToBuffer(UTILITY_INFO_BUFFER, {sizeof(uint32_t)}, { offsetof(FUtilityData, ActivePointLightsCount)}, {&CurrentPointLightsCount});
				CurrentPointLightsCount = LoadedPointLightsCount;
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

			if (LoadedPointLightsCount != CurrentPointLightsCount)
			{
				RESOURCE_ALLOCATOR()->LoadDataToBuffer(UTILITY_INFO_BUFFER, {sizeof(uint32_t)}, { offsetof(FUtilityData, ActivePointLightsCount)}, {&CurrentPointLightsCount});
				CurrentPointLightsCount = LoadedPointLightsCount;
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

			CurrentPointLightsCount++;

            return Light;
        }
    }
}
