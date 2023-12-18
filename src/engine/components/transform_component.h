#pragma once

#include "maths.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FTransformComponent
        {
            FVector3 Position = {0.f, 0.f, 0.f};
            FVector3 Direction = {0.f, 0.f, 1.f};
            FVector3 Up = {0.f, 1.f, 0.f};
            FVector3 Scale = {1.f, 1.f, 1.f};
        };
    }
}