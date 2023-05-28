#version 460

#extension GL_EXT_ray_tracing : require

struct HitPayload
{
    vec3 Color;
    vec2 IBLCoordinates;
};

layout(location = 0) rayPayloadInEXT HitPayload Hit;

layout(set = 0, binding = 4) uniform sampler2D IBITextureSampelr;

void main()
{
    Hit.Color = vec3(texture(IBITextureSampelr, Hit.IBLCoordinates));
}
