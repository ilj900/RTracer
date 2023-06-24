#pragma once

#include "buffer.h"

#include "system.h"
#include "coordinator.h"

#include "maths.h"

namespace ECS
{
    namespace SYSTEMS
    {
        class FLightSystem : public FSystem
        {
        public:
            void Init(int NumberOfSimultaneousSubmits);
            bool Update();
            FLightSystem& SetLightPosition(FEntity LightEntity, const FVector3& Position);
            FLightSystem& SetLightPosition(FEntity LightEntity, float X, float Y, float Z);
            FLightSystem& SetLightDirection(FEntity LightEntity, const FVector3& Direction);
            FLightSystem& SetLightColor(FEntity LightEntity, const FVector3& Color);
            FLightSystem& SetLightIntensity(FEntity LightEntity, float Intensity);

            FEntity CreatePointLight(const FVector3& Position, const FVector3& Color, float Intensity);
            FEntity CreateDirectionalLight(const FVector3& Direction, const FVector3& Color, float Intensity);
            FEntity CreateSpotLight(const FVector3& Position, const FVector3& Color, float Intensity, float OuterAngle, float InnerAngle);

            void RequestAllUpdate();
            void RequestUpdate(int FrameIndex);

            int GetTotalSize();


        public:
            std::vector<bool> BufferPartThatNeedsUpdate;
            int NumberOfSimultaneousSubmits = 2;

            FBuffer DeviceLightBuffer;
        };
    }
}

#define LIGHT_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FLightSystem>()
