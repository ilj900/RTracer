#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "common_defines.h"
#include "common_structures.h"

layout (location = 0) rayPayloadInEXT FHitPayload HitPayload;

void main()
{
    HitPayload.RenderableIndex = UINT_MAX;
    HitPayload.PrimitiveIndex = UINT_MAX;
    HitPayload.HitUV = vec2(0);
    HitPayload.MaterialIndex = TOTAL_MATERIALS - 1;
}
