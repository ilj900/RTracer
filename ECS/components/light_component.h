#pragma once

#include "maths.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FLightComponent
        {
            enum class LightType {NONE, POINT_LIGHT, DIRECTIONAL_LIGHT, SPOT_LIGHT};
            FLightComponent() = default;
            FLightComponent( const FVector3& PositionIn,
                    const FVector3& ColorIn,
                    const FVector3& DirectionIn,
                    float IntensityIn,
                    LightType Type,
                    float OuterAngleIn,
                    float InnerAngleIn) :
                    Position(PositionIn),
                    Color(ColorIn),
                    Direction(DirectionIn),
                    Intensity(IntensityIn),
                    OuterAngle(OuterAngleIn),
                    InnerAngle(InnerAngleIn) {};

            FVector3 Position = {0, 0, 0};
            float Intensity = 5.f;

            FVector3 Color = {1, 1, 1};
            LightType Type = LightType::NONE;

            FVector3 Direction = {0, -1, 0};
            float OuterAngle = 45.f;

            float InnerAngle = 30.f;
            float dummy_1;
            float dummy_2;
            float dummy_3;
        };
    }
}

#define LIGHT_TYPE ECS::COMPONENTS::FLightComponent::LightType