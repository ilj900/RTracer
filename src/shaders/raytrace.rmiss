#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "common_defines.h"
#include "common_structures.h"

layout (location = 0) rayPayloadInEXT FHit Hit;

void main()
{
    Hit.RenderableIndex = UINT_MAX;
    Hit.PrimitiveIndex = UINT_MAX;
    Hit.HitUV = vec2(0);
    Hit.MaterialIndex = UINT_MAX;
}
