#include "camera.h"

void FCamera::WS(float Val)
{
    Position += Direction.GetNormalized() * Val;
}

void FCamera::AD(float Val)
{
    auto Right = Direction.GetNormalized() * Up.GetNormalized();
    Position += Right * Val;
}

void FCamera::QE(float Val)
{
    Up = Up.Rotate(Val, Direction);
}

void FCamera::ZC(float Val)
{
    Position += Up.GetNormalized() * Val;
}

void FCamera::MLMR(float Val)
{
    Direction = Direction.Rotate(Val, Up);
}

void FCamera::MUMD(float Val)
{
    Direction = Direction.Rotate(Val, (Direction * Up).GetNormalized());
}

FMatrix4 FCamera::GetViewMatrix()
{
    return LookAt(Position, Position + Direction, Up);
}

FMatrix4 FCamera::GetProjectionMatrix()
{
    return GetPerspective(FOV / 90.f, Ratio, ZNear, ZFar);
}