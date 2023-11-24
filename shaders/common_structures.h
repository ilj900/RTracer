#pragma once

#ifndef __cplusplus
#define FVector4 vec4
#define FVector3 vec3
#define FMatrix4 mat4
#define uint32_t uint
#endif

struct HitPayload
{
    FVector3 Color;
    FVector3 Direction;
};

struct FRayData
{
    FVector4 Origin;
    FVector4 Direction;
    uint32_t RayFlags;
    float TMin;
    float TMax;
    float Dummy;
};

struct FCamera
{
    FMatrix4 ViewMatrix;
    FMatrix4 ProjectionMatrix;
};

#define LIGHT_TYPE_NONE 0
#define LIGHT_TYPE_POINT_LIGHT 1
#define LIGHT_TYPE_DIRECTIONAL_LIGHT 2
#define LIGHT_TYPE_SPOT_LIGHT 3

struct FLight
{
    FVector3 Position;
    float Intensity;

    FVector3 Color;
    uint32_t Type;

    FVector3 Direction;
    float OuterAngle;

    float InnerAngle;
    float dummy_1;
    float dummy_2;
    float dummy_3;
};

struct FMaterial
{
    FVector3 BaseAlbedo;
    float ReflectionRoughness;

    FVector3 ReflectionAlbedo;
    float RefractionRoughness;

    FVector3 CoatingAlbedo;
    float ReflectionIOR;

    FVector3 RefractionAlbedo;
    float RefractionIOR;
};