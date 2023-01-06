#version 450

layout (location = 0) in vec2 OutUV;

layout (location = 0) out vec4 FragColor;

layout (set = 0, binding = 0) uniform sampler2D InTexture;

void main()
{
    FragColor = texture(InTexture, OutUV).rgba;
}