#ifndef LIGHTING_H
#define LIGHTING_H

#include "common_defines.h"
#include "common_structures.h"
#include "process_material_interaction.h"
#include "random.h"

uint GetActiveLightCount(uint LightType)
{
    if (LightType == POINT_LIGHT)       return UtilityData.ActivePointLightsCount;
    if (LightType == DIRECTIONAL_LIGHT) return UtilityData.ActiveDirectionalLightsCount;
    if (LightType == SPOT_LIGHT)        return UtilityData.ActiveSpotLightsCount;
    return 0;
}

FDeviceAliasTableEntry GetImportanceEntry(uint LightType, uint LightIndex)
{
    if (LightType == POINT_LIGHT)       return PointLightsImportanceBuffer[LightIndex];
    if (LightType == DIRECTIONAL_LIGHT) return DirectionalLightsImportanceBuffer[LightIndex];
    if (LightType == SPOT_LIGHT)        return SpotLightsImportanceBuffer[LightIndex];
    FDeviceAliasTableEntry Empty = FDeviceAliasTableEntry(0.0, 0u); // adjust to your struct
    return Empty;
}

uint GetLightIndex(inout FSamplingState SamplingState, uint SamplingStrategy, uint LightType)
{
    uint Count = GetActiveLightCount(LightType);
     if (Count == 0) return UINT_MAX;

    uint LightIndex = uint(RandomFloat(SamplingState) * Count);

    if (SamplingStrategy == SAMPLE_IMPORTANCE)
    {
        FDeviceAliasTableEntry Entry = GetImportanceEntry(LightType, LightIndex);

        if (RandomFloat(SamplingState) > Entry.Threshold)
            LightIndex = Entry.Alias;
    }

    return LightIndex;
}

/// Selects a point light (uniformly or by power-weighted alias table)
/// Traces a shadow ray toward it, and
/// If unoccluded, returns rgb = color * intensity / dist² * NdotL / PDF, w = PDF.
/// Always writes both UniformSamplingPDF and ImportanceSamplingPDF for MIS.
vec4 ComputePointLightInput(inout FSamplingState SamplingState, out vec3 Direction, uint SamplingStrategy, inout float OtherSamplingPDF)
{
    /// If not Importance or Uniform sampling, return indicating color
    if (SamplingStrategy != SAMPLE_IMPORTANCE && SamplingStrategy != SAMPLE_UNIFORM)
        return vec4(1, 0, 1, 1);

    uint LightIndex = GetLightIndex(SamplingState, SamplingStrategy, POINT_LIGHT);

    FPointLight PointLight = PointLightsBuffer[LightIndex];
    Direction = PointLight.Position - ShadingData.IntersectionCoordinatesInWorldSpace;
    float LightDistance = length(Direction);
    Direction = normalize(Direction);

    /// Check whether light is over the surface
    float NDotI = dot(ShadingData.NormalInWorldSpace, Direction);

    if(NDotI <= 0)
    {
        return vec4(0);
    }

    /// Construct a ray that goes to the light
    FRayData RayData;
    RayData.RayFlags = 0;
    RayData.Direction.xyz = Direction;
    RayData.Origin.xyz = ShadingData.IntersectionCoordinatesInWorldSpace + ShadingData.NormalInWorldSpace * FLOAT_EPSILON;

    /// Trace the ray
    traceRayEXT(TLAS, gl_RayFlagsOpaqueEXT, 0xFF, 0, 0, 0, RayData.Origin.xyz, 0.000001f, RayData.Direction.xyz, LightDistance, 0);

    /// And if we didn't hit any geometry, then we store light data
    if (HitPayload.RenderableIndex == UINT_MAX)
    {
        float Attenuation = 1.f / LightDistance;
        Attenuation *= Attenuation;

        /// Get the probabilities
        float UniformSamplingPDF = 1.f / UtilityData.ActivePointLightsCount;
        float ImportanceSamplingPDF = PointLight.Power / UtilityData.TotalPointLightPower;

        float PDF = SamplingStrategy == SAMPLE_UNIFORM ? UniformSamplingPDF : ImportanceSamplingPDF;
        OtherSamplingPDF = SamplingStrategy == SAMPLE_UNIFORM ? ImportanceSamplingPDF : UniformSamplingPDF;

        return vec4(PointLight.Color * PointLight.Intensity * Attenuation * NDotI / PDF, PDF);
    }

    return vec4(0);
}

