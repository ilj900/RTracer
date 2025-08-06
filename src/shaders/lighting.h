#ifndef LIGHTING_H
#define LIGHTING_H

vec4 ComputeUniformPointLightInput(inout FSamplingState SamplingState, uint PointLightsCount, out vec3 Direction, inout float ImportanceSamplingPDF)
{
    /// Get light index
    const uint LightIndex = uint(RandomFloat(SamplingState) * PointLightsCount);
    FPointLight PointLightUniform = PointLightsBuffer[LightIndex];
    Direction = PointLightUniform.Position - ShadingData.IntersectionCoordinatesInWorldSpace;
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
        /// Here, the probability of sampling a particular point light is one to the number of point lights
        ImportanceSamplingPDF = PointLightUniform.Power / UtilityData.TotalPointLightPower;
        return vec4(PointLightUniform.Color * PointLightUniform.Intensity * Attenuation * NDotI * PointLightsCount, 1.f / PointLightsCount);
    }
    else
    {
        return vec4(0);
    }
}

vec4 ComputeImportancePointLightInput(inout FSamplingState SamplingState, uint PointLightsCount, out vec3 Direction, inout float UniformSamplingPDF)
{
    /// Get light index by importance sampling it
    uint ImportanceLightIndex = uint(RandomFloat(SamplingState) * PointLightsCount);
    FDeviceAliasTableEntry ImportanceSampleTableEntry = PointLightsImportanceBuffer[ImportanceLightIndex];

    if (RandomFloat(SamplingState) > ImportanceSampleTableEntry.Threshold)
    {
        ImportanceLightIndex = ImportanceSampleTableEntry.Alias;
    }

    FPointLight PointLightImportance = PointLightsBuffer[ImportanceLightIndex];

    Direction = PointLightImportance.Position - ShadingData.IntersectionCoordinatesInWorldSpace;
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
        /// Here, the probability of sampling a particular point light depends on it's power and total power of all lights
        float PDF = PointLightImportance.Power / UtilityData.TotalPointLightPower;
        UniformSamplingPDF = 1.f / PointLightsCount;
        return vec4(PointLightImportance.Color * PointLightImportance.Intensity * Attenuation * NDotI / PDF, PDF);
    }
    else
    {
        return vec4(0);
    }
}

vec4 ComputeUniformDirectionalLightInput(inout FSamplingState SamplingState, uint DirectionalLightsCount, out vec3 Direction, inout float ImportanceSamplingPDF)
{
    /// Get light index by uniformly sampling it
    const uint LightIndex = uint(RandomFloat(SamplingState) * DirectionalLightsCount);
    FDirectionalLight DirectionalLight = DirectionalLightsBuffer[LightIndex];

    /// Check whether light is over the surface
    Direction = -DirectionalLight.Direction;
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
        /// Here, the probability of sampling a particular directional light is one to the number of directional lights
        ImportanceSamplingPDF = DirectionalLight.Power / UtilityData.TotalDirectionalLightPower;
        return vec4(DirectionalLight.Color * DirectionalLight.Intensity * NDotI * DirectionalLightsCount, 1.f / DirectionalLightsCount);
    }
    else
    {
        return vec4(0);
    }
}

/// Sample a directional light by it's power
vec4 ComputeImportanceDirectionalLightInput(inout FSamplingState SamplingState, uint DirectionalLightsCount, out vec3 Direction, inout float UniformSamplingPDF)
{
    /// Get light index by importance sampling it
    uint LightIndex = uint(RandomFloat(SamplingState) * DirectionalLightsCount);
    FDeviceAliasTableEntry ImportanceSampleTableEntry = DirectionalLightsImportanceBuffer[LightIndex];

    if (RandomFloat(SamplingState) > ImportanceSampleTableEntry.Threshold)
    {
        LightIndex = ImportanceSampleTableEntry.Alias;
    }

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
        float PDF = DirectionalLight.Power / UtilityData.TotalDirectionalLightPower;
        UniformSamplingPDF = 1.f / DirectionalLightsCount;
        return vec4(DirectionalLight.Color * DirectionalLight.Intensity * NDotI / PDF, PDF);
    }
    else
    {
        return vec4(0);
    }
}

