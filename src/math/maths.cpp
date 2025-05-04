#include "maths.h"

#include <string>

///****************************************************************
///FQuaternion and it's operations
///****************************************************************

FQuaternion FQuaternion::GetInverse()
{
    FQuaternion Result = GetConjugation();
    float Q2 = W * W + X * X + Y * Y + Z * Z;
    assert(Q2 > 0.000001f && "Quaternion length failed assertion.");
    Result.X /= Q2;
    Result.Y /= Q2;
    Result.Z /= Q2;
    Result.W /= Q2;
    return Result;
};

FQuaternion FQuaternion::GetConjugation()
{
    return {W, X == 0.f ? X : -X, Y == 0.f ? Y : -Y, Z == 0.f ? Z : -Z};
};

FQuaternion FQuaternion::GetNormalized()
{
    float Length = W * W + X * X + Y * Y + Z * Z;
	Length = sqrt(Length);
	return FQuaternion(W / Length, X / Length, Y / Length, Z / Length);
};

FQuaternion operator*(const FQuaternion& A, const FQuaternion& B)
{
    FQuaternion Result;
    Result.W = A.W * B.W - A.X * B.X - A.Y * B.Y - A.Z * B.Z;
    Result.X = A.W * B.X + A.X * B.W + A.Y * B.Z - A.Z * B.Y;
    Result.Y = A.W * B.Y - A.X * B.Z + A.Y * B.W + A.Z * B.X;
    Result.Z = A.W * B.Z + A.X * B.Y - A.Y * B.X + A.Z * B.W;
    return Result;
}

///****************************************************************
///FVector4 and it's operations
///****************************************************************

float FVector4::Length()
{
    return sqrt(X * X + Y * Y + Z * Z + W + W);
}

float FVector4::Length2()
{
    return X * X + Y * Y + Z * Z + W + W;
}

bool operator==(const FVector4& A, const FVector4& B)
{
    return (A.X == B.X && A.Y == B.Y && A.Z == B.Z && A.W == B.W);
}

FVector4 operator*(const FVector4& A, const FVector4& B)
{
    return FVector4(A.X * B.X, A.Y * B.Y, A.Z * B.Z, A.W * B.W);
}

FVector4 operator+(const FVector4& A, const FVector4& B)
{
    return FVector4(A.X + B.X, A.Y + B.Y, A.Z + B.Z, A.W + B.W);
}

FVector4 operator-(const FVector4& A, const FVector4& B)
{
    return FVector4(A.X - B.X, A.Y - B.Y, A.Z - B.Z, A.W - B.W);
}

FVector4 operator*(const FVector4& A, float Val)
{
    return FVector4(A.X * Val, A.Y * Val, A.Z * Val, A.W * Val);
}

FVector4 operator/(const FVector4& A, float Val)
{
    return FVector4(A.X / Val, A.Y / Val, A.Z / Val, A.W / Val);
}

FVector4& operator*=(FVector4& A, const FVector4& B)
{
    A.X *= B.X;
    A.Y *= B.Y;
    A.Z *= B.Z;
    A.W *= B.W;

    return A;
}

FVector4& operator+=(FVector4& A, const FVector4& B)
{
    A.X += B.X;
    A.Y += B.Y;
    A.Z += B.Z;
    A.W += B.W;

    return A;
}

FVector4& operator-=(FVector4& A, const FVector4& B)
{
    A.X -= B.X;
    A.Y -= B.Y;
    A.Z -= B.Z;
    A.W -= B.W;

    return A;
}

FVector4& operator*=(FVector4& A, float Val)
{
    A.X *= Val;
    A.Y *= Val;
    A.Z *= Val;
    A.W *= Val;

    return A;
}

FVector4& operator/=(FVector4& A, float Val)
{
    A.X /= Val;
    A.Y /= Val;
    A.Z /= Val;
    A.W /= Val;

    return A;
}

FVector4 operator-(const FVector4& A)
{
    return FVector4(-A.X, -A.Y, -A.Z, -A.W);
}

