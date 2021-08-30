#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "model.h"

#include <unordered_map>

size_t std::hash<FVertex>::operator()(FVertex const& Vertex) const
{
    return ((std::hash<FVector3>{}(Vertex.Pos) ^
             (std::hash<FVector3>{}(Vertex.Color) << 1)) >> 1) ^
           (std::hash<FVector2>{}(Vertex.TexCoord) << 1);
}

VkVertexInputBindingDescription FVertex::GetBindingDescription()
{
    VkVertexInputBindingDescription BindingDescription{};
    BindingDescription.binding = 0;
    BindingDescription.stride = sizeof(FVertex);
    BindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return BindingDescription;
}

std::array<VkVertexInputAttributeDescription, 3> FVertex::GetAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 3> AttributeDescription{};
    AttributeDescription[0].binding = 0;
    AttributeDescription[0].location = 0;
    AttributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    AttributeDescription[0].offset = offsetof(FVertex, Pos);

    AttributeDescription[1].binding = 0;
    AttributeDescription[1].location = 1;
    AttributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    AttributeDescription[1].offset = offsetof(FVertex, Color);

    AttributeDescription[2].binding = 0;
    AttributeDescription[2].location = 2;
    AttributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
    AttributeDescription[2].offset = offsetof(FVertex, TexCoord);

    return AttributeDescription;
}

bool FVertex::operator==(const FVertex& Other) const
{
    return Pos == Other.Pos && Color == Other.Color && TexCoord == Other.TexCoord;
}

FModel::FModel(const std::string &Path, VkDevice LogicalDevice, std::shared_ptr<FResourceAllocator> ResourceAllocator)
{
    /// Load model into RAM
    tinyobj::attrib_t Attrib;
    std::vector<tinyobj::shape_t> Shapes;
    std::vector<tinyobj::material_t> Materials;
    std::string Warn, Err;

    if (!tinyobj::LoadObj(&Attrib, &Shapes, &Materials, &Warn, &Err, Path.c_str()))
    {
        throw std::runtime_error(Warn + Err);
    }

    std::unordered_map<FVertex, uint32_t> UniqueVertices{};


    for (const auto& Shape : Shapes)
    {
        for (const auto& Index : Shape.mesh.indices)
        {
            FVertex Vert{};

            Vert.Pos = {
                    Attrib.vertices[3 * Index.vertex_index + 0],
                    Attrib.vertices[3 * Index.vertex_index + 1],
                    Attrib.vertices[3 * Index.vertex_index + 2]
            };

            Vert.TexCoord = {
                    Attrib.texcoords[2 * Index.texcoord_index + 0],
                    1.f - Attrib.texcoords[2 * Index.texcoord_index + 1],
            };

            Vert.Color = {1.f, 1.f, 1.f};

            if (UniqueVertices.find(Vert) == UniqueVertices.end())
            {
                UniqueVertices[Vert] = static_cast<uint32_t>(Vertices.size());
                Vertices.push_back(Vert);
            }

            Indices.push_back(UniqueVertices[Vert]);
        }
    }

    /// Create vertex buffer
    VkDeviceSize VertexBufferSize = sizeof(Vertices[0]) * Vertices.size();

    StagingBuffer = ResourceAllocator->CreateBuffer(VertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* VertexData;
    vkMapMemory(LogicalDevice, StagingBuffer.Memory, 0, VertexBufferSize, 0, &VertexData);
    memcpy(VertexData, Vertices.data(), (std::size_t)VertexBufferSize);
    vkUnmapMemory(LogicalDevice, StagingBuffer.Memory);

    VertexBuffer = ResourceAllocator->CreateBuffer(VertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    ResourceAllocator->CopyBuffer(StagingBuffer, VertexBuffer, VertexBufferSize);
    ResourceAllocator->DestroyBuffer(StagingBuffer);

    /// Create index buffer
    VkDeviceSize IndexBufferSize = sizeof(Indices[0]) * Indices.size();

    StagingBuffer = ResourceAllocator->CreateBuffer(IndexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* IndexData;
    vkMapMemory(LogicalDevice, StagingBuffer.Memory, 0, IndexBufferSize, 0, &IndexData);
    memcpy(IndexData, Indices.data(), (std::size_t)IndexBufferSize);
    vkUnmapMemory(LogicalDevice, StagingBuffer.Memory);

    IndexBuffer = ResourceAllocator->CreateBuffer(IndexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    ResourceAllocator->CopyBuffer(StagingBuffer, IndexBuffer, IndexBufferSize);
    ResourceAllocator->DestroyBuffer(StagingBuffer);
}