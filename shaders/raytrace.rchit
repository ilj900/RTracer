#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) rayPayloadInEXT vec3 HitValue;
hitAttributeEXT vec3 Attribs;

void main()
{
  HitValue = vec3(0.2f, 0.5f, 0.5f);
}