vec4 ComputeDirectionalLightInput(inout FSamplingState SamplingState, out vec3 Direction, uint SamplingStrategy, inout float OtherSamplingPDF)
{
    /// If not Importance or Uniform sampling, return indicating color
    if (SamplingStrategy != SAMPLE_IMPORTANCE && SamplingStrategy != SAMPLE_UNIFORM)
        return vec4(1, 0, 1, 1);

    uint LightIndex = GetLightIndex(SamplingState, SamplingStrategy, DIRECTIONAL_LIGHT);

    FDirectionalLight DirectionalLight = DirectionalLightsBuffer[LightIndex];
    Direction = -DirectionalLight.Direction;

    /// Check whether light is over the surface
    float NDotI = dot(ShadingData.NormalInWorldSpace, Direction);

    if(NDotI <= 0)
    {
        return vec4(0);
    }

    /// Construct a ray that goes to the light
    FRayData RayData;
    RayData.RayFlags = 0;
    RayData.Direction.xyz = Direction;
    RayData.Origin.xyz = ShadingData.IntersectionCoordinatesInWorldSpace + ShadingData.NormalInWorldSpace * FLOAT_EPSILON;

    /// Trace the ray
    traceRayEXT(TLAS, gl_RayFlagsOpaqueEXT, 0xFF, 0, 0, 0, RayData.Origin.xyz, 0.000001f, RayData.Direction.xyz, 10000, 0);

    /// And if we didn't hit any geometry, then we store light data
    if (HitPayload.RenderableIndex == UINT_MAX)
    {
        /// Here, the probability of sampling a particular directional light is it's power to the total power of all directional light
        float UniformSamplingPDF = 1.f / UtilityData.ActiveDirectionalLightsCount;
        float ImportanceSamplingPDF = DirectionalLight.Power / UtilityData.TotalDirectionalLightPower;

        float PDF = SamplingStrategy == SAMPLE_UNIFORM ? UniformSamplingPDF : ImportanceSamplingPDF;
        OtherSamplingPDF = SamplingStrategy == SAMPLE_UNIFORM ? ImportanceSamplingPDF : UniformSamplingPDF;

        return vec4(DirectionalLight.Color * DirectionalLight.Intensity * NDotI / PDF, PDF);
    }

    return vec4(0);
}

vec4 ComputeSpotLightInput(inout FSamplingState SamplingState, out vec3 Direction, uint SamplingStrategy, inout float OtherSamplingPDF)
{
    /// If not Importance or Uniform sampling, return indicating color
    if (SamplingStrategy != SAMPLE_IMPORTANCE && SamplingStrategy != SAMPLE_UNIFORM)
        return vec4(1, 0, 1, 1);

    uint LightIndex = GetLightIndex(SamplingState, SamplingStrategy, SPOT_LIGHT);

    FSpotLight SpotLight = SpotLightsBuffer[LightIndex];

    Direction = SpotLight.Position - ShadingData.IntersectionCoordinatesInWorldSpace;
    float LightDistance = length(Direction);
    Direction = normalize(Direction);

    /// Check whether light is over the surface
    float NDotI = dot(ShadingData.NormalInWorldSpace, Direction);

    if(NDotI <= 0)
    {
        return vec4(0);
    }

    float LightAngle = acos(dot(SpotLight.Direction, -Direction));

    /// If point is outside out outer angle, it is also not illuminated
    if (LightAngle > SpotLight.OuterAngle)
    {
        return vec4(0);
    }

    /// Construct a ray that goes to the light
    FRayData RayData;
    RayData.RayFlags = 0;
    RayData.Direction.xyz = Direction;
    RayData.Origin.xyz = ShadingData.IntersectionCoordinatesInWorldSpace + ShadingData.NormalInWorldSpace * FLOAT_EPSILON;

    /// Trace the ray
    traceRayEXT(TLAS, gl_RayFlagsOpaqueEXT, 0xFF, 0, 0, 0, RayData.Origin.xyz, 0.000001f, RayData.Direction.xyz, LightDistance, 0);

    /// And if we didn't hit any geometry, then we store light data
    if (HitPayload.RenderableIndex == UINT_MAX)
    {
        float Attenuation = 1.f / LightDistance;
        Attenuation *= Attenuation;

        /// If point between outer and inner angle - interpolate
        if (LightAngle > SpotLight.InnerAngle)
        {
            float Fraction = LightAngle - SpotLight.InnerAngle;
            float Delta = SpotLight.OuterAngle - SpotLight.InnerAngle;
            Fraction = Fraction / Delta;
            Attenuation *= pow((1. - Fraction), 2.4);
        }

        /// Here, the probability of sampling a particular spot light is one to the number of spot lights
        float UniformSamplingPDF = 1.f / UtilityData.ActiveSpotLightsCount;
        float ImportanceSamplingPDF = SpotLight.Power / UtilityData.TotalSpotLightPower;

        float PDF = SamplingStrategy == SAMPLE_UNIFORM ? UniformSamplingPDF : ImportanceSamplingPDF;
        OtherSamplingPDF = SamplingStrategy == SAMPLE_UNIFORM ? ImportanceSamplingPDF : UniformSamplingPDF;

        return vec4(SpotLight.Color * SpotLight.Intensity * Attenuation * NDotI / PDF, PDF);
    }

    return vec4(0);
}

