#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4 Model;
    mat4 View;
    mat4 Projection;
} UBO;

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Color;
layout(location = 2) in vec2 TexCoord;

layout(location = 0) out vec3 FragColor;
layout(location = 1) out vec2 FragTexCoord;

void main()
{
    gl_Position = UBO.Projection * UBO.View * UBO.Model * vec4(Position, 1.0);
    FragColor = Color;
    FragTexCoord = TexCoord;
}