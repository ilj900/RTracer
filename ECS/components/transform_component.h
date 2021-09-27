#pragma once

#include "maths.h"

namespace ECS
{
    namespace Components
    {
        struct TransformComponent
        {
            FVector3 Position;
            FVector3 Direction;
            FVector3 Up;
        };
    }
}