vec4 ComputeUniformAreaLightInput(inout FSamplingState SamplingState, out vec3 LightDirection, inout float UniformSamplingImportancePDF, inout float UniformSamplingBXDFPDF)
{
    /// First - select an area light at random (uniformly)
    uint UniformLightIndex = uint(RandomFloat(SamplingState) * UtilityData.ActiveAreaLightsCount);

    FAreaLight ArealLight = AreaLightsBuffer[UniformLightIndex];

    /// Second - select a triangle within area light at random (also uniformly)
    uint UniformTriangleIndex = uint(RandomFloat(SamplingState) * ArealLight.NumberOfTriangles);

    /// Get that random triangle from the mesh that is marked as area light
    Vertices Verts = Vertices(ArealLight.VertexBufferAddress);
    Indices Inds = Indices(ArealLight.IndexBufferAddress);
    bool RenderableIsIndexed = (ArealLight.IsIndexedFlagAndRenderableIndex & 0x80000000) == 0x80000000;
    uint RenderableIndex = ArealLight.IsIndexedFlagAndRenderableIndex & 0x7FFFFFFF;

    FDeviceVertex DV0;
    FDeviceVertex DV1;
    FDeviceVertex DV2;

    if (RenderableIsIndexed)
    {
        uint I0 = 0;
        uint I1 = 1;
        uint I2 = 2;

        I0 = Inds.I[UniformTriangleIndex * 3];
        I1 = Inds.I[UniformTriangleIndex * 3 + 1];
        I2 = Inds.I[UniformTriangleIndex * 3 + 2];

        DV0 = Verts.V[I0];
        DV1 = Verts.V[I1];
        DV2 = Verts.V[I2];
    }
    else
    {
        uint Index = UniformTriangleIndex * 3;
        DV0 = Verts.V[Index];
        DV1 = Verts.V[Index + 1];
        DV2 = Verts.V[Index + 2];
    }

    /// Unpack data
    FVertex V0 = UnpackDeviceVertex(DV0);
    FVertex V1 = UnpackDeviceVertex(DV1);
    FVertex V2 = UnpackDeviceVertex(DV2);

    /// Sample random point on that triangle
    vec2 RandomVec2 = Sample2DUnitQuad(SamplingState);

    /// Without this, RandomPoint will be placed in parallelogram denoted by the triangle
    if (RandomVec2.x + RandomVec2.y > 1.)
    {
        RandomVec2.x = 1. - RandomVec2.x;
        RandomVec2.y = 1. - RandomVec2.y;
    }

    vec3 V0V1Direction = V1.Position - V0.Position;
    vec3 V0V2Direction = V2.Position - V0.Position;
    float TriangleArea = 0.5f * length(cross(V0V1Direction, V0V2Direction));
    /// Get RandomPoint in local space
    vec3 RandomPoint = V0.Position + V0V1Direction * RandomVec2.x + V0V2Direction * RandomVec2.y;

    /// Transform point into world-space
    FDeviceTransform Transform = DeviceTransforms[ArealLight.TransformIndex];
    RandomPoint = vec3(vec4(RandomPoint, 1.f) * Transform.ModelMatrix);
    /// A direction to light
    LightDirection = RandomPoint - ShadingData.IntersectionCoordinatesInWorldSpace;
    float Distance2 = dot(LightDirection, LightDirection);
    LightDirection = normalize(LightDirection);

    /// Check whether light is over the surface
    float NDotL = dot(ShadingData.NormalInWorldSpace, LightDirection);

    if(NDotL <= 0)
    {
        /// Light is on the other side
        return vec4(0);
    }

    /// Construct a ray that goes to the light
    FRayData RayData;
    RayData.RayFlags = 0;
    RayData.Direction.xyz = LightDirection;
    RayData.Origin.xyz = ShadingData.IntersectionCoordinatesInWorldSpace + ShadingData.NormalInWorldSpace * FLOAT_EPSILON;

    /// Trace the ray
    traceRayEXT(TLAS, gl_RayFlagsOpaqueEXT, 0xFF, 0, 0, 0, RayData.Origin.xyz, 0.000001f, RayData.Direction.xyz, 10000, 0);

    /// And if we did hit a required triange, then we store light data
    if (HitPayload.PrimitiveIndex == UniformTriangleIndex && HitPayload.RenderableIndex == RenderableIndex)
    {
        vec3 Barycentrics = vec3(1.f - HitPayload.HitUV.x - HitPayload.HitUV.y, HitPayload.HitUV.x, HitPayload.HitUV.y);

        /// Compute UV
        vec2 UV = V0.TexCoord * Barycentrics.x + V1.TexCoord * Barycentrics.y + V2.TexCoord * Barycentrics.z;
        /// Compute normal
        vec3 AreaLightTriangleNormal = (V0.Normal * Barycentrics.x + V1.Normal * Barycentrics.y + V2.Normal * Barycentrics.z);
        AreaLightTriangleNormal = AreaLightTriangleNormal * mat3(Transform.InverseModelMatrix);
        AreaLightTriangleNormal = normalize(AreaLightTriangleNormal);
        float NDotI = abs(dot(AreaLightTriangleNormal, LightDirection));

        /// Get the material
        FDeviceMaterial EmissiveMaterial = GetEmissiveMaterial(UV, ArealLight.MaterialIndex);
        /// TODO: Is this a correct PDF formula?
        float PDF = Distance2 / (UtilityData.ActiveAreaLightsCount * ArealLight.NumberOfTriangles * TriangleArea * NDotI);
        UniformSamplingImportancePDF = Distance2 / (UtilityData.TotalAreaLightArea * NDotI);
        UniformSamplingBXDFPDF = EvaluateScatteringPDF(Material, ShadingData.MaterialInteractionType, LightDirection);

        return vec4(EmissiveMaterial.EmissionColor * NDotL / PDF, PDF);
    }
    else
    {
        /// Something ocludded the light
        return vec4(0);
    }
}

