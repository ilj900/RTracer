#pragma once

#include "buffer.h"

#include "gpu_bufferable_system.h"
#include "coordinator.h"

#include "maths.h"

namespace ECS
{
    namespace SYSTEMS
    {
        class FPointLightSystem : public FGPUBufferableSystem
        {
        public:
            void Init(uint32_t NumberOfSimultaneousSubmits) override;
            bool Update() override;
            bool Update(int Index) override;

            FPointLightSystem& SetLightPosition(FEntity LightEntity, const FVector3& Position);
            FPointLightSystem& SetLightPosition(FEntity LightEntity, float X, float Y, float Z);
            FPointLightSystem& SetLightColor(FEntity LightEntity, const FVector3& Color);
            FPointLightSystem& SetLightIntensity(FEntity LightEntity, float Intensity);

            FEntity CreatePointLight(const FVector3& Position, const FVector3& Color, float Intensity);

            const uint32_t MAX_POINT_LIGHTS = 512;
        };
    }
}

#define POINT_LIGHT_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FPointLightSystem>()
