#include "mesh_component.h"

#include "vulkan/vulkan.h"

/// Implementation of hash function
/// I'm not sure this is a good hash function
size_t std::hash<FVertex>::operator()(FVertex const& Vertex) const
{
    return ((std::hash<FVector3>{}(Vertex.Position) ^
             (std::hash<FVector3>{}(Vertex.Normal) << 1)) >> 1) ^
           (std::hash<FVector2>{}(Vertex.TexCoord) << 1);
}

FVertex::FVertex(float PosX, float PosY, float PosZ, float NormX, float NormY, float NormZ, float TexU, float TexV):
        Position(PosX, PosY, PosZ), Normal(NormX, NormY, NormZ), TexCoord(TexU, TexV)
{
}

bool FVertex::operator==(const FVertex& Other) const
{
    return Position == Other.Position && TexCoord == Other.TexCoord;
}

FVertex& FVertex::operator=(const FVertex& Other)
{
    Position.X = Other.Position.X;
    Position.Y = Other.Position.Y;
    Position.Z = Other.Position.Z;
    Normal.X = Other.Normal.X;
    Normal.Y = Other.Normal.Y;
    Normal.Z = Other.Normal.Z;
    TexCoord.X = Other.TexCoord.X;
    TexCoord.Y = Other.TexCoord.Y;
    return *this;
}

VkVertexInputBindingDescription FVertex::GetBindingDescription()
{
    VkVertexInputBindingDescription BindingDescription{};
    BindingDescription.binding = 0;
    BindingDescription.stride = sizeof(FVertex);
    BindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return BindingDescription;
}

std::vector<VkVertexInputAttributeDescription> FVertex::GetAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription> AttributeDescription{};

    VkVertexInputAttributeDescription VertexInputAttributeDescription;

    VertexInputAttributeDescription.location = 0;
    VertexInputAttributeDescription.binding = 0;
    VertexInputAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
    VertexInputAttributeDescription.offset = offsetof(FVertex, Position);
    AttributeDescription.push_back(VertexInputAttributeDescription);

    VertexInputAttributeDescription.location = 1;
    VertexInputAttributeDescription.binding = 0;
    VertexInputAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
    VertexInputAttributeDescription.offset = offsetof(FVertex, Normal);
    AttributeDescription.push_back(VertexInputAttributeDescription);

    VertexInputAttributeDescription.location = 2;
    VertexInputAttributeDescription.binding = 0;
    VertexInputAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
    VertexInputAttributeDescription.offset = offsetof(FVertex, TexCoord);
    AttributeDescription.push_back(VertexInputAttributeDescription);

    return AttributeDescription;
}