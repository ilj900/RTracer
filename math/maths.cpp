#include "maths.h"

FVector3 FVector3::GetNormalized() const
{
    float L = std::sqrt(X * X + Y * Y + Z * Z);

    if (L <= 0.000001f)
    {
        throw std::runtime_error("Failed to normalize FVector3");
    }

    return FVector3(X / L, Y / L, Z / L);
}

FVector3 operator*(const FVector3& A, const FVector3& B)
{
    return FVector3(A.Y * B.Z - B.Y * A.Z, A.Z * B.X - B.Z * A.X, A.X * B.Y - B.X * A.Y);
}

FVector3 operator-(const FVector3& A, const FVector3& B)
{
    return FVector3(A.X - B.X, A.Y - B.Y, A.Z - B.Z);
}

float Dot(const FVector3& L, const FVector3& R)
{
    return L.X * R.X + L.Y * R.Y + L.Z * R.Z;
}

bool operator==(const FVector4& A, const FVector4& B)
{
    return (A.X == B.X && A.Y == B.Y && A.Z == B.Z && A.W == B.W);
}

bool operator==(const FVector3& A, const FVector3& B)
{
    return (A.X == B.X && A.Y == B.Y && A.Z == B.Z);
}

bool operator==(const FVector2& A, const FVector2& B)
{
    return (A.X == B.X && A.Y == B.Y);
}

FMatrix4 Rotate(float Angle, const FVector3& Axis)
{
    auto C = std::cos(Angle);
    auto S = std::sin(Angle);

    FMatrix4 RotationMatrix;

    RotationMatrix.Data[0].X = C + (1.f - C) * Axis.X * Axis.X;
    RotationMatrix.Data[0].Y = (1.f - C) * Axis.X * Axis.Y - S * Axis.Z;
    RotationMatrix.Data[0].Z = (1.f - C) * Axis.X * Axis.Z + S * Axis.Y;
    RotationMatrix.Data[0].W = 0.f;

    RotationMatrix.Data[1].X = (1.f - C) * Axis.X * Axis.Y + S * Axis.Z;
    RotationMatrix.Data[1].Y = C + (1.f - C) * Axis.Y * Axis.Y;
    RotationMatrix.Data[1].Z = (1.f - C) * Axis.Y * Axis.Z - S * Axis.X;
    RotationMatrix.Data[1].W = 0.f;

    RotationMatrix.Data[2].X = (1.f - C) * Axis.X * Axis.Z - S * Axis.Y;
    RotationMatrix.Data[2].Y = (1.f - C) * Axis.Y * Axis.Z + S * Axis.X;
    RotationMatrix.Data[2].Z = C + (1.f - C) * Axis.Z * Axis.Z;
    RotationMatrix.Data[2].W = 0.f;

    RotationMatrix.Data[3].X = 0.f;
    RotationMatrix.Data[3].Y = 0.f;
    RotationMatrix.Data[3].Z = 0.f;
    RotationMatrix.Data[3].W = 1.f;

    return RotationMatrix;
}

FMatrix4 LookAt(const FVector3& Eye, const FVector3& Point, const FVector3& Up)
{
    FMatrix4 RotationMatrix;

    FVector3 F = (Point - Eye).GetNormalized();
    FVector3 R = (F * Up).GetNormalized();
    FVector3 U = (R * F).GetNormalized();

    RotationMatrix.Data[0].X = R.X;
    RotationMatrix.Data[0].Y = U.X;
    RotationMatrix.Data[0].Z = -F.X;
    RotationMatrix.Data[0].W = 0.f;

    RotationMatrix.Data[1].X = R.Y;
    RotationMatrix.Data[1].Y = U.Y;
    RotationMatrix.Data[1].Z = -F.Y;
    RotationMatrix.Data[1].W = 0.f;

    RotationMatrix.Data[2].X = R.Z;
    RotationMatrix.Data[2].Y = U.Z;
    RotationMatrix.Data[2].Z = -F.Z;
    RotationMatrix.Data[2].W = 0.f;

    RotationMatrix.Data[3].X = -Dot(R, Eye);
    RotationMatrix.Data[3].Y = -Dot(U, Eye);
    RotationMatrix.Data[3].Z = Dot(F, Eye);
    RotationMatrix.Data[3].W = 1.f;

    return RotationMatrix;
}

FMatrix4 GetPerspective(float FOV, float AspectRatio, float NearDistance, float FarDistance)
{
    FMatrix4 PerspectiveMatrix;

    auto TanHalfFOV = std::tan(FOV / 2.f);

    PerspectiveMatrix.Data[0].X = 1.f / (AspectRatio * TanHalfFOV);
    PerspectiveMatrix.Data[1].Y = -1.f / TanHalfFOV;
    PerspectiveMatrix.Data[2].Z = -(FarDistance + NearDistance) / (FarDistance - NearDistance);
    PerspectiveMatrix.Data[2].W = -1.f;
    PerspectiveMatrix.Data[3].Z = -(FarDistance * NearDistance) / (FarDistance - NearDistance);
    PerspectiveMatrix.Data[3].W = 0.f;

    return PerspectiveMatrix;
}