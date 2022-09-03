#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

struct HitPayload
{
    vec3 HitValue;
};

layout(location = 0) rayPayloadInEXT HitPayload Hit;

hitAttributeEXT vec2 Attribs;

void main()
{
    Hit.HitValue = vec3(0.2f, 0.5f, 0.5f);
}