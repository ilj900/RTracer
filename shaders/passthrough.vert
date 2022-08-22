#version 450

layout(location = 0) out vec2 OutUV;

void main()
{
    OutUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(OutUV * 2.f - 1.f, 1.f, 1.f);
}