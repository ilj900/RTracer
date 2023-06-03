#version 460

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_buffer_reference2 : require

const uint RENDERABLE_SELECTED_BIT = 1 << 5;
const uint RENDERABLE_HAS_TEXTURE = 1 << 6;
const uint RENDERABLE_IS_INDEXED = 1 << 7;

struct HitPayload
{
    vec3 Color;
    vec2 IBLCoordinates;
};

hitAttributeEXT vec2 Attributes;

layout(location = 0) rayPayloadInEXT HitPayload Hit;

struct FVertex
{
    vec3 Position;
    vec3 Normal;
    vec2 TexCoord;
};

layout(buffer_reference, scalar) buffer Vertices
{
    FVertex V[];
};

layout(buffer_reference, scalar) buffer Indices
{
    ivec3 I[];
};

struct FRenderable
{
    vec3 RenderableColor;
    float dummy_1;

    int RenderableIndex;
    uint RenderablePropertyMask;
    uint dummy_2;
    uint dummy_3;

    uint64_t VertexBufferAddress;
    uint64_t IndexBufferAddress;
};

layout (set = 0, binding = 3) buffer RenderableBufferObject
{
    FRenderable Renderables[];
} RenderableBuffer;

bool CheckFlag(uint Mask, uint Field)
{
    if ((Mask & Field) == Field)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool IsIndexed(FRenderable Renderable)
{
    return CheckFlag(Renderable.RenderablePropertyMask, RENDERABLE_IS_INDEXED);
}

void main()
{
    FRenderable Renderable = RenderableBuffer.Renderables[nonuniformEXT(gl_InstanceCustomIndexEXT)];
    Vertices Verts = Vertices(Renderable.VertexBufferAddress);
    Indices Inds = Indices(Renderable.IndexBufferAddress);

    vec3 Normal = vec3(1.f, 1.f, 1.f);

    if (IsIndexed(Renderable))
    {
        int I0 = 0;
        int I1 = 1;
        int I2 = 2;

        ivec3 I =  Inds.I[gl_PrimitiveID];
        I0 = I.x;
        I1 = I.y;
        I2 = I.z;

        FVertex V0 = Verts.V[I0];
        FVertex V1 = Verts.V[I1];
        FVertex V2 = Verts.V[I2];
        vec3 V10 = V1.Position - V0.Position;
        vec3 V20 = V2.Position - V0.Position;
        vec3 Cross = normalize(cross(V10, V20));

        Normal = vec3(Cross);
    }
    else
    {
        int Index = gl_PrimitiveID * 3;
        FVertex V0 = Verts.V[Index];
        FVertex V1 = Verts.V[Index + 1];
        FVertex V2 = Verts.V[Index + 2];

        Normal = normalize(cross((V1.Position - V0.Position), (V2.Position - V0.Position)));
    }
    Hit.Color = Normal;
}
