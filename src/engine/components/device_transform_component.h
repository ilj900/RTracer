#pragma once

#include "maths.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FDeviceTransformComponent
        {
            FDeviceTransformComponent() = default;
            FDeviceTransformComponent(FMatrix4& ModelMatrix) :
                ModelMatrix(ModelMatrix) {};

            FMatrix4 ModelMatrix;
        };
    }
}