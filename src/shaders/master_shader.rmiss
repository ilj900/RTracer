#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "common_defines.h"
#include "common_structures.h"

layout (location = 0) rayPayloadInEXT FHitPayload HitPayload;

void main()
{
    /// We only need to mark it that we missed any geometry by setting RenderableIndex to UINT_MAX
    HitPayload.RenderableIndex = UINT_MAX;
}
