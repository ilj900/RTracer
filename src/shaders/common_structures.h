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

struct FSimpleHitPayload
{
	uint32_t PayloadFlags;
};

struct FRayData
{
    FVector4 Origin;
    FVector4 Direction;
    uint32_t RayFlags;
    float TMin;
    float TMax;
    float Eta;
};

struct FDeviceCamera
{
    FVector3 Origin;
    float FocalDistance;

	FVector3 Direction;
	float SensorSizeX;

	FVector3 Right;
	float SensorSizeY;

	FVector3 Up;
	float Dummy;
};

struct FDeviceTransform
{
	FMatrix4 ModelMatrix;
	FMatrix4 InverseModelMatrix;
};

struct FPointLight
{
	FVector3 Position;
	float Intensity;

	FVector3 Color;
	float Power;
};

struct FDirectionalLight
{
	FVector3 Position;
	float Intensity;

	FVector3 Color;
	float Power;

	FVector3 Direction;
	float Dummy2;
};

struct FSpotLight
{
	FVector3 Position;
	float Intensity;

	FVector3 Color;
	float InnerAngle;

	FVector3 Direction;
	float OuterAngle;

	float Power;
	float Dummy1;
	float Dummy2;
	float Dummy3;
};

struct FAreaLight
{
	uint32_t IsIndexedFlagAndRenderableIndex;
	uint32_t NumberOfTriangles;
	uint32_t TransformIndex;
	uint32_t MaterialIndex;

	uint64_t VertexBufferAddress;
	uint64_t IndexBufferAddress;

	uint64_t AliasTableBufferAddress;
	float Area;
	float Dummy3;
};

struct FUtilityData
{
	uint32_t ActiveDirectionalLightsCount;
	float TotalDirectionalLightPower;
	uint32_t ActiveSpotLightsCount;
	float TotalSpotLightPower;
	uint32_t ActivePointLightsCount;
	float TotalPointLightPower;
	uint32_t ActiveAreaLightsCount;
	float TotalAreaLightArea;
	uint32_t AOVIndex;
	uint32_t AccumulateFrames;
	uint32_t AccumulateBounces;
	uint32_t Dummy;
};

/// Should be aligned with FAliasTableEntry
struct FDeviceAliasTableEntry
{
	float Threshold;
	uint32_t Alias;
};

struct FDeviceMaterial
{
    FVector3 BaseColor;
    float BaseWeight;
	///--------------------------------
    FVector3 Normal;
    float DiffuseRoughness;
	///--------------------------------
    float Metalness;

    float SpecularWeight;
    float SpecularRoughness;
    float SpecularIOR;
	///--------------------------------
    FVector3 SpecularColor;
    float SpecularAnisotropy;
	///--------------------------------
    float SpecularRotation;

    float TransmissionWeight;
    float TransmissionDepth;
    float TransmissionAnisotropy;
	///--------------------------------
    FVector3 TransmissionColor;
    float TransmissionDispersion;
	///--------------------------------
    FVector3 TransmissionScatter;
    float TransmissionRoughness;

    FVector3 SubsurfaceColor;
    float SubsurfaceWeight;
	///--------------------------------
    FVector3 SubsurfaceRadius;
    float SubsurfaceScale;
	///--------------------------------
    float SubsurfaceAnisotropy;

    float SheenWeight;
    float SheenRoughness;
    float CoatWeight;
	///--------------------------------
    FVector3 SheenColor;
    float CoatRoughness;
	///--------------------------------

    FVector3 CoatColor;
    float CoatAnisotropy;
	///--------------------------------
    FVector3 CoatNormal;
    float CoatRotation;
	///--------------------------------
    float CoatIOR;
    float CoatAffectColor;
    float CoatAffectRoughness;

    float ThinFilmThickness;
	///--------------------------------
    FVector3 EmissionColor;
    float ThinFilmIOR;
	///--------------------------------
    FVector3 Opacity;
    float EmissionWeight;

	///--------------------------------
    uint32_t ThinWalled;
	float Dummy1;
	float Dummy2;
	float Dummy3;

	///--------------------------------
	uint32_t BaseWeightTexture;
    uint32_t BaseColorTexture;
    uint32_t DiffuseRoughnessTexture;
    uint32_t MetalnessTexture;
    ///--------------------------------
    uint32_t NormalTexture;

    uint32_t SpecularWeightTexture;
    uint32_t SpecularColorTexture;
    uint32_t SpecularRoughnessTexture;
    ///--------------------------------
    uint32_t SpecularIORTexture;
    uint32_t SpecularAnisotropyTexture;
    uint32_t SpecularRotationTexture;

    uint32_t TransmissionWeightTexture;
    ///--------------------------------
    uint32_t TransmissionColorTexture;
    uint32_t TransmissionDepthTexture;
    uint32_t TransmissionScatterTexture;
    uint32_t TransmissionAnisotropyTexture;
    ///--------------------------------
    uint32_t TransmissionDispersionTexture;
    uint32_t TransmissionRoughnessTexture;

    uint32_t SubsurfaceWeightTexture;
    uint32_t SubsurfaceColorTexture;
    ///--------------------------------
    uint32_t SubsurfaceRadiusTexture;
    uint32_t SubsurfaceScaleTexture;
    uint32_t SubsurfaceAnisotropyTexture;

    uint32_t SheenWeightTexture;
    ///--------------------------------
    uint32_t SheenColorTexture;
    uint32_t SheenRoughnessTexture;

