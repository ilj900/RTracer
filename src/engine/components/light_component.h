#pragma once

#include "maths.h"
#include "common_defines.h"
#include "common_structures.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FPointLightComponent : FPointLight
        {
			FPointLightComponent()
            {
                Position = {0, 0, 0};
                Intensity = 5.f;
                Color = {1, 1, 1};
            };

			FPointLightComponent(const FVector3& PositionIn, const FVector3& ColorIn, float IntensityIn)
            {
                    Position = PositionIn;
                    Color = ColorIn;
                    Intensity = IntensityIn;
            };
        };

		struct FDirectionalLightComponent : FDirectionalLight
		{
			FDirectionalLightComponent()
			{
					Position = {0, 0, 0};
					Intensity = 5.f;
					Color = {1, 1, 1};
					Direction = {0, -1, 0};
			};

			FDirectionalLightComponent(const FVector3& PositionIn, const FVector3& ColorIn, const FVector3& DirectionIn, float IntensityIn)
			{
					Position = PositionIn;
					Intensity = IntensityIn;
					Color = ColorIn;
					Direction = DirectionIn;
			};
		};

		struct FSpotLightComponent : FSpotLight
		{
			FSpotLightComponent()
			{
					Position = {0, 0, 0};
					Intensity = 5.f;
					Color = {1, 1, 1};
					Direction = {0, -1, 0};
					InnerAngle = 30.f;
					OuterAngle = 45.f;
			};

			FSpotLightComponent(const FVector3& PositionIn, const FVector3& ColorIn, const FVector3& DirectionIn, float IntensityIn, float InnerAngleIn, float OuterAngleIn)
			{
					Position = PositionIn;
					Intensity = IntensityIn;
					Color = ColorIn;
					Direction = DirectionIn;
					InnerAngle = InnerAngleIn;
					OuterAngle = OuterAngleIn;
			};
		};
    }
}