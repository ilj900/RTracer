#include "named_resources.h"
#include "light_component.h"
#include "directional_light_system.h"


namespace ECS
{
    namespace SYSTEMS
    {
        void FDirectionalLightSystem::Init(uint32_t NumberOfSimultaneousSubmits)
        {
			LoadedDirectionalLightsCount = 0;
			CurrentDirectionalLightsCount = 0;

            FGPUBufferableSystem::Init(NumberOfSimultaneousSubmits, sizeof(ECS::COMPONENTS::FDirectionalLightComponent) * MAX_DIRECTIONAL_LIGHTS,
                                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, "Device_Directional_Lights");
        }

        bool FDirectionalLightSystem::Update()
        {
			bool bAnyUpdate = false;

			if (bAliasTableShouldBeUpdated)
			{
				auto [UpdatedAliasTable, _] = GenerateImportanceMapFast<ECS::COMPONENTS::FDirectionalLightComponent>(COORDINATOR().Data<ECS::COMPONENTS::FDirectionalLightComponent>(),
					CurrentDirectionalLightsCount, 1, [](ECS::COMPONENTS::FDirectionalLightComponent Component){return double(Component.Power);});

				RESOURCE_ALLOCATOR()->LoadDataToBuffer(DIRECTIONAL_LIGHTS_IMPORTANCE_BUFFER, UpdatedAliasTable.size() * sizeof(FAliasTableEntry), 0, UpdatedAliasTable.data());

				bAliasTableShouldBeUpdated = false;
			}

			if (LoadedDirectionalLightsCount != CurrentDirectionalLightsCount ||
				LoadedDirectionalLightsPower != CurrentDirectionalLightsPower)
			{
				/// Load two entries even though only one can be dirty
				/// Pay close attention to the order of member fields: CurrentDirectionalLightsCount should be followed by CurrentDirectionalLightPower
				RESOURCE_ALLOCATOR()->LoadDataToBuffer(UTILITY_INFO_BUFFER, sizeof(uint32_t) * 2,  offsetof(FUtilityData, ActiveDirectionalLightsCount), &CurrentDirectionalLightsCount);
				LoadedDirectionalLightsCount = CurrentDirectionalLightsCount;
				LoadedDirectionalLightsPower = CurrentDirectionalLightsPower;
			}

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

			if (bAliasTableShouldBeUpdated)
			{
				auto [UpdatedAliasTable, _] = GenerateImportanceMapFast<ECS::COMPONENTS::FDirectionalLightComponent>(COORDINATOR().Data<ECS::COMPONENTS::FDirectionalLightComponent>(),
					CurrentDirectionalLightsCount, 1, [](ECS::COMPONENTS::FDirectionalLightComponent Component){return double(Component.Power);});

				RESOURCE_ALLOCATOR()->LoadDataToBuffer(DIRECTIONAL_LIGHTS_IMPORTANCE_BUFFER, UpdatedAliasTable.size() * sizeof(FAliasTableEntry), 0, UpdatedAliasTable.data());

				bAliasTableShouldBeUpdated = false;
			}

			if (LoadedDirectionalLightsCount != CurrentDirectionalLightsCount ||
				LoadedDirectionalLightsPower != CurrentDirectionalLightsPower)
			{
				/// Load two entries even though only one can be dirty
				/// Pay close attention to the order of member fields: CurrentDirectionalLightsCount should be followed by CurrentDirectionalLightPower
				RESOURCE_ALLOCATOR()->LoadDataToBuffer(UTILITY_INFO_BUFFER, sizeof(uint32_t) * 2,  offsetof(FUtilityData, ActiveDirectionalLightsCount), &CurrentDirectionalLightsCount);
				LoadedDirectionalLightsCount = CurrentDirectionalLightsCount;
				LoadedDirectionalLightsPower = CurrentDirectionalLightsPower;
			}

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
            LightComponent.Direction = Direction.GetNormalized();
            LightComponent.Color = Color;
            LightComponent.Intensity = Intensity;
			LightComponent.Power = (Color.x + Color.y + Color.z) * Intensity;
            MarkDirty(Light);

			CurrentDirectionalLightsCount++;
			CurrentDirectionalLightsPower += LightComponent.Power;
			bAliasTableShouldBeUpdated = true;

            return Light;
        }
    }
}