vec4 ComputeImportanceAreaLightInput(inout FSamplingState SamplingState, out vec3 LightDirection, inout float ImportanceSamplingUniformPDF, inout float ImportanceSamplingBXDFPDF)
{
    /// First - select and area light based on it's area
    uint ImportanceLightIndex = uint(RandomFloat(SamplingState) * UtilityData.ActiveAreaLightsCount);
    FDeviceAliasTableEntry ImportanceSampleTableEntry = AreaLightsImportanceBuffer[ImportanceLightIndex];

    if (RandomFloat(SamplingState) > ImportanceSampleTableEntry.Threshold)
    {
        ImportanceLightIndex = ImportanceSampleTableEntry.Alias;
    }

    FAreaLight ArealLightImportance = AreaLightsBuffer[ImportanceLightIndex];

    /// Second - select a triangle within area light based on it's area
    uint ImportanceTriangleIndex = uint(RandomFloat(SamplingState) * ArealLightImportance.NumberOfTriangles);
    AreaLightsAliasTableReference AreaLightAliasTable = AreaLightsAliasTableReference(ArealLightImportance.AliasTableBufferAddress);

    ImportanceSampleTableEntry = AreaLightAliasTable.T[ImportanceTriangleIndex];

    if (RandomFloat(SamplingState) > ImportanceSampleTableEntry.Threshold)
    {
        ImportanceTriangleIndex = ImportanceSampleTableEntry.Alias;
    }

    /// Get that random triangle from the mesh that is marked as area light
    Vertices Verts = Vertices(ArealLightImportance.VertexBufferAddress);
    Indices Inds = Indices(ArealLightImportance.IndexBufferAddress);
    bool RenderableIsIndexed = (ArealLightImportance.IsIndexedFlagAndRenderableIndex & 0x80000000) == 0x80000000;
    uint RenderableIndex = ArealLightImportance.IsIndexedFlagAndRenderableIndex & 0x7FFFFFFF;

    FDeviceVertex DV0;
    FDeviceVertex DV1;
    FDeviceVertex DV2;

    if (RenderableIsIndexed)
    {
        uint I0 = 0;
        uint I1 = 1;
        uint I2 = 2;

        I0 = Inds.I[ImportanceTriangleIndex * 3];
        I1 = Inds.I[ImportanceTriangleIndex * 3 + 1];
        I2 = Inds.I[ImportanceTriangleIndex * 3 + 2];

        DV0 = Verts.V[I0];
        DV1 = Verts.V[I1];
        DV2 = Verts.V[I2];
    }
    else
    {
        uint Index = ImportanceTriangleIndex * 3;
        DV0 = Verts.V[Index];
        DV1 = Verts.V[Index + 1];
        DV2 = Verts.V[Index + 2];
    }

    /// Unpack data
    FVertex V0 = UnpackDeviceVertex(DV0);
    FVertex V1 = UnpackDeviceVertex(DV1);
    FVertex V2 = UnpackDeviceVertex(DV2);

    /// Sample random point on that triangle
    vec2 RandomVec2 = Sample2DUnitQuad(SamplingState);

    /// Without this, RandomPoint will be placed in parallelogram denoted by the triangle
    if (RandomVec2.x + RandomVec2.y > 1.)
    {
        RandomVec2.x = 1. - RandomVec2.x;
        RandomVec2.y = 1. - RandomVec2.y;
    }

    vec3 V0V1Direction = V1.Position - V0.Position;
    vec3 V0V2Direction = V2.Position - V0.Position;
    float TriangleArea = 0.5f * length(cross(V0V1Direction, V0V2Direction));
    /// Get RandomPoint in local space
    vec3 RandomPoint = V0.Position + V0V1Direction * RandomVec2.x + V0V2Direction * RandomVec2.y;

    /// Transform point into world-space
    FDeviceTransform Transform = DeviceTransforms[ArealLightImportance.TransformIndex];
    RandomPoint = vec3(vec4(RandomPoint, 1.f) * Transform.ModelMatrix);
    /// A direction to light
    LightDirection = RandomPoint - ShadingData.IntersectionCoordinatesInWorldSpace;
    float Distance2 = dot(LightDirection, LightDirection);
    LightDirection = normalize(LightDirection);

    /// Check whether light is over the surface
    float NDotL = dot(ShadingData.NormalInWorldSpace, LightDirection);

    if(NDotL <= 0)
    {
        /// Light is on the other side
        return vec4(0);
    }

    /// Construct a ray that goes to the light
    FRayData RayData;
    RayData.RayFlags = 0;
    RayData.Direction.xyz = LightDirection;
    RayData.Origin.xyz = ShadingData.IntersectionCoordinatesInWorldSpace + ShadingData.NormalInWorldSpace * FLOAT_EPSILON;

    /// Trace the ray
    traceRayEXT(TLAS, gl_RayFlagsOpaqueEXT, 0xFF, 0, 0, 0, RayData.Origin.xyz, 0.000001f, RayData.Direction.xyz, 10000, 0);

    /// And if we did hit a required triange, then we store light data
    if (HitPayload.PrimitiveIndex == ImportanceTriangleIndex && HitPayload.RenderableIndex == RenderableIndex)
    {
        vec3 Barycentrics = vec3(1.f - HitPayload.HitUV.x - HitPayload.HitUV.y, HitPayload.HitUV.x, HitPayload.HitUV.y);

        /// Compute UV
        vec2 UV = V0.TexCoord * Barycentrics.x + V1.TexCoord * Barycentrics.y + V2.TexCoord * Barycentrics.z;
        /// Compute normal
        vec3 AreaLightTriangleNormal = (V0.Normal * Barycentrics.x + V1.Normal * Barycentrics.y + V2.Normal * Barycentrics.z);
        AreaLightTriangleNormal = AreaLightTriangleNormal * mat3(Transform.InverseModelMatrix);
        AreaLightTriangleNormal = normalize(AreaLightTriangleNormal);
        float NDotI = abs(dot(AreaLightTriangleNormal, LightDirection));

        /// Get the material
        FDeviceMaterial EmissiveMaterial = GetEmissiveMaterial(UV, ArealLightImportance.MaterialIndex);
        /// TODO: Is this a correct PDF formula?
        float PDF = Distance2 / (UtilityData.TotalAreaLightArea * NDotI);
        ImportanceSamplingUniformPDF = Distance2 / (UtilityData.ActiveAreaLightsCount * ArealLightImportance.NumberOfTriangles * TriangleArea * NDotI);
        ImportanceSamplingBXDFPDF = EvaluateScatteringPDF(Material, ShadingData.MaterialInteractionType, LightDirection);

        return vec4(EmissiveMaterial.EmissionColor * NDotL / PDF, PDF);
    }
    else
    {
        /// Something ocludded the light
        return vec4(0);
    }
}

