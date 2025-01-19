#pragma once

#include "buffer.h"

#include "gpu_bufferable_system.h"
#include "coordinator.h"

#include "maths.h"

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

            const uint32_t MAX_DIRECTIONAL_LIGHTS = 32;
        };
    }
}

#define DIRECTIONAL_LIGHT_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FDirectionalLightSystem>()
