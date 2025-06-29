#pragma once

#include <array>
#include <stdexcept>
#include <cmath>
#include <cassert>

#ifndef M_PI
#define M_PI 3.14159265359f
#endif

#ifndef M_PI_2
#define M_PI_2 (M_PI / 2.f)
#endif

#ifndef M_2_PI
#define M_2_PI (M_PI * 2.f)
#endif

#ifndef M_PI_4
#define M_PI_4 (M_PI / 4.f)
#endif

#ifndef M_RAD
#define M_RAD (180.f / M_PI)
#endif

///The selected coordinate system is right handed
///X positive direction goes right
///Y positive direction goes up
///Z positive direction goes "from the screen" to "you"
///Vector multiplication works as in glsl - componentwise


struct FMatrix3;
struct FMatrix4;

struct FQuaternion
{
    /// Constructors
    FQuaternion() = default;
    FQuaternion(float W, float X, float Y, float Z) :W(W), X(X), Y(Y), Z(Z) {};

    /// Functions
    FQuaternion GetConjugation();
    FQuaternion GetInverse();
	FQuaternion GetNormalized();

    /// Data
    float X = 0.f;
    float Y = 0.f;
    float Z = 0.f;
    float W = 0.f;
};

FQuaternion operator*(const FQuaternion& A, const FQuaternion& B);

struct FVector4
{
    /// Constructors
    FVector4() = default;
    FVector4(float X, float Y, float Z, float W) : X(X), Y(Y), Z(Z), W(W) {}

    float Length();
    float Length2();

    union
	{
		struct
		{
			float X;
			float Y;
			float Z;
			float W;
		};
		/// Aliases for glsl
		struct
		{
			float x;
			float y;
			float z;
			float w;
		};
	};
};

bool operator==(const FVector4& A, const FVector4& B);
FVector4 operator*(const FVector4& A, const FVector4& B);
FVector4 operator+(const FVector4& A, const FVector4& B);
FVector4 operator-(const FVector4& A, const FVector4& B);
FVector4 operator*(const FVector4& A, float Val);
FVector4 operator/(const FVector4& A, float Val);
FVector4& operator*=(FVector4& A, const FVector4& B);
FVector4& operator+=(FVector4& A, const FVector4& B);
FVector4& operator-=(FVector4& A, const FVector4& B);
FVector4& operator*=(FVector4& A, float Val);
FVector4& operator/=(FVector4& A, float Val);
FVector4 operator-(const FVector4& A);
FVector4 operator*(const FMatrix4& B, const FVector4& A);

struct FVector3
{
    /// Constructors
    FVector3() = default;
    FVector3(float X, float Y, float Z): X(X), Y(Y), Z(Z) {}

    /// Functions
    FVector3 GetNormalized() const;
    FVector3& Normalize();
    FVector3 Rotate(float Angle, const FVector3& Axis);
	FVector3& SelfRotateX(float Angle);
    FVector3& SelfRotateY(float Angle);
	FVector3& SelfRotateZ(float Angle);
    float Length();
    float Length2();
    std::string ToString();

    /// Data
	union
	{
		struct
		{
			float X;
			float Y;
			float Z;
		};
		/// Aliases for glsl
		struct
		{
			float x;
			float y;
			float z;
		};
	};
};

bool operator==(const FVector3& A, const FVector3& B);
FVector3 operator*(const FVector3& A, const FVector3& B);
FVector3 operator+(const FVector3& A, const FVector3& B);
FVector3 operator-(const FVector3& A, const FVector3& B);
FVector3 operator*(const FVector3& A, float Val);
FVector3 operator/(const FVector3& A, float Val);
FVector3& operator*=(FVector3& A, const FVector3& B);
FVector3& operator+=(FVector3& A, const FVector3& B);
FVector3& operator-=(FVector3& A, const FVector3& B);
FVector3& operator*=(FVector3& A, float Val);
FVector3& operator/=(FVector3& A, float Val);
FVector3 operator-(const FVector3& A);
FVector3 operator*(const FVector3& A, const FMatrix3& B);

struct FVector2
{
public:
    /// Constructors
    FVector2() = default;
    FVector2(float X, float Y) : X(X), Y(Y) {};

    /// Data
	/// Data
	union
	{
		struct
		{
			float X;
			float Y;
		};
		/// Aliases for glsl
		struct
		{
			float x;
			float y;
		};
	};
};