vec4 ComputeUniformSpotLightInput(inout FSamplingState SamplingState, uint SpotLightsCount, out vec3 Direction, inout float ImportanceSamplingPDF)
{
    /// Get light index
    const uint LightIndex = uint(RandomFloat(SamplingState) * SpotLightsCount);
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
        ImportanceSamplingPDF = SpotLight.Power / UtilityData.TotalSpotLightPower;
        return vec4(SpotLight.Color * SpotLight.Intensity * Attenuation * NDotI * SpotLightsCount, 1.f / SpotLightsCount);
    }
    else
    {
        return vec4(0);
    }
}

vec4 ComputeImportanceSpotLightInput(inout FSamplingState SamplingState, uint SpotLightsCount, out vec3 Direction, inout float UniformSamplingPDF)
{
    uint ImportanceLightIndex = uint(RandomFloat(SamplingState) * SpotLightsCount);
    FDeviceAliasTableEntry ImportanceSampleTableEntry = SpotLightsImportanceBuffer[ImportanceLightIndex];

    if (RandomFloat(SamplingState) > ImportanceSampleTableEntry.Threshold)
    {
        ImportanceLightIndex = ImportanceSampleTableEntry.Alias;
    }

    FSpotLight SpotLightImportance = SpotLightsBuffer[ImportanceLightIndex];

    Direction = SpotLightImportance.Position - ShadingData.IntersectionCoordinatesInWorldSpace;
    float LightDistance = length(Direction);
    Direction = normalize(Direction);

    /// Check whether light is over the surface
    float NDotI = dot(ShadingData.NormalInWorldSpace, Direction);

    if(NDotI <= 0)
    {
        return vec4(0);
    }

    float LightAngle = acos(dot(SpotLightImportance.Direction, -Direction));
    /// If point is outside out outer angle, it is also not illuminated
    if (LightAngle > SpotLightImportance.OuterAngle)
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
        if (LightAngle > SpotLightImportance.InnerAngle)
        {
            float Fraction = LightAngle - SpotLightImportance.InnerAngle;
            float Delta = SpotLightImportance.OuterAngle - SpotLightImportance.InnerAngle;
            Fraction = Fraction / Delta;
            Attenuation *= pow((1. - Fraction), 2.4);
        }

        /// Here, the probability of sampling a particular spot light is dependent on the spot light's power and total spot lights power
        float PDF = SpotLightImportance.Power / UtilityData.TotalSpotLightPower;
        UniformSamplingPDF = 1.f / SpotLightsCount;
        return vec4(SpotLightImportance.Color * SpotLightImportance.Intensity * Attenuation * NDotI / PDF, PDF);
    }
    else
    {
        return vec4(0);
    }
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

vec4 ComputeUniformIBLInput(inout FSamplingState SamplingState, out vec3 LightDirection, inout float UniformSamplingImportancePDF, inout float UniformSamplingBXDFPDF)
{
    /// TODO: Verify that sampling is uniform
    /// Sample a random direction in the hemisphere and transform it into world-space coordinate system
    LightDirection = Sample3DUnitHemisphere(SamplingState) * ShadingData.TransposedTNBMatrix;

    FRayData RayData;
    RayData.RayFlags = 0;
    RayData.Direction.xyz = LightDirection;
    RayData.Origin.xyz = ShadingData.IntersectionCoordinatesInWorldSpace + ShadingData.NormalInWorldSpace * FLOAT_EPSILON;

    /// Trace the ray
    traceRayEXT(TLAS, gl_RayFlagsOpaqueEXT, 0xFF, 0, 0, 0, RayData.Origin.xyz, FLOAT_EPSILON, RayData.Direction.xyz, 10000, 0);

    /// And if we didn't hit any geometry, then we sample the IBL
    if (HitPayload.RenderableIndex == UINT_MAX)
    {
        const uvec2 IBLSize = textureSize(IBLTextureSamplerLinear, 0);
        vec2 IBLUV = Vec3ToSphericalUV(ShadingData.WorldSpaceOutgoingDirection, M_PI_2);
        uint TexelIndex = uint(IBLUV.y * IBLSize.x * IBLSize.y) + uint(IBLUV.x * IBLSize.x);

        float NDotL = dot(ShadingData.NormalInWorldSpace, LightDirection);
        float PDF = 0.5f * M_INV_PI;
        UniformSamplingImportancePDF = IBLPDFBuffer[TexelIndex];
        UniformSamplingBXDFPDF = EvaluateScatteringPDF(Material, ShadingData.MaterialInteractionType, LightDirection);
        return vec4(texture(IBLTextureSamplerLinear, IBLUV).xyz * NDotL / PDF, PDF);
    }
    else
    {
        return vec4(0);
    }
}

