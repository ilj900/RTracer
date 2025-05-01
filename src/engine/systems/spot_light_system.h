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
        class FSpotLightSystem : public FGPUBufferableSystem
        {
        public:
            void Init(uint32_t NumberOfSimultaneousSubmits) override;
            bool Update() override;
            bool Update(int Index) override;

            FSpotLightSystem& SetLightPosition(FEntity LightEntity, const FVector3& Position);
            FSpotLightSystem& SetLightPosition(FEntity LightEntity, float X, float Y, float Z);
            FSpotLightSystem& SetLightDirection(FEntity LightEntity, const FVector3& Direction);
			FSpotLightSystem& SetLightDirection(FEntity LightEntity, float X, float Y, float Z);
            FSpotLightSystem& SetLightColor(FEntity LightEntity, const FVector3& Color);
            FSpotLightSystem& SetLightIntensity(FEntity LightEntity, float Intensity);
			FSpotLightSystem& SetInnerAngle(FEntity LightEntity, float InnerAngle);
			FSpotLightSystem& SetOuterAngle(FEntity LightEntity, float OuterAngle);

            FEntity CreateSpotLight(const FVector3& Position, const FVector3& Direction, const FVector3& Color, float Intensity, float OuterAngle, float InnerAngle);

            static const uint32_t MAX_LIGHTS = 32;
			uint32_t LoadedSpotLightsCount = 0;
			float LoadedTotalSpotlightPower = 0.f;
			uint32_t CurrentSpotLightsCount = 0;
			float CurrentTotalSpotlightPower = 0.f;
			bool bAliasTableShouldBeUpdated = false;
        };
    }
}

#define SPOT_LIGHT_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FSpotLightSystem>()
