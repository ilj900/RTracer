#version 450

layout (location = 0) in vec2 OutUV;
layout (location = 0) out vec4 FragColor;

layout (set = 2, binding = 0) uniform sampler2D InTexture;

void main()
{
    FragColor = vec4(1.f, 0.f, 0.f, 1.f);//texture(InTexture, OutUV).rgba;
}