#version 460

#extension GL_EXT_ray_tracing : require

struct HitPayload
{
    vec3 HitValue;
};

layout(location = 0) rayPayloadInEXT HitPayload Hit;

void main()
{
    Hit.HitValue = vec3(0.f, 0.1f, 0.3f);
}