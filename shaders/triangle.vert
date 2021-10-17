#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform CameraBufferObject
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
} CameraBuffer;

layout(set = 1, binding = 0) uniform TransformBufferObject
{
    mat4 TransformMatrix;
} TransformBuffer;

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec3 Color;
layout(location = 3) in vec2 TexCoord;

layout(location = 0) out vec3 FragPosition;
layout(location = 1) out vec3 FragNormal;
layout(location = 2) out vec3 FragColor;
layout(location = 3) out vec2 FragTexCoord;

void main()
{
    gl_Position = CameraBuffer.ProjectionMatrix * CameraBuffer.ViewMatrix * TransformBuffer.TransformMatrix * vec4(Position, 1.0);
    FragPosition = vec3(gl_Position);
    FragNormal = Normal;
    FragColor = Color;
    FragTexCoord = TexCoord;
}