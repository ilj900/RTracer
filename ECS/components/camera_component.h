#pragma once

#include "entities/entity.h"
#include "maths.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FCameraComponent
        {
            FCameraComponent() = default;
            FCameraComponent(FVector3& Position, FVector3& Direction, FVector3& Up, float ZNear, float ZFar, float FOV, float Ration) :
                Position(Position), Direction(Direction), Up(Up), ZNear(ZNear), ZFar(ZFar), FOV(FOV), Ratio(Ration) {};
            FVector3 Position = {0.f, 0.f, 5.f};
            FVector3 Direction = {0.f, 0.f, -1.f};
            FVector3 Up = {0.f, 1.f, 0.f};
            float ZNear = 0.1f;
            float ZFar = 1000.f;
            float FOV = 45.f;
            float Ratio = 1.7777777777777777f;
        };
    }
}