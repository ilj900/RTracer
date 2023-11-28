#ifndef COMMON_STRUCTURES_H
#define COMMON_STRUCTURES_H

#ifndef __cplusplus
#define FVector4 vec4
#define FVector3 vec3
#define FVector2 vec2
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

const uint32_t RENDERABLE_SELECTED_BIT = 1 << 5;
const uint32_t RENDERABLE_HAS_TEXTURE = 1 << 6;
const uint32_t RENDERABLE_IS_INDEXED = 1 << 7;

struct FRenderable
{
    FVector3 RenderableColor;
    float dummy_1;

    int RenderableIndex;
    uint32_t RenderablePropertyMask;
    uint32_t dummy_2;
    uint32_t dummy_3;

    uint64_t VertexBufferAddress;
    uint64_t IndexBufferAddress;
};

struct FVertex
{
    FVector3 Position;
    FVector3 Normal;
    FVector2 TexCoord;
};

#endif // COMMON_STRUCTURES_H