    uint32_t CoatWeightTexture;
    uint32_t CoatColorTexture;
    ///--------------------------------
    uint32_t CoatRoughnessTexture;
    uint32_t CoatAnisotropyTexture;
    uint32_t CoatRotationTexture;
    uint32_t CoatIORTexture;
    ///--------------------------------
    uint32_t CoatNormalTexture;
    uint32_t CoatAffectColorTexture;
    uint32_t CoatAffectRoughnessTexture;

    uint32_t ThinFilmThicknessTexture;
    ///--------------------------------
    uint32_t ThinFilmIORTexture;

    uint32_t EmissionWeightTexture;
    uint32_t EmissionColorTexture;

    uint32_t OpacityTexture;
    ///--------------------------------
    uint32_t ThinWalledTexture;
    uint32_t Dummy4;
    uint32_t Dummy5;
    uint32_t Dummy6;
};

struct FOpenPBRSurface
{
    // Base
    float BaseWeight;
    FVector3 BaseColor;
    float BaseMetalness;
    float BaseDiffuseRoughness;

    // Specular
    float SpecularWeight;
    FVector3 SpecularColor;
    float SpecularRoughness;
    float SpecularRoughnessAnisotropy;
    float SpecularIor;

    // Transmission
    float TransmissionWeight;
    FVector3 TransmissionColor;
    float TransmissionDepth;
    FVector3 TransmissionScatter;
    float TransmissionScatterAnisotropy;
    float TransmissionDispersionScale;
    float TransmissionDispersionAbbeNumber;

    // Subsurface
    float SubsurfaceWeight;
    FVector3 SubsurfaceColor;
    float SubsurfaceRadius;
    FVector3 SubsurfaceRadiusScale;
    float SubsurfaceScatterAnisotropy;

    // Coat
    float CoatWeight;
    FVector3 CoatColor;
    float CoatRoughness;
    float CoatRoughnessAnisotropy;
    float CoatIor;
    float CoatDarkening;

    // Fuzz
    float FuzzWeight;
    FVector3 FuzzColor;
    float FuzzRoughness;

    // Emission
    float EmissionLuminance;
    FVector3 EmissionColor;

    // Thin film
    float ThinFilmWeight;
    float ThinFilmThickness;
    float ThinFilmIor;

    // Geometry
    float GeometryOpacity;
    uint32_t GeometryThinWalled;
    FVector3 GeometryNormal;
    FVector3 GeometryTangent;
    FVector3 GeometryCoatNormal;
    FVector3 GeometryCoatTangent;

	// Base
	uint32_t BaseWeightTextureIndex;
	uint32_t BaseColorTextureIndex;
	uint32_t BaseMetalnessTextureIndex;
	uint32_t BaseDiffuseRoughnessTextureIndex;

	// Specular
	uint32_t SpecularWeightTextureIndex;
	uint32_t SpecularColorTextureIndex;
	uint32_t SpecularRoughnessTextureIndex;
	uint32_t SpecularRoughnessAnisotropyTextureIndex;
	uint32_t SpecularIorTextureIndex;

	// Transmission
	uint32_t TransmissionWeightTextureIndex;
	uint32_t TransmissionColorTextureIndex;
	uint32_t TransmissionDepthTextureIndex;
	uint32_t TransmissionScatterTextureIndex;
	uint32_t TransmissionScatterAnisotropyTextureIndex;
	uint32_t TransmissionDispersionScaleTextureIndex;
	uint32_t TransmissionDispersionAbbeNumberTextureIndex;

	// Subsurface
	uint32_t SubsurfaceWeightTextureIndex;
	uint32_t SubsurfaceColorTextureIndex;
	uint32_t SubsurfaceRadiusTextureIndex;
	uint32_t SubsurfaceRadiusScaleTextureIndex;
	uint32_t SubsurfaceScatterAnisotropyTextureIndex;

	// Coat
	uint32_t CoatWeightTextureIndex;
	uint32_t CoatColorTextureIndex;
	uint32_t CoatRoughnessTextureIndex;
	uint32_t CoatRoughnessAnisotropyTextureIndex;
	uint32_t CoatIorTextureIndex;
	uint32_t CoatDarkeningTextureIndex;

	// Fuzz
	uint32_t FuzzWeightTextureIndex;
	uint32_t FuzzColorTextureIndex;
	uint32_t FuzzRoughnessTextureIndex;

	// Emission
	uint32_t EmissionLuminanceTextureIndex;
	uint32_t EmissionColorTextureIndex;

	// Thin film
	uint32_t ThinFilmWeightTextureIndex;
	uint32_t ThinFilmThicknessTextureIndex;
	uint32_t ThinFilmIorTextureIndex;

	// Geometry
	uint32_t GeometryOpacityTextureIndex;
	uint32_t GeometryThinWalledTextureIndex;
	uint32_t GeometryNormalTextureIndex;
	uint32_t GeometryTangentTextureIndex;
	uint32_t GeometryCoatNormalTextureIndex;
	uint32_t GeometryCoatTangentTextureIndex;
};

const uint32_t RENDERABLE_AREA_LIGHT_INDEX_MASK = 0x01FF; // It's related to maximum number of area lights (512) in Area lights system
const uint32_t RENDERABLE_SELECTED_BIT = 1 << 9;
const uint32_t RENDERABLE_HAS_TEXTURE = 1 << 10;
const uint32_t RENDERABLE_IS_INDEXED = 1 << 11;

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

struct FViewportResolutionPushConstants
{
	uint32_t Width;
	uint32_t Height;
	uint32_t BounceIndex;
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