FVector4 operator*(const FMatrix4& B, const FVector4& A)
{
    FVector4 Result;
    Result.X = A.X * B.Data[0].X + A.Y * B.Data[1].X + A.Z * B.Data[2].X + A.W * B.Data[3].X;
    Result.Y = A.X * B.Data[0].Y + A.Y * B.Data[1].Y + A.Z * B.Data[2].Y + A.W * B.Data[3].Y;
    Result.Z = A.X * B.Data[0].Z + A.Y * B.Data[1].Z + A.Z * B.Data[2].Z + A.W * B.Data[3].Z;
    Result.W = A.X * B.Data[0].Z + A.Y * B.Data[1].Z + A.Z * B.Data[2].Z + A.W * B.Data[3].W;

    return Result;
}

///****************************************************************
///FVector3 and it's operations
///****************************************************************

FVector3 FVector3::GetNormalized() const
{
    float L = std::sqrt(X * X + Y * Y + Z * Z);

    if (L <= 0.000001f)
    {
        throw std::runtime_error("Failed to normalize FVector3");
    }

    return FVector3(X / L, Y / L, Z / L);
}

FVector3& FVector3::Normalize()
{
    auto L = Length();
    X /= L;
    Y /= L;
    Z /= L;

    return *this;
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

FVector3& FVector3::SelfRotateY(float Angle)
{
    float CosAngle = std::cos(Angle);
    float SinAngle = std::sin(Angle);
    auto RotationMatrix = FMatrix3(CosAngle, 0, -SinAngle, 0, 1, 0, SinAngle, 0, CosAngle);
    *this = *this * RotationMatrix;
    return *this;
}

FVector3& FVector3::SelfRotateX(float Angle)
{
	float CosAngle = std::cos(Angle);
	float SinAngle = std::sin(Angle);
	auto RotationMatrix = FMatrix3(1, 0, 0, 0, CosAngle, SinAngle, 0, -SinAngle, CosAngle);
	*this = *this * RotationMatrix;
	return *this;
}

FVector3& FVector3::SelfRotateZ(float Angle)
{
	float CosAngle = std::cos(Angle);
	float SinAngle = std::sin(Angle);
	auto RotationMatrix = FMatrix3(CosAngle, SinAngle, 0, -SinAngle, CosAngle, 0, 0, 0, 1);
	*this = *this * RotationMatrix;
	return *this;
}

float FVector3::Length()
{
    return sqrt(X * X + Y * Y + Z * Z);
}

float FVector3::Length2()
{
    return X * X + Y * Y + Z * Z;
}

std::string FVector3::ToString()
{
    std::string Result = "FVector3(";
    Result += std::to_string(X) + ", ";
    Result += std::to_string(Y) + ", ";
    Result += std::to_string(Z) + ")";

    return Result;
}

bool operator==(const FVector3& A, const FVector3& B)
{
    return (A.X == B.X && A.Y == B.Y && A.Z == B.Z);
}

FVector3 operator*(const FVector3& A, const FVector3& B)
{
    return FVector3(A.X * B.X, A.Y * B.Y, A.Z * B.Z);
}

FVector3 operator+(const FVector3& A, const FVector3& B)
{
    return FVector3(A.X + B.X, A.Y + B.Y, A.Z + B.Z);
}

FVector3 operator-(const FVector3& A, const FVector3& B)
{
    return FVector3(A.X - B.X, A.Y - B.Y, A.Z - B.Z);
}

FVector3 operator*(const FVector3& A, float Val)
{
    return {A.X * Val, A.Y * Val, A.Z * Val};
}

FVector3 operator/(const FVector3& A, float Val)
{
    return FVector3(A.X / Val, A.Y / Val, A.Z / Val);
}

FVector3& operator*=(FVector3& A, const FVector3& B)
{
    A.X *= B.X;
    A.Y *= B.Y;
    A.Z *= B.Z;

    return A;
}

FVector3& operator+=(FVector3& A, const FVector3& B)
{
    A.X += B.X;
    A.Y += B.Y;
    A.Z += B.Z;

    return A;
}

FVector3& operator-=(FVector3& A, const FVector3& B)
{
    A.X -= B.X;
    A.Y -= B.Y;
    A.Z -= B.Z;

    return A;
}

FVector3& operator*=(FVector3& A, float Val)
{
    A.X *= Val;
    A.Y *= Val;
    A.Z *= Val;

    return A;
}

FVector3& operator/=(FVector3& A, float Val)
{
    A.X /= Val;
    A.Y /= Val;
    A.Z /= Val;

    return A;
}

FVector3 operator-(const FVector3& A)
{
    return {-A.X, -A.Y, -A.Z};
}

FVector3 operator*(const FVector3& A, const FMatrix3& B)
{
    FVector3 Result{};
    Result.X = A.X * B.Data[0].X + A.Y * B.Data[1].X + A.Z * B.Data[2].X;
    Result.Y = A.X * B.Data[0].Y + A.Y * B.Data[1].Y + A.Z * B.Data[2].Y;
    Result.Z = A.X * B.Data[0].Z + A.Y * B.Data[1].Z + A.Z * B.Data[2].Z;

    return Result;
}

///****************************************************************
///FVector2 and it's operations
///****************************************************************

bool operator==(const FVector2& A, const FVector2& B)
{
    return (A.X == B.X && A.Y == B.Y);
}

FVector2 operator+(const FVector2& A, const FVector2& B)
{
    return {A.X + B.X, A.Y + B.Y};
}

FVector2 operator-(const FVector2& A, const FVector2& B)
{
	return {A.X - B.X, A.Y - B.Y};
}

FVector2 operator*(const FVector2& A, float Val)
{
	return {A.X * Val, A.Y * Val};
}

///****************************************************************
///FMatrix3 and it's operations
///****************************************************************

FMatrix3::FMatrix3(const FMatrix4& M) :
	Data({FVector3{M.Data[0].X, M.Data[0].Y, M.Data[0].Z},
		 FVector3{M.Data[1].X, M.Data[1].Y, M.Data[1].Z},
		 FVector3{M.Data[1].X, M.Data[2].Y, M.Data[2].Z}})
{
}

FMatrix3 FMatrix3::GetInverse()
{
	FMatrix3 Result{};

	float Determinant = Data[0].X * Data[1].Y * Data[2].Z +
		                Data[0].Y * Data[1].Z * Data[2].X +
		                Data[0].Z * Data[1].X * Data[2].Y +
		                Data[0].X * Data[1].Z * Data[2].Y -
		                Data[0].Y * Data[1].X * Data[2].Z -
		                Data[0].Z * Data[1].Y * Data[2].X;

	if (Determinant == 0.f)
	{
		throw std::runtime_error("You are about to divide by zero");
	}

	Result.Data[0].X = Data[0].X * Data[1].Y * Data[2].Z - Data[0].X * Data[1].Z * Data[2].Y;
	Result.Data[0].Y = Data[0].Y * Data[1].Z * Data[2].X - Data[0].Y * Data[1].X * Data[2].Z;
	Result.Data[0].Z = Data[0].Z * Data[1].X * Data[2].Y - Data[0].Z * Data[1].Y * Data[2].X;

	Result.Data[1].X = Data[1].X * Data[0].Y * Data[2].Z - Data[1].X * Data[0].Z * Data[2].Y;
	Result.Data[1].Y = Data[1].Y * Data[0].Z * Data[2].X - Data[1].Y * Data[0].X * Data[2].Z;
	Result.Data[1].Z = Data[1].Z * Data[0].X * Data[2].Y - Data[1].Z * Data[0].Y * Data[2].X;

	Result.Data[2].X = Data[2].X * Data[0].Y * Data[1].Z - Data[2].X * Data[0].Z * Data[1].Y;
	Result.Data[2].Y = Data[2].Y * Data[0].Z * Data[1].X - Data[2].Y * Data[0].X * Data[1].Z;
	Result.Data[2].Z = Data[2].Z * Data[0].X * Data[1].Y - Data[2].Z * Data[0].Y * Data[1].X;

	return Result / Determinant;
}

FMatrix3& FMatrix3::Transpose()
{
	float Tmp = Data[0].Y;
	Data[0].Y = Data[1].X;
	Data[1].X = Tmp;

	Tmp = Data[0].Z;
	Data[0].Z = Data[2].X;
	Data[2].X = Tmp;

	Tmp = Data[1].Z;
	Data[1].Z = Data[2].Y;
	Data[2].Y = Tmp;

	return *this;
}

FMatrix3 operator/(const FMatrix3& A, float Val)
{
	return {A.Data[0].X / Val, A.Data[0].Y / Val, A.Data[0].Z / Val, A.Data[1].X / Val, A.Data[1].Y / Val, A.Data[1].Z / Val, A.Data[2].X / Val, A.Data[2].Y / Val, A.Data[2].Z / Val};
}

///****************************************************************
///FMatrix4 and it's operations
///****************************************************************

FMatrix4 FMatrix4::GetInverse()
{
	FMatrix4 Result{};

	Result.Data[0].X =
		Data[1].Y * Data[2].Z * Data[3].W -
		Data[1].Y * Data[2].W * Data[3].Z -
		Data[2].Y * Data[1].Z * Data[3].W +
		Data[2].Y * Data[1].W * Data[3].Z +
		Data[3].Y * Data[1].Z * Data[2].W -
		Data[3].Y * Data[1].W * Data[2].Z;

	Result.Data[1].X =
	   -Data[1].X * Data[2].Z * Data[3].W +
		Data[1].X * Data[2].W * Data[3].Z +
		Data[2].X * Data[1].Z * Data[3].W -
		Data[2].X * Data[1].W * Data[3].Z -
		Data[3].X * Data[1].Z * Data[2].W +
		Data[3].X * Data[1].W * Data[2].Z;

	Result.Data[2].X =
		Data[1].X * Data[2].Y * Data[3].W -
		Data[1].X * Data[2].W * Data[3].Y -
		Data[2].X * Data[1].Y * Data[3].W +
		Data[2].X * Data[1].W * Data[3].Y +
		Data[3].X * Data[1].Y * Data[2].W -
		Data[3].X * Data[1].W * Data[2].Y;

	Result.Data[3].X =
	   -Data[1].X * Data[2].Y * Data[3].Z +
		Data[1].X * Data[2].Z * Data[3].Y +
		Data[2].X * Data[1].Y * Data[3].Z -
		Data[2].X * Data[1].Z * Data[3].Y -
		Data[3].X * Data[1].Y * Data[2].Z +
		Data[3].X * Data[1].Z * Data[2].Y;

	Result.Data[0].Y =
	   -Data[0].Y * Data[2].Z * Data[3].W +
		Data[0].Y * Data[2].W * Data[3].Z +
		Data[2].Y * Data[0].Z * Data[3].W -
		Data[2].Y * Data[0].W * Data[3].Z -
		Data[3].Y * Data[0].Z * Data[2].W +
		Data[3].Y * Data[0].W * Data[2].Z;

	Result.Data[1].Y =
		Data[0].X * Data[2].Z * Data[3].W -
		Data[0].X * Data[2].W * Data[3].Z -
		Data[2].X * Data[0].Z * Data[3].W +
		Data[2].X * Data[0].W * Data[3].Z +
		Data[3].X * Data[0].Z * Data[2].W -
		Data[3].X * Data[0].W * Data[2].Z;

	Result.Data[2].Y =
	   -Data[0].X * Data[2].Y * Data[3].W +
		Data[0].X * Data[2].W * Data[3].Y +
		Data[2].X * Data[0].Y * Data[3].W -
		Data[2].X * Data[0].W * Data[3].Y -
		Data[3].X * Data[0].Y * Data[2].W +
		Data[3].X * Data[0].W * Data[2].Y;

	Result.Data[3].Y =
		Data[0].X * Data[2].Y * Data[3].Z -
		Data[0].X * Data[2].Z * Data[3].Y -
		Data[2].X * Data[0].Y * Data[3].Z +
		Data[2].X * Data[0].Z * Data[3].Y +
		Data[3].X * Data[0].Y * Data[2].Z -
		Data[3].X * Data[0].Z * Data[2].Y;

	Result.Data[0].Z =
		Data[0].Y * Data[1].Z * Data[3].W -
		Data[0].Y * Data[1].W * Data[3].Z -
		Data[1].Y * Data[0].Z * Data[3].W +
		Data[1].Y * Data[0].W * Data[3].Z +
		Data[3].Y * Data[0].Z * Data[1].W -
		Data[3].Y * Data[0].W * Data[1].Z;

	Result.Data[1].Z =
	   -Data[0].X * Data[1].Z * Data[3].W +
		Data[0].X * Data[1].W * Data[3].Z +
		Data[1].X * Data[0].Z * Data[3].W -
		Data[1].X * Data[0].W * Data[3].Z -
		Data[3].X * Data[0].Z * Data[1].W +
		Data[3].X * Data[0].W * Data[1].Z;

	Result.Data[2].Z =
		Data[0].X * Data[1].Y * Data[3].W -
		Data[0].X * Data[1].W * Data[3].Y -
		Data[1].X * Data[0].Y * Data[3].W +
		Data[1].X * Data[0].W * Data[3].Y +
		Data[3].X * Data[0].Y * Data[1].W -
		Data[3].X * Data[0].W * Data[1].Y;

	Result.Data[3].Z =
	   -Data[0].X * Data[1].Y * Data[3].Z +
		Data[0].X * Data[1].Z * Data[3].Y +
		Data[1].X * Data[0].Y * Data[3].Z -
		Data[1].X * Data[0].Z * Data[3].Y -
		Data[3].X * Data[0].Y * Data[1].Z +
		Data[3].X * Data[0].Z * Data[1].Y;

	Result.Data[0].W =
	   -Data[0].Y * Data[1].Z * Data[2].W +
		Data[0].Y * Data[1].W * Data[2].Z +
		Data[1].Y * Data[0].Z * Data[2].W -
		Data[1].Y * Data[0].W * Data[2].Z -
		Data[2].Y * Data[0].Z * Data[1].W +
		Data[2].Y * Data[0].W * Data[1].Z;

	Result.Data[1].W =
		Data[0].X * Data[1].Z * Data[2].W -
		Data[0].X * Data[1].W * Data[2].Z -
		Data[1].X * Data[0].Z * Data[2].W +
		Data[1].X * Data[0].W * Data[2].Z +
		Data[2].X * Data[0].Z * Data[1].W -
		Data[2].X * Data[0].W * Data[1].Z;

	Result.Data[2].W =
	   -Data[0].X * Data[1].Y * Data[2].W +
		Data[0].X * Data[1].W * Data[2].Y +
		Data[1].X * Data[0].Y * Data[2].W -
		Data[1].X * Data[0].W * Data[2].Y -
		Data[2].X * Data[0].Y * Data[1].W +
		Data[2].X * Data[0].W * Data[1].Y;

	Result.Data[3].W =
		Data[0].X * Data[1].Y * Data[2].Z -
		Data[0].X * Data[1].Z * Data[2].Y -
		Data[1].X * Data[0].Y * Data[2].Z +
		Data[1].X * Data[0].Z * Data[2].Y +
		Data[2].X * Data[0].Y * Data[1].Z -
		Data[2].X * Data[0].Z * Data[1].Y;

	float Determinant = Data[0].X * Result.Data[0].X + Data[0].Y * Result.Data[1].X + Data[0].Z * Result.Data[2].X + Data[0].W * Result.Data[3].X;

	if (Determinant == 0)
	{
		throw std::runtime_error("You are about to divide by zero");
	}

	Determinant = 1.f / Determinant;

	Result = Result * Determinant;

	return Result;

}

FMatrix4& FMatrix4::Transpose()
{
	float Tmp = Data[0].Y;
	Data[0].Y = Data[1].X;
	Data[1].X = Tmp;

	Tmp = Data[0].Z;
	Data[0].Z = Data[2].X;
	Data[2].X = Tmp;

	Tmp = Data[0].W;
	Data[0].W = Data[3].X;
	Data[3].X = Tmp;

	Tmp = Data[1].Z;
	Data[1].Z = Data[2].Y;
	Data[2].Y = Tmp;

	Tmp = Data[1].W;
	Data[1].W = Data[3].Y;
	Data[3].Y = Tmp;

	Tmp = Data[2].W;
	Data[2].W = Data[3].Z;
	Data[3].Z = Tmp;

	return *this;
}

FMatrix4 operator*(const FMatrix4& A, float Val)
{
	FMatrix4 Result = A;

	Result.Data[0].X *= Val;
	Result.Data[0].Y *= Val;
	Result.Data[0].Z *= Val;
	Result.Data[0].W *= Val;

	Result.Data[1].X *= Val;
	Result.Data[1].Y *= Val;
	Result.Data[1].Z *= Val;
	Result.Data[1].W *= Val;

	Result.Data[2].X *= Val;
	Result.Data[2].Y *= Val;
	Result.Data[2].Z *= Val;
	Result.Data[2].W *= Val;

	Result.Data[3].X *= Val;
	Result.Data[3].Y *= Val;
	Result.Data[3].Z *= Val;
	Result.Data[3].W *= Val;

	return Result;
}

FMatrix4 operator/(const FMatrix4& A, float Val)
{
	float InvVal = 1.f / Val;
	return A * InvVal;
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

float Dot(const FVector3& L, const FVector3& R)
{
    return L.X * R.X + L.Y * R.Y + L.Z * R.Z;
}

FVector3 Cross(const FVector3& A, const FVector3& B)
{
    return FVector3(A.Y * B.Z - B.Y * A.Z, A.Z * B.X - B.Z * A.X, A.X * B.Y - B.X * A.Y);
}

/// Suppose incoming vectors are normalized
/// Return matrix that rotate OriginalDirection to FinalDirection around axis perpendicular to them
FMatrix4 GetRotationMatrix(FVector3 OriginalDirection, FVector3 FinalDirection)
{
    auto A = Cross(OriginalDirection, FinalDirection);
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
	FVector3 F = Direction.GetNormalized();
	FVector3 R = Cross(Up, F).GetNormalized();
	FVector3 U = Cross(F, R);

	R *= Scale.X;
	U *= Scale.Y;
	F *= Scale.Z;

	FMatrix4 LocalToWorldMatrix = {
		R.X, U.X, F.X, Position.X,
		R.Y, U.Y, F.Y, Position.Y,
		R.Z, U.Z, F.Z, Position.Z,
		0, 0, 0, 1.f,};

	return LocalToWorldMatrix;
}

FMatrix4 LookAt(const FVector3& Eye, const FVector3& Point, const FVector3& Up)
{
    FVector3 F = (Point - Eye).GetNormalized();
    FVector3 R = Cross(F, Up).GetNormalized();
    FVector3 U = Cross(R, F).GetNormalized();

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

FVector3 normalize(FVector3 Vec)
{
	return Vec.GetNormalized();
}

FVector3 cross(FVector3 A, FVector3 B)
{
	return Cross(A, B);
}

float max(float A, float B)
{
	return (A > B) ? A : B;
}

FQuaternion QuatFromVectors(const FVector3& Right, const FVector3& Up, const FVector3& Front)
{
	FQuaternion Result;
	Result.W = 0.5f * sqrt(abs(1 + Right.X + Up.Y + Front.Z));

	Result.X = 0.5f * sqrt(abs(1 + Right.X - Up.Y - Front.Z));
	Result.X = (Front.Y - Up.Z) < 0 ? -Result.X : Result.X;

	Result.Y = 0.5f * sqrt(abs(1 - Right.X + Up.Y - Front.Z));
	Result.Y = (Right.Z - Front.X) < 0 ? -Result.Y : Result.Y;

	Result.Z = 0.5f * sqrt(abs(1 - Right.X - Up.Y + Front.Z));
	Result.Z = (Up.X - Right.Y) < 0 ? -Result.Z : Result.Z;

	return Result.GetNormalized();
}

uint32_t Log2(uint32_t Integer)
{
    uint32_t Power = 0;
    while (Integer >>= 1)
    {
        ++Power;
    }

    return Power;
}
