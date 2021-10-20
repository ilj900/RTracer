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

float FVector3::Length()
{
    return X * X + Y * Y + Z * Z;
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

FMatrix4 operator*(const FMatrix4& A, const FMatrix4& B)
{
    FMatrix4 Result;
    Result.Data[0].X = A.Data[0].X * B.Data[0].X + A.Data[0].Y * B.Data[1].X + A.Data[0].Z * B.Data[2].X + A.Data[0].W * B.Data[3].X;
    Result.Data[0].Y = A.Data[0].X * B.Data[0].Y + A.Data[0].Y * B.Data[1].Y + A.Data[0].Z * B.Data[2].Y + A.Data[0].W * B.Data[3].Y;
    Result.Data[0].Z = A.Data[0].X * B.Data[0].Z + A.Data[0].Y * B.Data[1].Z + A.Data[0].Z * B.Data[2].Z + A.Data[0].W * B.Data[3].Z;
    Result.Data[0].W = A.Data[0].X * B.Data[0].W + A.Data[0].Y * B.Data[1].W + A.Data[0].Z * B.Data[2].W + A.Data[0].W * B.Data[3].W;
    Result.Data[1].X = A.Data[1].X * B.Data[0].X + A.Data[1].Y * B.Data[1].X + A.Data[1].Z * B.Data[2].X + A.Data[1].W * B.Data[3].X;
    Result.Data[1].Y = A.Data[1].X * B.Data[0].Y + A.Data[1].Y * B.Data[1].Y + A.Data[1].Z * B.Data[2].Y + A.Data[1].W * B.Data[3].Y;
    Result.Data[1].Z = A.Data[1].X * B.Data[0].Z + A.Data[1].Y * B.Data[1].Z + A.Data[1].Z * B.Data[2].Z + A.Data[1].W * B.Data[3].Z;
    Result.Data[1].W = A.Data[1].X * B.Data[0].W + A.Data[1].Y * B.Data[1].W + A.Data[1].Z * B.Data[2].W + A.Data[1].W * B.Data[3].W;
    Result.Data[2].X = A.Data[2].X * B.Data[0].X + A.Data[2].Y * B.Data[1].X + A.Data[2].Z * B.Data[2].X + A.Data[2].W * B.Data[3].X;
    Result.Data[2].Y = A.Data[2].X * B.Data[0].Y + A.Data[2].Y * B.Data[1].Y + A.Data[2].Z * B.Data[2].Y + A.Data[2].W * B.Data[3].Y;
    Result.Data[2].Z = A.Data[2].X * B.Data[0].Z + A.Data[2].Y * B.Data[1].Z + A.Data[2].Z * B.Data[2].Z + A.Data[2].W * B.Data[3].Z;
    Result.Data[2].W = A.Data[2].X * B.Data[0].W + A.Data[2].Y * B.Data[1].W + A.Data[2].Z * B.Data[2].W + A.Data[2].W * B.Data[3].W;
    Result.Data[3].X = A.Data[3].X * B.Data[0].X + A.Data[3].Y * B.Data[1].X + A.Data[3].Z * B.Data[2].X + A.Data[3].W * B.Data[3].X;
    Result.Data[3].Y = A.Data[3].X * B.Data[0].Y + A.Data[3].Y * B.Data[1].Y + A.Data[3].Z * B.Data[2].Y + A.Data[3].W * B.Data[3].Y;
    Result.Data[3].Z = A.Data[3].X * B.Data[0].Z + A.Data[3].Y * B.Data[1].Z + A.Data[3].Z * B.Data[2].Z + A.Data[3].W * B.Data[3].Z;
    Result.Data[3].W = A.Data[3].X * B.Data[0].W + A.Data[3].Y * B.Data[1].W + A.Data[3].Z * B.Data[2].W + A.Data[3].W * B.Data[3].W;

    return Result;
}

FVector3 operator*(const FMatrix4& A, const FVector3& B)
{
    return FVector3{A.Data[0].X * B.X + A.Data[0].Y * B.Y + A.Data[0].Z * B.Z,
                    A.Data[1].X * B.X + A.Data[1].Y * B.Y + A.Data[1].Z * B.Z,
                    A.Data[2].X * B.X + A.Data[2].Y * B.Y + A.Data[2].Z * B.Z};
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

/// Suppose incoming vectors are normalized
/// Return matrix that rotate OriginalDirection to FinalDirection around axis perpendicular to them
FMatrix4 GetRotationMatrix(FVector3 OriginalDirection, FVector3 FinalDirection)
{
    auto A = OriginalDirection * FinalDirection;
    auto C = Dot(OriginalDirection, FinalDirection);
    auto S = std::sin(std::acos(C));
    auto O = 1 - C;
    FMatrix4 Result{C + O * A.X * A.X, O * A.X * A.Y - S * A.Z, O * A.X * A.Z + S * A.Y, 0.f,
                    O * A.X * A.Y + S * A.Z, C + O * A.Y * A.Y, O * A.Y * A.Z - S * A.X, 0.f,
                    O * A.X * A.Z - S * A.Y, O * A.Y * A.Z + S * A.X, C + O * A.Z * A.Z, 0.f,
                    0.f, 0.f, 0.f, 1.f};

    return Result;
}

FMatrix4 GetRotationMatrixX(float Angle)
{
    auto S = std::sin(Angle);
    auto C = std::cos(Angle);
    FMatrix4 Result{1.f, 0.f, 0.f, 0.f,
                    0.f, C, -S, 0.f,
                    0.f, S, C, 0.f,
                    0.f, 0.f, 0.f, 1.f};

    return Result;
}

FMatrix4 GetRotationMatrixY(float Angle)
{
    auto S = std::sin(Angle);
    auto C = std::cos(Angle);
    FMatrix4 Result{C, 0.f, S, 0.f,
                    0.f, 1.f, 0.f, 0.f,
                    -S, 0.f, C, 0.f,
                    0.f, 0.f, 0.f, 1.f};

    return Result;
}

FMatrix4 GetRotationMatrixZ(float Angle)
{
    auto S = std::sin(Angle);
    auto C = std::cos(Angle);
    FMatrix4 Result{C, -S, 0.f, 0.f,
                    S, C, 0.f, 0.f,
                    0.f, 0.f, 1.f, 0.f,
                    0.f, 0.f, 0.f, 1.f};

    return Result;
}

FMatrix4 Transform(const FVector3& Position, const FVector3& Direction, const FVector3& Up, const FVector3& Scale)
{
    FMatrix4 ScaleMatrix{Scale.X, 0.f, 0.f, 0.f,
                         0.f, Scale.Y, 0.f, 0.f,
                         0.f, 0.f, Scale.Z, 0.f,
                         0.f, 0.f, 0.f, 1.f};

    /// Get rotation matrix to rotate base direction to align it with the desired one
    FMatrix4 RotationMatrixFirst = GetRotationMatrix(FVector3{0.f, 0.f, 1.f}, Direction);
    /// Calculate new UP direction
    FVector3 RotatedUp = RotationMatrixFirst * FVector3{0.f, 1.f, 0.f};
    /// Get second rotation matrix to align new UP direction with desired one.
    FMatrix4 RotationMatrixSecond = GetRotationMatrix(RotatedUp, Up);

    FMatrix4 TranslationMatrix(1.f, 0.f, 0.f, 0.f,
                               0.f, 1.f, 0.f, 0.f,
                               0.f, 0.f, 1.f, 0.f,
                               Position.X, Position.Y, Position.Z, 1.f);

    return ScaleMatrix * RotationMatrixFirst * RotationMatrixSecond * TranslationMatrix;
}

FMatrix4 LookAt(const FVector3& Eye, const FVector3& Point, const FVector3& Up)
{
    FVector3 F = (Point - Eye).GetNormalized();
    FVector3 R = (F * Up).GetNormalized();
    FVector3 U = (R * F).GetNormalized();

    return FMatrix4{R.X,            U.X,            -F.X,           0.f,
                    R.Y,            U.Y,            -F.Y,           0.f,
                    R.Z,            U.Z,            -F.Z,           0.f,
                    -Dot(R, Eye),   -Dot(U, Eye),   Dot(F, Eye),    1.f};
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