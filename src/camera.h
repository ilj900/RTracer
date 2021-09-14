#pragma once

#include "maths.h"

class FCamera
{
public:
    FCamera() = default;
    void WS(float Val);
    void AD(float Val);
    void QE(float Val);
    void ZC(float Val);
    void MLMR(float Val);
    void MUMD(float Val);
    FMatrix4 GetViewMatrix();
    FMatrix4 GetProjectionMatrix();
private:
    FVector3 Position = {0.f, 0.f, 1.f};
    FVector3 Direction = {0.f, 0.f, -1.f};
    FVector3 Up = {0.f, 1.f, 0.f};
    float ZNear = 0.1f;
    float ZFar = 1000.f;
    float FOV = 45.f;
    float Ratio = 1.7777777777777777f;
};