bool operator==(const FVector2& A, const FVector2& B);
FVector2 operator+(const FVector2& A, const FVector2& B);
FVector2 operator-(const FVector2& A, const FVector2& B);
FVector2 operator*(const FVector2& A, float Val);

struct FMatrix3
{
    /// Constructors
    FMatrix3() : Data({FVector3{1.f, 0.f, 0.f}, FVector3{0.f, 1.f, 0.f}, FVector3{0.f, 0.f, 1.f}}){};
    FMatrix3(float A00, float A01, float A02,
             float A10, float A11, float A12,
             float A20, float A21, float A22) :
             Data({FVector3{A00, A01, A02}, FVector3{A10, A11, A12}, FVector3{A20, A21, A22}}) {};
	FMatrix3(const FMatrix4& M);

	FMatrix3 GetInverse();
	FMatrix3& Transpose();
    /// Data
    std::array<FVector3, 3> Data;
};

FMatrix3 operator/(const FMatrix3& A, float Val);

struct FMatrix4
{
    /// Constructors
    FMatrix4() : Data({FVector4{1.f, 0.f, 0.f, 0.f}, FVector4{0.f, 1.f, 0.f, 0.f}, FVector4{0.f, 0.f, 1.f, 0.f}, FVector4{0.f, 0.f, 0.f, 1.f}}){};
    FMatrix4(float A00, float A01, float A02, float A03,
             float A10, float A11, float A12, float A13,
             float A20, float A21, float A22, float A23,
             float A30, float A31, float A32, float A33) :
             Data({FVector4{A00, A01, A02, A03}, FVector4{A10, A11, A12, A13}, FVector4{A20, A21, A22, A23}, FVector4{A30, A31, A32, A33}}) {};
    FMatrix4(FVector4& Vec1, FVector4& Vec2, FVector4& Vec3, FVector4& Vec4) :
             Data({Vec1, Vec2, Vec3, Vec4}) {};
	FMatrix4(const FMatrix3& M) :
 			Data({FVector4{M.Data[0].X, M.Data[0].Y, M.Data[0].Z, 0}, FVector4{M.Data[1].X, M.Data[1].Y, M.Data[1].Z, 0}, FVector4{M.Data[2].X, M.Data[2].Y, M.Data[2].Z, 0}, FVector4{0, 0, 0, 1}}) {};

	FMatrix4 GetInverse();
	FMatrix4& Transpose();

    /// Data
    std::array<FVector4, 4> Data;
};

FMatrix4 operator*(const FMatrix4& A, float Val);
FMatrix4 operator/(const FMatrix4& A, float Val);
FMatrix4 operator*(const FMatrix4& A, const FMatrix4& B);
FVector3 operator*(const FMatrix4& A, const FVector3& B);

///Other functions
float Dot(const FVector3& L, const FVector3& R);
FVector3 Cross(const FVector3& A, const FVector3& B);
FMatrix4 GetRotationMatrix(FVector3 OriginalDirection, FVector3 FinalDirection);
FMatrix4 GetRotationMatrixX(float Angle);
FMatrix4 GetRotationMatrixY(float Angle);
FMatrix4 GetRotationMatrixZ(float Angle);
FMatrix4 Transform(const FVector3& Position, const FVector3& Direction, const FVector3& Up, const FVector3& Scale);
FMatrix4 LookAt(const FVector3& Eye, const FVector3& Point, const FVector3& Up);
FMatrix4 GetPerspective(float FOV, float AspectRatio, float NearDistance, float FarDistance);

FVector3 normalize(FVector3 Vec);
float dot(const FVector3& L, const FVector3& R);
FVector3 cross(FVector3 A, FVector3 B);
float clamp(float Val, float Min, float Max);
float max(float A, float B);

FQuaternion QuatFromVectors(const FVector3& Right, const FVector3& Up, const FVector3& Front);

/// Template specializations for hashing functions
template<>
struct std::hash<FVector2>
{
    size_t operator()(const FVector2& Vector)
    {
        size_t hash = 0;
        hash = std::hash<float>()(Vector.X) ^ ((std::hash<float>()(Vector.Y) << 1) >> 1);

        return hash;
    }
};

template<>
struct std::hash<FVector3>
{
    size_t operator()(FVector3 const &Vector)
    {
        size_t hash = 0;
        hash = std::hash<float>()(Vector.X) ^ ((std::hash<float>()(Vector.Y) << 1) >> 1) ^ (std::hash<float>()(Vector.Z) << 1);

        return hash;
    }
};

uint32_t Log2(uint32_t Integer);