vec4 ComputeBXDFAreaLightInput(inout FSamplingState SamplingState, vec3 LightDirection, float BXDFPDF, inout float BXDFSamplingUniformPDF, inout float BXDFSamplingImportancePDF)
{
    /// Construct a shadow ray
    FRayData RayData;
    RayData.RayFlags = 0;
    RayData.Direction.xyz = LightDirection;
    RayData.Origin.xyz = ShadingData.IntersectionCoordinatesInWorldSpace + ShadingData.NormalInWorldSpace * FLOAT_EPSILON;

    /// Trace the ray
    /// TODO: Utilize gl_HitTEXT in hit shader
    traceRayEXT(TLAS, gl_RayFlagsOpaqueEXT, 0xFF, 0, 0, 0, RayData.Origin.xyz, 0.000001f, RayData.Direction.xyz, 10000, 0);

    /// If we hit any geometry, let's check whether it's an emissive
    if (HitPayload.RenderableIndex != UINT_MAX)
    {
        FRenderable Renderable = Renderables[HitPayload.RenderableIndex];
        /// TODO: Add a flag to renderable to skipp all those computations if renderable is not emissive
        Vertices Verts = Vertices(Renderable.VertexBufferAddress);
        Indices Inds = Indices(Renderable.IndexBufferAddress);

        FDeviceVertex DV0;
        FDeviceVertex DV1;
        FDeviceVertex DV2;

        if (IsIndexed(Renderable))
        {
            uint I0 = 0;
            uint I1 = 1;
            uint I2 = 2;

            I0 = Inds.I[HitPayload.PrimitiveIndex * 3];
            I1 = Inds.I[HitPayload.PrimitiveIndex * 3 + 1];
            I2 = Inds.I[HitPayload.PrimitiveIndex * 3 + 2];

            DV0 = Verts.V[I0];
            DV1 = Verts.V[I1];
            DV2 = Verts.V[I2];
        }
        else
        {
            uint Index = HitPayload.PrimitiveIndex * 3;
            DV0 = Verts.V[Index];
            DV1 = Verts.V[Index + 1];
            DV2 = Verts.V[Index + 2];
        }

        /// Unpack data
        FVertex V0 = UnpackDeviceVertex(DV0);
        FVertex V1 = UnpackDeviceVertex(DV1);
        FVertex V2 = UnpackDeviceVertex(DV2);

        vec3 Barycentrics = vec3(1.0 - HitPayload.HitUV.x - HitPayload.HitUV.y, HitPayload.HitUV.x, HitPayload.HitUV.y);
        vec2 TextureCoords = V0.TexCoord * Barycentrics.x + V1.TexCoord * Barycentrics.y + V2.TexCoord * Barycentrics.z;

        /// TODO: Optimize
        FDeviceMaterial EmissiveMaterial = GetEmissiveMaterial(TextureCoords, Renderable.MaterialIndex);

        if (EmissiveMaterial.EmissionWeight != 0)
        {
            FDeviceTransform Transform = DeviceTransforms[Renderable.TransformIndex];

            vec3 EmissiveNormalInWorldSpace = (V0.Normal * Barycentrics.x + V1.Normal * Barycentrics.y + V2.Normal * Barycentrics.z);
            /// Pay more attention to why vector * matrix, and not vice versa
            EmissiveNormalInWorldSpace = EmissiveNormalInWorldSpace * mat3(Transform.InverseModelMatrix);
            EmissiveNormalInWorldSpace = normalize(EmissiveNormalInWorldSpace);

            /// This is the cosine of the angle betweeen the emissive triangle's normal and a shadow ray direction
            /// Why do we take the abs? Because the shadow ray is have to be on the correct side of the hemisphere and
            /// we don't have two sided materials, so emissive emmits on both sides
            float NDotL = abs(dot(EmissiveNormalInWorldSpace, RayData.Direction.xyz));

            uint AreaLightIndex = Renderable.RenderablePropertyMask & RENDERABLE_AREA_LIGHT_INDEX_MASK;
            FAreaLight ArealLight = AreaLightsBuffer[AreaLightIndex];

            vec3 WorldSpaceIntersectionCoordinates = V0.Position * Barycentrics.x + V1.Position * Barycentrics.y + V2.Position * Barycentrics.z;
            WorldSpaceIntersectionCoordinates = vec3(vec4(WorldSpaceIntersectionCoordinates, 1.f) * Transform.ModelMatrix);
            float Distance2 = dot(WorldSpaceIntersectionCoordinates - RayData.Origin.xyz, WorldSpaceIntersectionCoordinates - RayData.Origin.xyz);
            vec3 P0 = vec3(vec4(V0.Position, 1.f) * Transform.ModelMatrix);
            vec3 P1 = vec3(vec4(V1.Position, 1.f) * Transform.ModelMatrix);
            vec3 P2 = vec3(vec4(V2.Position, 1.f) * Transform.ModelMatrix);
            float TriangleArea = 0.5f * length(cross(P1 - P0, P2 - P0));

            float PDF = BXDFPDF;
            BXDFSamplingUniformPDF = Distance2 / (UtilityData.ActiveAreaLightsCount * ArealLight.NumberOfTriangles * TriangleArea * NDotL);
            BXDFSamplingImportancePDF = Distance2 / (UtilityData.TotalAreaLightArea * NDotL);
            return vec4(EmissiveMaterial.EmissionColor * ShadingData.NDotI / PDF, PDF);
        }
    }

    return vec4(0);
}

