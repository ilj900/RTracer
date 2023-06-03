#pragma once

#include "maths.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FDeviceCameraComponent
        {
            FDeviceCameraComponent() = default;
            FDeviceCameraComponent(FMatrix4& ViewMatrix, FMatrix4& ProjectionMatrix) :
                ViewMatrix(ViewMatrix), ProjectionMatrix(ProjectionMatrix) {};

            FMatrix4 ViewMatrix = {1.f ,0.f, -0.f, 0.f, -0.f, 1.f, -0.f, 0.f, 0.f, 0.f, 1.f, 0.f, -0.f, -0.f, -1.f, 1.f};
            FMatrix4 ProjectionMatrix = {2.20292854f, 0.f, 0.f, 0.f, 0.f, -3.91631746f, 0.f, 0.f, 0.f, 0.f, -1.f, -1.f, 0.f, 0.f, -0.1f, 0.f};
        };
    }
}