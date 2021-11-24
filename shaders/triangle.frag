#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 MainColorOutput;
layout(location = 1) out vec4 SecondaryOutput;

layout(location = 0) in vec3 FragPosition;
layout(location = 1) in vec3 FragNormal;
layout(location = 2) in vec3 FragColor;
layout(location = 3) in vec2 FragTexCoord;
flat layout(location = 4) in uint RenderableIndex;
flat layout(location = 5) in uint RenderablePropertyMask;

layout(set = 0, binding = 1) uniform sampler2D TexSampler;

vec3 LightPosition = vec3(20.f, 20.f, 0.f);

const uint RENDERABLE_SELECTED_BIT = 1 << 5;

bool GetSelected(uint Mask)
{
    if ((Mask & RENDERABLE_SELECTED_BIT) == RENDERABLE_SELECTED_BIT)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void main()
{
    vec3 LightDirection = normalize(LightPosition - FragPosition);
    vec3 Normal = normalize(FragNormal);
    float LightAngle = dot(LightDirection, Normal);
    vec3 Color = FragColor;

    vec3 AmbientColor = Color * 0.1f;
    if (LightAngle < 0.f)
    {
        LightAngle = 0.f;
    }
    vec3 DiffuseColor = Color * 0.9 * LightAngle;

    if (!GetSelected(RenderablePropertyMask))
    {
        MainColorOutput = vec4(DiffuseColor + AmbientColor, 1.f);
    }

    if (GetSelected(RenderablePropertyMask))
    {
        MainColorOutput = vec4(1.f, 1.f, 1.f, 1.f);
    }

    SecondaryOutput = vec4(Normal, 1.f);

//    MainColorOutput = texture(TexSampler, FragTexCoord);
}