vec4 ComputeImportanceIBLInput(inout FSamplingState SamplingState, out vec3 LightDirection, inout float ImportanceSamplingUniformPDF, inout float ImportanceSamplingBXDFPDF)
{
    vec2 UVCoordinates = Sample2DUnitQuad(SamplingState);
    const uvec2 IBLSize = textureSize(IBLTextureSamplerLinear, 0);
    uint TexelIndex = uint(IBLSize.x * IBLSize.y * UVCoordinates.y) + uint(IBLSize.x * UVCoordinates.x);

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
    LightDirection.x = sin(SphericalCoordinates.y) * cos(SphericalCoordinates.x);
    LightDirection.y = cos(SphericalCoordinates.y);
    LightDirection.z = sin(SphericalCoordinates.y) * sin(SphericalCoordinates.x);

    float NDotL = dot(ShadingData.NormalInWorldSpace, LightDirection);

    if(NDotL <= 0)
    {
        return vec4(0);
    }

    FRayData RayData;
    RayData.RayFlags = 0;
    RayData.Direction.xyz = LightDirection;
    RayData.Origin.xyz = ShadingData.IntersectionCoordinatesInWorldSpace + ShadingData.NormalInWorldSpace * FLOAT_EPSILON;

    /// Trace the ray
    traceRayEXT(TLAS, gl_RayFlagsOpaqueEXT, 0xFF, 0, 0, 0, RayData.Origin.xyz, FLOAT_EPSILON, RayData.Direction.xyz, 10000, 0);

    /// And if we didn't hit any geometry, then we sample the IBL
    if (HitPayload.RenderableIndex == UINT_MAX)
    {
        float PDF = IBLPDFBuffer[TexelIndex] * IBLSize.x * IBLSize.y;
        ImportanceSamplingUniformPDF = 0.5f * M_INV_PI;
        ImportanceSamplingBXDFPDF = EvaluateScatteringPDF(Material, ShadingData.MaterialInteractionType, LightDirection);
        return vec4(texture(IBLTextureSamplerLinear, UVCoordinates).xyz * NDotL / PDF, PDF);
    }
    else
    {
        return vec4(0);
    }
}

vec4 ComputeBXDFIBLInput(inout FSamplingState SamplingState, vec3 LightDirection, float BXDFSamplingPDF, inout float BXDFSamplingUniformPDF, inout float BXDFSamplingImportancePDF)
{
    FRayData RayData;
    RayData.RayFlags = 0;
    RayData.Direction.xyz = LightDirection;
    bool bRayLeftOnTheOtherSide = (ShadingData.MaterialInteractionType & TRANSMISSION_LAYER) == TRANSMISSION_LAYER;
    RayData.Origin.xyz = ShadingData.IntersectionCoordinatesInWorldSpace + (bRayLeftOnTheOtherSide ? (-ShadingData.NormalInWorldSpace) : ShadingData.NormalInWorldSpace) * FLOAT_EPSILON;

    /// Trace the ray
    traceRayEXT(TLAS, gl_RayFlagsOpaqueEXT, 0xFF, 0, 0, 0, RayData.Origin.xyz, FLOAT_EPSILON, RayData.Direction.xyz, 10000, 0);

    /// And if we didn't hit any geometry, then we sample the IBL
    if (HitPayload.RenderableIndex == UINT_MAX)
    {
        const uvec2 IBLSize = textureSize(IBLTextureSamplerLinear, 0);
        vec2 IBLUV = Vec3ToSphericalUV(LightDirection, M_PI_2);
        uint TexelIndex = uint(IBLUV.y * IBLSize.x * IBLSize.y) + uint(IBLUV.x * IBLSize.x);

        float PDF = BXDFSamplingPDF;
        BXDFSamplingUniformPDF = 0.5f * M_INV_PI;
        BXDFSamplingImportancePDF = IBLPDFBuffer[TexelIndex];
        /// We use abs because LightDirection is actually the direction of scattered ray and that ray can not be on the wrong side of the hemisphere
        float NDotL = abs(dot(ShadingData.NormalInWorldSpace, LightDirection));
        return vec4(texture(IBLTextureSamplerLinear, IBLUV).xyz * NDotL / PDF, PDF);
    }
    else
    {
        return vec4(0);
    }
}

#endif // LIGHTING_H
