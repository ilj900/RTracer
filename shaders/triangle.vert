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

layout(set = 1, binding = 1) uniform RenderableBufferObject
{
    vec3 RenderableColor;
    uint Padding1;

    uint RenderableIndex;
    uint RenderablePropertyMask;
    uint Padding2;
    uint Padding3;
} RenderableBuffer;

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoord;

layout(location = 0) out vec3 FragPosition;
layout(location = 1) out vec3 FragNormal;
layout(location = 2) out vec3 FragColor;
layout(location = 3) out vec2 FragTexCoord;

flat layout(location = 4) out uint RenderableIndex;
flat layout(location = 5) out uint RenderablePropertyMask;

void main()
{
    gl_Position = CameraBuffer.ProjectionMatrix * CameraBuffer.ViewMatrix * TransformBuffer.TransformMatrix * vec4(Position, 1.0);
    FragPosition = vec3(gl_Position);
    FragNormal = Normal;
    FragTexCoord = TexCoord;
    FragColor = RenderableBuffer.RenderableColor;
    RenderableIndex = RenderableBuffer.RenderableIndex;
    RenderablePropertyMask = RenderableBuffer.RenderablePropertyMask;
}