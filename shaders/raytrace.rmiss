#version 460

#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadEXT vec3 hitValue;

void main()
{
    hitValue = vec3(0.f, 0.1f, 0.3f);
}