/// Function generates a random vector direction using importance sampling
vec3 ImportanceSampleIBL(inout FSamplingState SamplingState)
{
    vec2 UVCoordinates = Sample2DUnitQuad(SamplingState);
    const uvec2 IBLSize = textureSize(IBLTextureSamplerLinear, 0);
    uint TexelIndex = uint(UVCoordinates.y * float(IBLSize.y)) * IBLSize.x + uint(UVCoordinates.x * float(IBLSize.x));

    FDeviceAliasTableEntry AliasTableEntry = IBLImportanceBuffer[TexelIndex];
    /// Get texel index
    if (RandomFloat(SamplingState) > AliasTableEntry.Threshold)
    {
        TexelIndex = AliasTableEntry.Alias;
        /// Transform texel index into actual UV coordinates
        UVCoordinates = vec2((float(TexelIndex % IBLSize.x) + 0.5) / float(IBLSize.x), (float(TexelIndex / IBLSize.x) + 0.5) / float(IBLSize.y));
    }

    /// Map UV coordinates to spherical coordinates
    vec2 SphericalCoordinates = UVCoordinates * vec2(M_2_PI, M_PI);
    SphericalCoordinates.x -= M_PI_2;
    vec3 Direction;
    Direction.x = sin(SphericalCoordinates.y) * cos(SphericalCoordinates.x);
    Direction.y = cos(SphericalCoordinates.y);
    Direction.z = sin(SphericalCoordinates.y) * sin(SphericalCoordinates.x);
    return vec3(Direction);
}

