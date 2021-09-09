#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 OutColor;

layout(location = 0) in vec3 FragPosition;
layout(location = 1) in vec3 FragNormal;
layout(location = 2) in vec3 FragColor;
layout(location = 3) in vec2 FragTexCoord;

layout(binding = 1) uniform sampler2D TexSampler;

vec3 LightPosition = vec3(20.f, 20.f, 0.f);

void main()
{
    vec3 LightDirection = normalize(LightPosition - FragPosition);
    vec3 Normal = normalize(FragNormal);
    float LightAngle = dot(LightDirection, Normal);
    vec3 AmbientColor = FragColor * 0.1f;
    if (LightAngle < 0.f)
    {
        LightAngle = 0.f;
    }
    vec3 DiffuseColor = FragColor * 0.9 * LightAngle;
    OutColor = vec4(DiffuseColor + AmbientColor, 1.f);
}