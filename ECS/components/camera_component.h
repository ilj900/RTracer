#pragma once

#include "entities/entity.h"
#include "maths.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FCameraComponent
        {
            FVector3 Position = {0.f, 0.f, 1.f};
            FVector3 Direction = {0.f, 0.f, -1.f};
            FVector3 Up = {0.f, 1.f, 0.f};
            float ZNear = 0.1f;
            float ZFar = 1000.f;
            float FOV = 45.f;
            float Ratio = 1.7777777777777777f;
        };
    }
}