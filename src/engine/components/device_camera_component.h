#pragma once

#include "maths.h"
#include "common_structures.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FDeviceCameraComponent :FDeviceCamera
        {
            FDeviceCameraComponent()
            {
                Origin = {0, 0, 7.5};
				Direction = {0, 0, -1};
				Right = {1, 0, 0};
				Up = {0, 1, 0};
				SensorSizeX = 0.036;
				SensorSizeY = 0.024;
				FocalDistance = 0.018;
            };

            FDeviceCameraComponent(FVector3& OriginIn, FVector3& DirectionIn)
            {
                Origin = OriginIn;
				Direction = DirectionIn;
				Right = {1, 0, 0};
				Up = {0, 1, 0};
				SensorSizeX = 0.036;
				SensorSizeY = 0.024;
				FocalDistance = 0.05;
            };
        };
    }
}