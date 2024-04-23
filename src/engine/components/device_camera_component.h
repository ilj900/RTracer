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
                ViewMatrix = {1.f ,0.f, -0.f, 0.f, -0.f, 1.f, -0.f, 0.f, 0.f, 0.f, 1.f, 0.f, -0.f, -0.f, -1.f, 1.f};
				InverseViewMatrix = ViewMatrix.GetInverse();
                ProjectionMatrix = {2.20292854f, 0.f, 0.f, 0.f, 0.f, -3.91631746f, 0.f, 0.f, 0.f, 0.f, -1.f, -1.f, 0.f, 0.f, -0.1f, 0.f};
				InverseProjectionMatrix = ProjectionMatrix.GetInverse();
                FVector3 Origin = {0, 0, 0,};
                float ZNear = 0.01;
                FVector3 ViewDirection = {0, 0, 1,};
                float ZFar = 10000.f;
                float FOV = 90.f;
            };

            FDeviceCameraComponent(FMatrix4& ViewMatrixIn, FMatrix4& ProjectionMatrixIn, FVector3 OriginIn, float ZNearIn, FVector3 ViewDirectionIn, float ZFarIn, float FOVIn)
            {
                ViewMatrix = ViewMatrixIn;
				InverseViewMatrix = ViewMatrixIn.GetInverse();
                ProjectionMatrix = ProjectionMatrixIn;
				InverseProjectionMatrix = ProjectionMatrix.GetInverse();
                Origin = OriginIn;
                ZNear =ZNearIn;
                ViewDirection = ViewDirectionIn;
                ZFar = ZFarIn;
                FOV = FOVIn;
            };
        };
    }
}