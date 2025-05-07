#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "common_structures.h"

layout (location = 0) rayPayloadInEXT FHitPayload HitPayload;
hitAttributeEXT vec2 HitAttributes;

void main()
{
    /// We need some data for the area lights
    HitPayload.RenderableIndex = gl_InstanceCustomIndexEXT;
    HitPayload.PrimitiveIndex = gl_PrimitiveID;
    HitPayload.HitUV = HitAttributes;
}