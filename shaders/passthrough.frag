#version 450

layout (location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(1.f, 0.f, 0.f, 1.f);//texture(InTexture, OutUV).rgba;
}