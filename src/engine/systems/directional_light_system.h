#pragma once

#include "buffer.h"

#include "gpu_bufferable_system.h"
#include "coordinator.h"

#include "maths.h"

#include "common_structures.h"

namespace ECS
{
    namespace SYSTEMS
    {
        class FDirectionalLightSystem : public FGPUBufferableSystem
        {
        public:
            void Init(uint32_t NumberOfSimultaneousSubmits) override;
            bool Update() override;
            bool Update(int Index) override;

			FDirectionalLightSystem& SetLightPosition(FEntity LightEntity, const FVector3& Position);
			FDirectionalLightSystem& SetLightPosition(FEntity LightEntity, float X, float Y, float Z);
			FDirectionalLightSystem& SetLightDirection(FEntity LightEntity, const FVector3& Direction);
			FDirectionalLightSystem& SetLightDirection(FEntity LightEntity, float X, float Y, float Z);
			FDirectionalLightSystem& SetLightColor(FEntity LightEntity, const FVector3& Color);
			FDirectionalLightSystem& SetLightIntensity(FEntity LightEntity, float Intensity);

            FEntity CreateDirectionalLight(const FVector3& Direction, const FVector3& Color, float Intensity);

            static const uint32_t MAX_DIRECTIONAL_LIGHTS = 32;
			uint32_t LoadedDirectionalLightsCount = 0;
			float LoadedDirectionalLightsPower = 0.f;
			uint32_t CurrentDirectionalLightsCount = 0;
			float CurrentDirectionalLightsPower = 0.f;
			bool bAliasTableShouldBeUpdated = false;
        };
    }
}

#define DIRECTIONAL_LIGHT_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FDirectionalLightSystem>()
