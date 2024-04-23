#pragma once

#include "maths.h"
#include "common_structures.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FDeviceTransformComponent : FDeviceTransform
        {
            FDeviceTransformComponent() = default;
            explicit FDeviceTransformComponent(FMatrix4& ModelMatrixIn)
			{
				ModelMatrix = ModelMatrixIn;
				InverseModelMatrix = FMatrix3(ModelMatrix).GetInverse().Transpose();
			};
        };
    }
}