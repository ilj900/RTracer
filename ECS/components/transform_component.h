#pragma once

#include "maths.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FTransformComponent
        {
            FVector3 Position;
            FVector3 Direction;
            FVector3 Up;
        };
    }
}