/// WARNING, this function is ugly af. Read that comment
/// It tries to cover all 3 types of sampling and to do so we
/// 1. In the case of BxDF sampling Direction should be the already scattered ray direction
/// 2. In the case of BxDF PDF1 should be the PDF of such scattering
/// 3. In case SamplingStrategy is SAMPLE_UNIFORM:
///     3.1 PDF1 will be filled with important sampling PDF
///     3.2 PDF2 will be filled with BxDF sampling PDF
/// 4. In case SamplingStrategy is SAMPLE_IMPORTANCE:
///     4.1 PDF1 will be filled with uniform sampling PDF
///     4.2 PDF2 will be filled with BxDF sampling PDF
/// 5. In case SamplingStrategy is SAMPLE_BXDF:
///     5.1 PDF1 will be filled with uniform sampling PDF
///     5.2 PDF2 will be filled with important sampling PDF
vec4 ComputeIBLInput(inout FSamplingState SamplingState, inout vec3 Direction, uint SamplingStrategy, inout float PDF1, inout float PDF2)
{
    if (SamplingStrategy == SAMPLE_UNIFORM)
        Direction = Sample3DUnitHemisphere(SamplingState) * ShadingData.TransposedTNBMatrix;

    if (SamplingStrategy == SAMPLE_IMPORTANCE)
        Direction = ImportanceSampleIBL(SamplingState);

    vec3 Normal = ShadingData.NormalInWorldSpace;

    /// We invert normal in case if we are sampling BxDF and it was a transmission, because in this case
    /// Transmitted BxDF is on the other side of the surface
    if (SamplingStrategy == SAMPLE_BXDF && ((ShadingData.MaterialInteractionType & TRANSMISSION_LAYER) == TRANSMISSION_LAYER))
        Normal = -Normal;

    float NDotL = dot(Normal, Direction);

    if(NDotL <= 0) return vec4(0);

    FRayData RayData;
    RayData.RayFlags = 0;
    RayData.Direction.xyz = Direction;
    RayData.Origin.xyz = ShadingData.IntersectionCoordinatesInWorldSpace + Normal * FLOAT_EPSILON;

    /// Trace the ray
    traceRayEXT(TLAS, gl_RayFlagsOpaqueEXT, 0xFF, 0, 0, 0, RayData.Origin.xyz, FLOAT_EPSILON, RayData.Direction.xyz, 10000, 0);

    /// And if we didn't hit any geometry, then we sample the IBL
    if (HitPayload.RenderableIndex == UINT_MAX)
    {
        vec2 IBLUV = Vec3ToSphericalUV(Direction, M_PI_2);
        vec3 IBL = texture(IBLTextureSamplerLinear, IBLUV).xyz;

        /// A special case for BxDF samplingm. If scattered ray is singular (perfectly reflected of refracted), just sample IBL and be off
        if (SamplingStrategy == SAMPLE_BXDF && ShadingData.IsScatteredRaySingular)
        {
            PDF1 = 0.f;
            PDF2 = 0.f;
            return vec4(IBL, 1.f);
        }

        const uvec2 IBLSize = textureSize(IBLTextureSamplerLinear, 0);
        uint TexelIndex = uint(IBLUV.y * float(IBLSize.y)) * IBLSize.x + uint(IBLUV.x * float(IBLSize.x));

        float SinTheta = sin(IBLUV.y * M_PI);
        float IBLImportancePDF = SinTheta > 1e-4 ?
        IBLPDFBuffer[TexelIndex] * float(IBLSize.x * IBLSize.y) / (SinTheta * 2.f * M_PI * M_PI) : 0.0;

        /// PDF[0] - Uniform, PDF[1] - Importance, PDF[2] - BxDF
        vec3 PDF = vec3(
            0.5f * M_INV_PI, IBLImportancePDF,
            SamplingStrategy == SAMPLE_BXDF ? PDF1 : EvaluateScatteringPDF(Material, ShadingData.MaterialInteractionType, Direction));
        float PDF0 = 0;

        if (SamplingStrategy == SAMPLE_UNIFORM)     { PDF0 = PDF.x; PDF1 = PDF.y; PDF2 = PDF.z; }
        if (SamplingStrategy == SAMPLE_IMPORTANCE)  { PDF1 = PDF.x; PDF0 = PDF.y; PDF2 = PDF.z; }
        if (SamplingStrategy == SAMPLE_BXDF)        { PDF1 = PDF.x; PDF2 = PDF.y; PDF0 = PDF.z; }

        return vec4(IBL * NDotL / PDF0, PDF0);
    }

    return vec4(0);
}

#endif // LIGHTING_H
