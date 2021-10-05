#pragma once

#include <array>
#include <stdexcept>
#include <cmath>
#include <cassert>

struct FQuaternion
{
    /// Constructors
    FQuaternion() = default;
    FQuaternion(float W, float X, float Y, float Z) :W(W), X(X), Y(Y), Z(Z) {};

    /// Functions
    FQuaternion GetConjugation();
    FQuaternion GetInverse();

    /// Operators
    friend FQuaternion operator*(const FQuaternion& A, const FQuaternion& B);

    /// Data
    float X = 0.f;
    float Y = 0.f;
    float Z = 0.f;
    float W = 0.f;
};

struct FVector4
{
    /// Constructors
    FVector4() = default;
    FVector4(float X, float Y, float Z, float W) : X(X), Y(Y), Z(Z), W(W) {}

    /// Operators
    friend bool operator==(const FVector4& A, const FVector4& B);

    /// Data
    float X = 0.f;
    float Y = 0.f;
    float Z = 0.f;
    float W = 0.f;
};

struct FVector3
{
    /// Constructors
    FVector3(float X, float Y, float Z): X(X), Y(Y), Z(Z) {}
    FVector3() = default;

    /// Functions
    FVector3 GetNormalized() const;
    FVector3 Rotate(float Angle, const FVector3& Axis);

    /// Operators
    friend bool operator==(const FVector3& A, const FVector3& B);
    friend FVector3 operator*(const FVector3& A, const FVector3& B);
    friend FVector3 operator+(const FVector3& A, const FVector3& B);
    friend FVector3 operator*(const FVector3& A, float Val);
    friend FVector3& operator+=(FVector3& A, const FVector3& B);
    friend FVector3 operator-(const FVector3& A, const FVector3& B);

    /// Data
    float X;
    float Y;
    float Z;
};

struct FVector2
{
public:
    /// Constructors
    FVector2() = default;
    FVector2(float X, float Y) : X(X), Y(Y) {};

    /// Operators
    friend bool operator==(const FVector2& A, const FVector2& B);

    /// Data
    float X;
    float Y;
};

struct FMatrix3
{
    /// Constructors
    FMatrix3() : Data({FVector3{1.f, 0.f, 0.f}, FVector3{0.f, 1.f, 0.f}, FVector3{0.f, 0.f, 1.f}}){}

    /// Data
    std::array<FVector3, 3> Data;
};

struct FMatrix4
{
    /// Constructors
    FMatrix4() : Data({FVector4{1.f, 0.f, 0.f, 0.f}, FVector4{0.f, 1.f, 0.f, 0.f}, FVector4{0.f, 0.f, 1.f, 0.f}, FVector4{0.f, 0.f, 0.f, 1.f}}){}
    FMatrix4(float A00, float A01, float A02, float A03,
             float A10, float A11, float A12, float A13,
             float A20, float A21, float A22, float A23,
             float A30, float A31, float A32, float A33) :
             Data({FVector4{A00, A01, A02, A03}, FVector4{A10, A11, A12, A13}, FVector4{A20, A21, A22, A23}, FVector4{A30, A31, A32, A33}}) {};
    FMatrix4(FVector4& Vec1, FVector4& Vec2, FVector4& Vec3, FVector4& Vec4) :
             Data({Vec1, Vec2, Vec3, Vec4}) {};
    /// Data
    std::array<FVector4, 4> Data;
};

/// Template specializations
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

/// Some math functions
FMatrix4 Transform(const FVector3& Position, const FVector3& Direction, const FVector3& Up);
FMatrix4 LookAt(const FVector3& Eye, const FVector3& Point, const FVector3& Up);
FMatrix4 GetPerspective(float FOV, float AspectRatio, float NearDistance, float FarDistance);
