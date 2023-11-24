#pragma once

#include "maths.h"
#include "common_structures.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FDeviceCameraComponent :FCamera
        {
            FDeviceCameraComponent()
            {
                ViewMatrix = {1.f ,0.f, -0.f, 0.f, -0.f, 1.f, -0.f, 0.f, 0.f, 0.f, 1.f, 0.f, -0.f, -0.f, -1.f, 1.f};
                ProjectionMatrix = {2.20292854f, 0.f, 0.f, 0.f, 0.f, -3.91631746f, 0.f, 0.f, 0.f, 0.f, -1.f, -1.f, 0.f, 0.f, -0.1f, 0.f};
            };

            FDeviceCameraComponent(FMatrix4& ViewMatrixIn, FMatrix4& ProjectionMatrixIn)
            {
                ViewMatrix = ViewMatrixIn;
                ProjectionMatrix = ProjectionMatrixIn;
            };
        };
    }
}