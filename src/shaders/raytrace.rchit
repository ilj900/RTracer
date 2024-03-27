#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_nonuniform_qualifier : enable

#include "common_defines.h"
#include "common_structures.h"

layout (location = 0) rayPayloadInEXT FHit Hit;
hitAttributeEXT vec2 HitAttributes;

layout (set = RAYTRACE_LAYOUT_INDEX, binding = RAYTRACE_RENDERABLE_BUFFER_INDEX) buffer RenderableBufferObject
{
    FRenderable Renderables[];
};

FRenderable FetchRenderable()
{
    return Renderables[nonuniformEXT(gl_InstanceCustomIndexEXT)];
}

void main()
{
    FRenderable Renderable = FetchRenderable();

    Hit.RenderableIndex = gl_InstanceCustomIndexEXT;
    Hit.PrimitiveIndex = gl_PrimitiveID;
    Hit.HitUV = HitAttributes;
    Hit.MaterialIndex = Renderable.MaterialIndex;
}
