#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_buffer_reference2 : require

#include "common_defines.h"
#include "common_structures.h"

hitAttributeEXT vec2 Attributes;

layout (location = 0) rayPayloadInEXT HitPayload Hit;
hitAttributeEXT vec2 HitAttributes;

layout (buffer_reference, scalar) buffer Vertices
{
    FVertex V[];
};

layout (buffer_reference, scalar) buffer Indices
{
    ivec3 I[];
};

layout (set = RAYTRACE_LAYOUT_INDEX, binding = RAYTRACE_RENDERABLE_BUFFER_INDEX) buffer RenderableBufferObject
{
    FRenderable Renderables[];
};

layout (set = RAYTRACE_LAYOUT_INDEX, binding = RAYTRACE_MATERIAL_BUFFER_INDEX) buffer MaterialBufferObject
{
    FMaterial Materials[];
};

layout (set = RAYTRACE_LAYOUT_INDEX, binding = RAYTRACE_LIGHT_BUFFER_INDEX) buffer LightBufferObject
{
    FLight Lights[];
};

layout (set = RAYTRACE_LAYOUT_INDEX, binding = RAYTRACE_TEXTURE_SAMPLER) uniform sampler Sampler;

layout (set = RAYTRACE_LAYOUT_INDEX, binding = RAYTRACE_TEXTURE_ARRAY) uniform texture2D Textures[MAX_TEXTURES];

FRenderable FetchRenderable()
{
    return Renderables[nonuniformEXT(gl_InstanceCustomIndexEXT)];
}

FMaterial FetchMaterial()
{
    return Materials[nonuniformEXT(gl_InstanceCustomIndexEXT)];
}

FLight FetchLight(int LightIndex)
{
    return Lights[nonuniformEXT(LightIndex)];
}

bool CheckFlag(uint Mask, uint Field)
{
    if ((Mask & Field) == Field)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsIndexed(FRenderable Renderable)
{
    return CheckFlag(Renderable.RenderablePropertyMask, RENDERABLE_IS_INDEXED);
}

void main()
{
    FRenderable Renderable = FetchRenderable();
    FMaterial Material = FetchMaterial();
    Vertices Verts = Vertices(Renderable.VertexBufferAddress);
    Indices Inds = Indices(Renderable.IndexBufferAddress);

    vec3 Normal = vec3(1.f, 1.f, 1.f);

    FVertex V0;
    FVertex V1;
    FVertex V2;

    if (IsIndexed(Renderable))
    {
        int I0 = 0;
        int I1 = 1;
        int I2 = 2;

        ivec3 I =  Inds.I[gl_PrimitiveID];
        I0 = I.x;
        I1 = I.y;
        I2 = I.z;

        V0 = Verts.V[I0];
        V1 = Verts.V[I1];
        V2 = Verts.V[I2];
        vec3 V10 = V1.Position - V0.Position;
        vec3 V20 = V2.Position - V0.Position;
        vec3 Cross = normalize(cross(V10, V20));

        Normal = vec3(Cross);
    }
    else
    {
        int Index = gl_PrimitiveID * 3;
        V0 = Verts.V[Index];
        V1 = Verts.V[Index + 1];
        V2 = Verts.V[Index + 2];

        Normal = normalize(cross((V1.Position - V0.Position), (V2.Position - V0.Position)));
    }

    const vec3 Barycentrics = vec3(1.0 - HitAttributes.x - HitAttributes.y, HitAttributes.x, HitAttributes.y);

    vec3 PointOfIntersectionInLocalSpace = V0.Position * Barycentrics.x + V1.Position * Barycentrics.y + V2.Position * Barycentrics.z;
    vec3 PointOfIntersectionInWorldSpace = vec3(gl_ObjectToWorldEXT * vec4(PointOfIntersectionInLocalSpace, 1.f));

    FLight Light = FetchLight(0);
    vec3 PointOfIntersectioToLightDirection = Light.Position - PointOfIntersectionInWorldSpace;
    float CosNormalToLightAngle = dot(normalize(PointOfIntersectioToLightDirection), Normal);
    float Distance2 = dot(PointOfIntersectioToLightDirection, PointOfIntersectioToLightDirection);
    float Luminance = Light.Intensity * CosNormalToLightAngle / Distance2;

    Hit.Color = Material.BaseAlbedo * Luminance;
}
