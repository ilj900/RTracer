#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "model.h"

#include "components/transform_component.h"
#include "components/device_transform_component.h"
#include "systems/transform_system.h"
#include "coordinator.h"

#include <unordered_map>

size_t std::hash<FVertex>::operator()(FVertex const& Vertex) const
{
    /// I'm not sure this is a good hash function
    return ((((std::hash<FVector3>{}(Vertex.Position) ^
    (std::hash<FVector3>{}(Vertex.Color) << 1)) >> 1) ^
    (std::hash<FVector3>{}(Vertex.Normal) << 1)) >> 1) ^
    (std::hash<FVector2>{}(Vertex.TexCoord) << 1);
}

FVertex::FVertex(float PosX, float PosY, float PosZ, float ColR, float ColG, float ColB, float TexU, float TexV):
Position(PosX, PosY, PosZ), Color(ColR, ColG, ColB), TexCoord(TexU, TexV)
{
}

FVertex& FVertex::operator=(const FVertex& Other)
{
    Position.X = Other.Position.X;
    Position.Y = Other.Position.Y;
    Position.Z = Other.Position.Z;
    Color.X = Other.Color.X;
    Color.Y = Other.Color.Y;
    Color.Z = Other.Color.Z;
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

std::array<VkVertexInputAttributeDescription, 4> FVertex::GetAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 4> AttributeDescription{};
    AttributeDescription[0].binding = 0;
    AttributeDescription[0].location = 0;
    AttributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    AttributeDescription[0].offset = offsetof(FVertex, Position);

    AttributeDescription[1].binding = 0;
    AttributeDescription[1].location = 1;
    AttributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    AttributeDescription[1].offset = offsetof(FVertex, Normal);

    AttributeDescription[2].binding = 0;
    AttributeDescription[2].location = 2;
    AttributeDescription[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    AttributeDescription[2].offset = offsetof(FVertex, Color);

    AttributeDescription[3].binding = 0;
    AttributeDescription[3].location = 3;
    AttributeDescription[3].format = VK_FORMAT_R32G32_SFLOAT;
    AttributeDescription[3].offset = offsetof(FVertex, TexCoord);

    return AttributeDescription;
}

bool FVertex::operator==(const FVertex& Other) const
{
    return Position == Other.Position && Color == Other.Color && TexCoord == Other.TexCoord;
}

FModel::FModel()
{
    auto& Coordinator = ECS::GetCoordinator();
    Model = Coordinator.CreateEntity();
    Coordinator.AddComponent<ECS::COMPONENTS::FTransformComponent>(Model, {FVector3{0.f, 0.f, 0.f}, FVector3{0.f, 0.f, 1.f}, FVector3{0.f, 1.f, 0.f}});
    Coordinator.AddComponent<ECS::COMPONENTS::FDeviceTransformComponent>(Model, {});
    auto TransformSystem = Coordinator.GetSystem<ECS::SYSTEMS::FTransformSystem>();
    TransformSystem->UpdateDeviceComponentData(Model);
}

FModel::FModel(const std::string &Path, VkDevice LogicalDevice, std::shared_ptr<FResourceAllocator> ResourceAllocator)
{
    auto& Coordinator = ECS::GetCoordinator();
    Model = Coordinator.CreateEntity();
    Coordinator.AddComponent<ECS::COMPONENTS::FTransformComponent>(Model, {FVector3{0.f, 0.f, 0.f}, FVector3{0.f, 0.f, 1.f},FVector3 {0.f, 1.f, 0.f}});
    Coordinator.AddComponent<ECS::COMPONENTS::FDeviceTransformComponent>(Model, {});
    auto TransformSystem = Coordinator.GetSystem<ECS::SYSTEMS::FTransformSystem>();
    TransformSystem->UpdateDeviceComponentData(Model);

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

            Vert.Position = {
                    Attrib.vertices[3 * Index.vertex_index + 0],
                    Attrib.vertices[3 * Index.vertex_index + 1],
                    Attrib.vertices[3 * Index.vertex_index + 2]
            };

            Vert.Normal = {Attrib.normals[3 * Index.normal_index + 0],
                           Attrib.normals[3 * Index.normal_index + 1],
                           Attrib.normals[3 * Index.normal_index + 2]
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

    LoadDataIntoGPU(LogicalDevice, ResourceAllocator);
}
void FModel::LoadDataIntoGPU(VkDevice LogicalDevice, std::shared_ptr<FResourceAllocator> ResourceAllocator)
{
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


void FModel::Draw(VkCommandBuffer CommandBuffer)
{
    vkCmdDrawIndexed(CommandBuffer, static_cast<uint32_t>(Indices.size()), 1, 0, 0, 0);
}

void FModel::Bind(VkCommandBuffer CommandBuffer)
{
    VkBuffer Buffers[] = {VertexBuffer.Buffer};
    VkDeviceSize Offsets[] = {0};
    vkCmdBindVertexBuffers(CommandBuffer, 0, 1, Buffers, Offsets);
    vkCmdBindIndexBuffer(CommandBuffer, IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);
}

FModel FModel::CreateTetrahedron(VkDevice LogicalDevice, std::shared_ptr<FResourceAllocator> ResourceAllocator)
{
    float A = 1.f / 3.f;
    float B = std::sqrt(8.f / 9.f);
    float C = std::sqrt(2.f / 9.f);
    float D = std::sqrt(2.f / 3.f);

    FModel Tetrahedron;

    Tetrahedron.Vertices.resize(4);
    Tetrahedron.Vertices[0] = FVertex(0.f, 0.f, 1.f, 0.6627f, 0.451f, 0.3647f, 0.f, 0.f);
    Tetrahedron.Vertices[1] = FVertex(-C, D, -A, 0.6627f, 0.451f, 0.3647f, 0.f, 0.f);
    Tetrahedron.Vertices[2] = FVertex(-C, -D, -A, 0.6627f, 0.451f, 0.3647f, 0.f, 0.f);
    Tetrahedron.Vertices[3] = FVertex(B, 0.f, -A, 0.6627f, 0.451f, 0.3647f, 0.f, 0.f);

    Tetrahedron.Indices = {0, 1, 2, 0, 2, 3, 0, 3, 1, 3, 2, 1};
    Tetrahedron.LoadDataIntoGPU(LogicalDevice, std::move(ResourceAllocator));

    return Tetrahedron;
}

FModel FModel::CreateHexahedron(VkDevice LogicalDevice, std::shared_ptr<FResourceAllocator> ResourceAllocator)
{
    float A = 1.f / 3.f;

    FModel Hexahedron;
    Hexahedron.Vertices.resize(8);
    Hexahedron.Vertices[0] = FVertex(-A, -A, -A, 0.6627f, 0.451f, 0.3647f, 0.f, 0.f);
    Hexahedron.Vertices[1] = FVertex( A, -A, -A, 0.6627f, 0.451f, 0.3647f, 0.f, 0.f);
    Hexahedron.Vertices[2] = FVertex( A,  A, -A, 0.6627f, 0.451f, 0.3647f, 0.f, 0.f);
    Hexahedron.Vertices[3] = FVertex(-A,  A, -A, 0.6627f, 0.451f, 0.3647f, 0.f, 0.f);
    Hexahedron.Vertices[4] = FVertex(-A, -A,  A, 0.6627f, 0.451f, 0.3647f, 0.f, 0.f);
    Hexahedron.Vertices[5] = FVertex( A, -A,  A, 0.6627f, 0.451f, 0.3647f, 0.f, 0.f);
    Hexahedron.Vertices[6] = FVertex( A,  A,  A, 0.6627f, 0.451f, 0.3647f, 0.f, 0.f);
    Hexahedron.Vertices[7] = FVertex(-A,  A,  A, 0.6627f, 0.451f, 0.3647f, 0.f, 0.f);

    Hexahedron.Indices = {3, 2, 1, 3, 1, 0,
                          2, 6, 5, 2, 5, 1,
                          5, 6, 7, 5, 7, 4,
                          0, 4, 7, 0, 7, 3,
                          3, 7, 6, 3, 6, 2,
                          1, 5, 4, 1, 4, 0};

    Hexahedron.LoadDataIntoGPU(LogicalDevice, std::move(ResourceAllocator));
    return Hexahedron;

}