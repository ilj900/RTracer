#version 450

layout(location = 0) out vec2 OutUV;

int Indices[] = {0, 2, 1};

void main()
{
    OutUV = vec2((Indices[gl_VertexIndex] << 1) & 2, Indices[gl_VertexIndex] & 2);
    gl_Position = vec4(OutUV * 2.f - 1.f, 0.f, 1.f);
}