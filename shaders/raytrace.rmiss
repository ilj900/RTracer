#version 460

#extension GL_EXT_ray_tracing : require

struct HitPayload
{
    vec3 Color;
    vec3 Direction;
};

layout(location = 0) rayPayloadInEXT HitPayload Hit;

layout(set = 0, binding = 7) uniform sampler2D IBITextureSampelr;

void main()
{
    vec3 NormalizedDirection = normalize(Hit.Direction);
    float Tmp = atan(NormalizedDirection.z, NormalizedDirection.x);
    float Phi = Tmp < 0.f ? (Tmp + (2 * 3.14159265357)) : Tmp;
    float Theta = acos(NormalizedDirection.y);
    Phi /= 2.f * 3.14159265357;
    Theta /= 3.14159265357;

    Hit.Color = vec3(texture(IBITextureSampelr, vec2(Phi, Theta)));
}
