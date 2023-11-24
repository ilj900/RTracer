#pragma once

#include "maths.h"
#include "common_structures.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FLightComponent : FLight
        {
            FLightComponent()
            {
                Position = {0, 0, 0};
                Intensity = 5.f;

                Color = {1, 1, 1};
                Type = LIGHT_TYPE_NONE;

                Direction = {0, -1, 0};
                OuterAngle = 45.f;

                InnerAngle = 30.f;
            };

            FLightComponent( const FVector3& PositionIn, const FVector3& ColorIn, const FVector3& DirectionIn, float IntensityIn, uint32_t TypeIn, float OuterAngleIn, float InnerAngleIn)
            {
                    Position = PositionIn;
                    Color = ColorIn;
                    Type = TypeIn;
                    Direction = DirectionIn;
                    Intensity = IntensityIn;
                    OuterAngle = OuterAngleIn;
                    InnerAngle = InnerAngleIn;
            };
        };
    }
}