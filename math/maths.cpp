#include "maths.h"

FQuaternion FQuaternion::GetInverse()
{
    FQuaternion Result = GetConjugation();
    float QQ = W * W + X * X + Y * Y + Z * Z;
    assert(QQ > 0.000001f && "Quaternion length failed assertion.");
    Result.X /= QQ;
    Result.Y /= QQ;
    Result.Z /= QQ;
    Result.W /= QQ;
    return Result;
};

FQuaternion FQuaternion::GetConjugation()
{
    return {W, -X, -Y, -Z};
};

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

FVector3 operator*(const FVector3& A, float Val)
{
    return FVector3(A.X * Val, A.Y * Val, A.Z * Val);
}

FVector3 operator+(const FVector3& A, const FVector3& B)
{
    return FVector3(A.X + B.X, A.Y + B.Y, A.Z + B.Z);
}

FVector3& operator+=(FVector3& A, const FVector3& B)
{
    A.X += B.X;
    A.Y += B.Y;
    A.Z += B.Z;

    return A;
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

FQuaternion operator*(const FQuaternion& A, const FQuaternion& B)
{
    FQuaternion Result;
    Result.W = A.W * B.W - A.X * B.X - A.Y * B.Y - A.Z * B.Z;
    Result.X = A.W * B.X + A.X * B.W + A.Y * B.Z - A.Z * B.Y;
    Result.Y = A.W * B.Y - A.X * B.Z + A.Y * B.W + A.Z * B.X;
    Result.Z = A.W * B.Z + A.X * B.Y - A.Y * B.X + A.Z * B.W;
    return Result;
}

FVector3 FVector3::Rotate(float Angle, const FVector3& Axis)
{
    float HalfAngle = Angle * 0.5f;
    float SinHalfAngle = std::sin(HalfAngle);
    FQuaternion RotationQuaternion(std::cos(HalfAngle), Axis.X * SinHalfAngle, Axis.Y * SinHalfAngle, Axis.Z * SinHalfAngle);
    FQuaternion InvertedRotationQuaternion = RotationQuaternion.GetInverse();
    FQuaternion Vector(0.f, X, Y, Z);
    FQuaternion Tmp1 = RotationQuaternion * Vector;
    FQuaternion Tmp2 = Tmp1 * InvertedRotationQuaternion;
    return FVector3(Tmp2.X, Tmp2.Y, Tmp2.Z);
}

FMatrix4 LookAt(const FVector3& Eye, const FVector3& Point, const FVector3& Up)
{
    FMatrix4 ViewMatrix;

    FVector3 F = (Point - Eye).GetNormalized();
    FVector3 R = (F * Up).GetNormalized();
    FVector3 U = (R * F).GetNormalized();

    ViewMatrix.Data[0].X = R.X;
    ViewMatrix.Data[0].Y = U.X;
    ViewMatrix.Data[0].Z = -F.X;
    ViewMatrix.Data[0].W = 0.f;

    ViewMatrix.Data[1].X = R.Y;
    ViewMatrix.Data[1].Y = U.Y;
    ViewMatrix.Data[1].Z = -F.Y;
    ViewMatrix.Data[1].W = 0.f;

    ViewMatrix.Data[2].X = R.Z;
    ViewMatrix.Data[2].Y = U.Z;
    ViewMatrix.Data[2].Z = -F.Z;
    ViewMatrix.Data[2].W = 0.f;

    ViewMatrix.Data[3].X = -Dot(R, Eye);
    ViewMatrix.Data[3].Y = -Dot(U, Eye);
    ViewMatrix.Data[3].Z = Dot(F, Eye);
    ViewMatrix.Data[3].W = 1.f;

    return ViewMatrix;
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