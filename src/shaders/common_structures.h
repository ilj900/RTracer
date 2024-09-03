#ifndef COMMON_STRUCTURES_H
#define COMMON_STRUCTURES_H

#ifndef __cplusplus
#define FVector4 vec4
#define FVector3 vec3
#define FVector2 vec2
#define FMatrix4 mat4
#define uint32_t uint
#endif

struct FHit
{
    uint32_t RenderableIndex;
    uint32_t PrimitiveIndex;
    FVector2 HitUV;
};

struct FHitPayload
{
    uint32_t RenderableIndex;
    uint32_t PrimitiveIndex;
    FVector2 HitUV;
    uint32_t MaterialIndex;
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

struct FDeviceCamera
{
    FMatrix4 ViewMatrix;
	FMatrix4 InverseViewMatrix;
    FMatrix4 ProjectionMatrix;
	FMatrix4 InverseProjectionMatrix;
    FVector3 Origin;
    float ZNear;
    FVector3 ViewDirection;
    float ZFar;
    float FOV;
    float dummy_1;
    float dummy_2;
    float dummy_3;
};

struct FDeviceTransform
{
	FMatrix4 ModelMatrix;
	FMatrix4 InverseModelMatrix;
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

struct FDeviceMaterial
{
    float BaseWeight;
    FVector3 BaseColor;
    float DiffuseRoughness;
    float Metalness;
    FVector3 Normal;

    float SpecularWeight;
    FVector3 SpecularColor;
    float SpecularRoughness;
    float SpecularIOR;
    float SpecularAnisotropy;
    float SpecularRotation;

    float TransmissionWeight;
    FVector3 TransmissionColor;
    float TransmissionDepth;
    FVector3 TransmissionScatter;
    float TransmissionAnisotropy;
    float TransmissionDispersion;
    float TransmissionRoughness;

    float SubsurfaceWeight;
    FVector3 SubsurfaceColor;
    FVector3 SubsurfaceRadius;
    float SubsurfaceScale;
    float SubsurfaceAnisotropy;

    float SheenWeight;
    FVector3 SheenColor;
    float SheenRoughness;

    float CoatWeight;
    FVector3 CoatColor;
    float CoatRoughness;
    float CoatAnisotropy;
    float CoatRotation;
    float CoatIOR;
    FVector3 CoatNormal;
    float CoatAffectColor;
    float CoatAffectRoughness;

    float ThinFilmThickness;
    float ThinFilmIOR;

    float Emission;
    FVector3 EmissionColor;

    FVector3 Opacity;
    uint32_t ThinWalled;
};

const uint32_t RENDERABLE_SELECTED_BIT = 1 << 5;
const uint32_t RENDERABLE_HAS_TEXTURE = 1 << 6;
const uint32_t RENDERABLE_IS_INDEXED = 1 << 7;

struct FRenderable
{
    FVector3 RenderableColor;
    uint32_t MaterialIndex;

    int RenderableIndex;
    uint32_t RenderablePropertyMask;
    uint32_t TransformIndex;
    uint32_t MeshIndex;

    uint64_t VertexBufferAddress;
    uint64_t IndexBufferAddress;
};

struct FVertex
{
    FVector3 Position;
    FVector3 Normal;
    FVector2 TexCoord;
};

struct FDeviceVertex
{
	FVector4 A;
	FVector4 B;
};

struct FPushConstants
{
    uint32_t Width;
    uint32_t Height;
    float InvWidth;
    float InvHeight;
    uint32_t TotalSize;
    uint32_t MaterialIndex;
	uint32_t BounceIndex;
};

struct FPassthroughPushConstants
{
	uint32_t Width;
	uint32_t Height;
};

struct FPushConstantsCountMaterialsPerChunk
{
    uint32_t TotalSize;
    uint32_t GroupSize;
    uint32_t MaxGroupSize;
};

struct FPushConstantsOffsets
{
    uint32_t GroupIndex;
    uint32_t MaxGroupSize;
};

struct FPushConstantsPrefixSums
{
    uint32_t D;
    uint32_t TotalGroupCount;
};

#endif // COMMON_STRUCTURES_H
