#pragma once

#ifndef __cplusplus
#define FVector4 vec4
#define uint32_t uint
#endif

struct FRayData
{
    FVector4 Origin;
    FVector4 Direction;
    uint32_t RayFlags;
    float TMin;
    float TMax;
    float Dummy;
};