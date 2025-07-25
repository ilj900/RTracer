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

			if (bAliasTableShouldBeUpdated)
			{
				auto [UpdatedAliasTable, _] = GenerateImportanceMapFast<ECS::COMPONENTS::FPointLightComponent>(COORDINATOR().Data<ECS::COMPONENTS::FPointLightComponent>(),
					CurrentPointLightsCount, 1, [](ECS::COMPONENTS::FPointLightComponent Component){return double(Component.Power);});

				RESOURCE_ALLOCATOR()->LoadDataToBuffer(POINT_LIGHTS_IMPORTANCE_BUFFER, UpdatedAliasTable.size() * sizeof(FAliasTableEntry), 0, UpdatedAliasTable.data());

				bAliasTableShouldBeUpdated = false;
			}

			if (LoadedPointLightsCount != CurrentPointLightsCount ||
				LoadedPointLightPower != CurrentPointLightPower)
			{
				/// Load two entries even though only one can be dirty
				/// Pay close attention to the order of member fields: CurrentPointLightsCount should be followed by CurrentPointLightPower
				RESOURCE_ALLOCATOR()->LoadDataToBuffer(UTILITY_INFO_BUFFER, sizeof(uint32_t) * 2,  offsetof(FUtilityData, ActivePointLightsCount), &CurrentPointLightsCount);
				LoadedPointLightsCount = CurrentPointLightsCount;
				LoadedPointLightPower = CurrentPointLightPower;
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

			if (bAliasTableShouldBeUpdated)
			{
				auto [UpdatedAliasTable, _] = GenerateImportanceMapFast<ECS::COMPONENTS::FPointLightComponent>(COORDINATOR().Data<ECS::COMPONENTS::FPointLightComponent>(),
					CurrentPointLightsCount, 1, [](ECS::COMPONENTS::FPointLightComponent Component){return double(Component.Power);});

				RESOURCE_ALLOCATOR()->LoadDataToBuffer(POINT_LIGHTS_IMPORTANCE_BUFFER, UpdatedAliasTable.size() * sizeof(FAliasTableEntry), 0, UpdatedAliasTable.data());

				bAliasTableShouldBeUpdated = false;
			}

			if (LoadedPointLightsCount != CurrentPointLightsCount ||
				LoadedPointLightPower != CurrentPointLightPower)
			{
				/// Load two entries even though only one can be dirty
				/// Pay close attention to the order of member fields: CurrentPointLightsCount should be followed by CurrentPointLightPower
				RESOURCE_ALLOCATOR()->LoadDataToBuffer(UTILITY_INFO_BUFFER, sizeof(uint32_t) * 2,  offsetof(FUtilityData, ActivePointLightsCount), &CurrentPointLightsCount);
				LoadedPointLightsCount = CurrentPointLightsCount;
				LoadedPointLightPower = CurrentPointLightPower;
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
			LightComponent.Power = (Color.x + Color.y + Color.z) * Intensity;
            MarkDirty(Light);

			CurrentPointLightsCount++;
			CurrentPointLightPower += LightComponent.Power;
			bAliasTableShouldBeUpdated = true;

            return Light;
        }
    }
}
