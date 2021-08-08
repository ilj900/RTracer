#pragma once

#include <array>
#include <stdexcept>
#include <cmath>

struct FVector4
{
public:
    FVector4(float X, float Y, float Z, float W) : X(X), Y(Y), Z(Z), W(W) {}
    FVector4() = default;

    friend bool operator==(const FVector4& A, const FVector4& B);

    float X;
    float Y;
    float Z;
    float W;
};

struct FVector3
{
public:
    FVector3(float X, float Y, float Z): X(X), Y(Y), Z(Z) {}
    FVector3() = default;

    FVector3 GetNormalized() const;

    friend bool operator==(const FVector3& A, const FVector3& B);
    friend FVector3 operator*(const FVector3& A, const FVector3& B);
    friend FVector3 operator-(const FVector3& A, const FVector3& B);

    float X;
    float Y;
    float Z;
};

struct FVector2
{
public:
    FVector2(float X, float Y) : X(X), Y(Y) {};
    FVector2() = default;

    friend bool operator==(const FVector2& A, const FVector2& B);

    float X;
    float Y;
};

struct FMatrix3
{
public:
    FMatrix3() : Data({FVector3{1.f, 0.f, 0.f}, FVector3{0.f, 1.f, 0.f}, FVector3{0.f, 0.f, 1.f}}){}

    std::array<FVector3, 3> Data;
};

struct FMatrix4 {
public:
    FMatrix4() : Data({FVector4{1.f, 0.f, 0.f, 0.f}, FVector4{0.f, 1.f, 0.f, 0.f}, FVector4{0.f, 0.f, 1.f, 0.f}, FVector4{0.f, 0.f, 0.f, 1.f}}){}

    std::array<FVector4, 4> Data;
};

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

FMatrix4 Rotate(float Angle, const FVector3& Axis);

FMatrix4 LookAt(const FVector3& Eye, const FVector3& Point, const FVector3& Up);

FMatrix4 GetPerspective(float FOV, float AspectRatio, float NearDistance, float FarDistance);
