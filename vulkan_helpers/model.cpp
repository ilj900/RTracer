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

FVertex::FVertex(float PosX, float PosY, float PosZ, float NormX, float NormY, float NormZ,float ColR, float ColG, float ColB, float TexU, float TexV):
Position(PosX, PosY, PosZ), Normal(NormX, NormY, NormZ), Color(ColR, ColG, ColB), TexCoord(TexU, TexV)
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
    if (Indices.size() > 0) {
        VkDeviceSize IndexBufferSize = sizeof(Indices[0]) * Indices.size();

        StagingBuffer = ResourceAllocator->CreateBuffer(IndexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void *IndexData;
        vkMapMemory(LogicalDevice, StagingBuffer.Memory, 0, IndexBufferSize, 0, &IndexData);
        memcpy(IndexData, Indices.data(), (std::size_t) IndexBufferSize);
        vkUnmapMemory(LogicalDevice, StagingBuffer.Memory);

        IndexBuffer = ResourceAllocator->CreateBuffer(IndexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        ResourceAllocator->CopyBuffer(StagingBuffer, IndexBuffer, IndexBufferSize);
        ResourceAllocator->DestroyBuffer(StagingBuffer);
    }
}


void FModel::Draw(VkCommandBuffer CommandBuffer)
{
    if (Indexed)
    {
        vkCmdDrawIndexed(CommandBuffer, static_cast<uint32_t>(Indices.size()), 1, 0, 0, 0);
    }
    else
    {
        vkCmdDraw(CommandBuffer, static_cast<uint32_t>(Vertices.size()), 1, 0, 0);
    }
}

void FModel::Bind(VkCommandBuffer CommandBuffer)
{
    VkBuffer Buffers[] = {VertexBuffer.Buffer};
    VkDeviceSize Offsets[] = {0};
    vkCmdBindVertexBuffers(CommandBuffer, 0, 1, Buffers, Offsets);
    if (Indexed)
    {
        vkCmdBindIndexBuffer(CommandBuffer, IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);
    }
}

FModel FModel::CreateTetrahedron(VkDevice LogicalDevice, std::shared_ptr<FResourceAllocator> ResourceAllocator)
{
    FVector3 Color{0.6627f, 0.451f, 0.3647f};

    float A = 1.f / 3.f;
    float B = std::sqrt(8.f / 9.f);
    float C = std::sqrt(2.f / 9.f);
    float D = std::sqrt(2.f / 3.f);

    std::vector<FVector3> Positions(4);
    Positions[0] = {0.f, 0.f, 1.f};
    Positions[1] = {-C, D, -A};
    Positions[2] = {-C, -D, -A};
    Positions[3] = {B, 0.f, -A};

    std::vector<uint32_t> Indices = {0, 1, 2,
                                     0, 2, 3,
                                     0, 3, 1,
                                     3, 2, 1};

    FModel Tetrahedron;

    for(uint32_t i = 0; i < Indices.size(); i += 3)
    {
        auto V1 = Positions[Indices[i+1]] - Positions[Indices[i]];
        auto V2 = Positions[Indices[i+2]] - Positions[Indices[i]];
        auto Normal = (V1 * V2).GetNormalized();
        Tetrahedron.Vertices.push_back({Positions[Indices[i]].X,    Positions[Indices[i]].Y,    Positions[Indices[i]].Z,
                                        Normal.X,                   Normal.Y,                   Normal.Z,
                                        Color.X,                    Color.Y,                    Color.Z,
                                        0.f, 0.f});
        Tetrahedron.Vertices.push_back({Positions[Indices[i+1]].X,    Positions[Indices[i+1]].Y,    Positions[Indices[i+1]].Z,
                                        Normal.X,                   Normal.Y,                   Normal.Z,
                                        Color.X,                    Color.Y,                    Color.Z,
                                        0.f, 0.f});
        Tetrahedron.Vertices.push_back({Positions[Indices[i+2]].X,    Positions[Indices[i+2]].Y,    Positions[Indices[i+2]].Z,
                                        Normal.X,                   Normal.Y,                   Normal.Z,
                                        Color.X,                    Color.Y,                    Color.Z,
                                        0.f, 0.f});
    }

    Tetrahedron.LoadDataIntoGPU(LogicalDevice, std::move(ResourceAllocator));

    return Tetrahedron;
}

FModel FModel::CreateHexahedron(VkDevice LogicalDevice, std::shared_ptr<FResourceAllocator> ResourceAllocator)
{
    FVector3 Color{0.6627f, 0.451f, 0.3647f};

    float A = 1.f / 3.f;

    std::vector<FVector3> Positions(8);

    Positions[0] = {-A, -A, -A};
    Positions[1] = { A, -A, -A};
    Positions[2] = { A,  A, -A};
    Positions[3] = {-A,  A, -A};
    Positions[4] = {-A, -A,  A};
    Positions[5] = { A, -A,  A};
    Positions[6] = { A,  A,  A};
    Positions[7] = {-A,  A,  A};

    std::vector<uint32_t> Indices = {3, 2, 1, 3, 1, 0,
                                     2, 6, 5, 2, 5, 1,
                                     5, 6, 7, 5, 7, 4,
                                     0, 4, 7, 0, 7, 3,
                                     3, 7, 6, 3, 6, 2,
                                     1, 5, 4, 1, 4, 0};

    FModel Hexahedron;

    for(uint32_t i = 0; i < Indices.size(); i += 3)
    {
        auto V1 = Positions[Indices[i+1]] - Positions[Indices[i]];
        auto V2 = Positions[Indices[i+2]] - Positions[Indices[i]];
        auto Normal = (V1 * V2).GetNormalized();
        Hexahedron.Vertices.push_back({Positions[Indices[i]].X,    Positions[Indices[i]].Y,    Positions[Indices[i]].Z,
                                        Normal.X,                   Normal.Y,                   Normal.Z,
                                        Color.X,                    Color.Y,                    Color.Z,
                                        0.f, 0.f});
        Hexahedron.Vertices.push_back({Positions[Indices[i+1]].X,    Positions[Indices[i+1]].Y,    Positions[Indices[i+1]].Z,
                                        Normal.X,                   Normal.Y,                   Normal.Z,
                                        Color.X,                    Color.Y,                    Color.Z,
                                        0.f, 0.f});
        Hexahedron.Vertices.push_back({Positions[Indices[i+2]].X,    Positions[Indices[i+2]].Y,    Positions[Indices[i+2]].Z,
                                        Normal.X,                   Normal.Y,                   Normal.Z,
                                        Color.X,                    Color.Y,                    Color.Z,
                                        0.f, 0.f});
    }

    Hexahedron.LoadDataIntoGPU(LogicalDevice, std::move(ResourceAllocator));
    return Hexahedron;
}

FModel FModel::CreateIcosahedron(VkDevice LogicalDevice, std::shared_ptr<FResourceAllocator> ResourceAllocator, uint32_t Depth)
{
    FVector3 Color{0.6627f, 0.451f, 0.3647f};

    float X = 0.52573111211f;
    float Z = 0.85065080835f;
    float N = 0.f;

    std::vector<FVector3> InitialPositions(12);
    InitialPositions[0] = {-X, N, Z};
    InitialPositions[1] = {X, N, Z};
    InitialPositions[2] = {-X, N, -Z};
    InitialPositions[3] = {X, N, -Z};
    InitialPositions[4] = {N, Z, X};
    InitialPositions[5] = {N, Z, -X};
    InitialPositions[6] = {N, -Z, X};
    InitialPositions[7] = {N, -Z, -X};
    InitialPositions[8] = {Z, X, N};
    InitialPositions[9] = {-Z, X, N};
    InitialPositions[10] = {Z, -X, N};
    InitialPositions[11] = {-Z, -X, N};

    std::vector<uint32_t> InitialIndices = {0, 1, 4, 0, 4, 9, 9, 4, 5, 4, 8, 5, 4, 1, 8,
                                     8, 1, 10, 8, 10, 3, 5, 8, 3, 5, 3, 2, 2, 3, 7,
                                     7, 3, 10, 7, 10, 6, 7, 6, 11, 11, 6, 0, 0, 6, 1,
                                     6, 10, 1, 9, 11, 0, 9, 2, 11, 9, 5, 2, 7, 11, 2};

    std::vector<FVector3> Positions;
    for(uint32_t i = 0; i < InitialIndices.size(); ++i)
    {
        Positions.push_back(InitialPositions[InitialIndices[i]]);
    }

    for (uint32_t i = 1; i < Depth; i++)
    {
        std::vector<FVector3> NextIterationPositions;

        for (uint32_t j = 0; j < Positions.size(); j += 3)
        {
            FVector3 A = Positions[j];
            FVector3 B = Positions[j+1];
            FVector3 C = Positions[j+2];

            FVector3 AB = (A + B).GetNormalized();
            FVector3 AC = (A + C).GetNormalized();
            FVector3 BC = (B + C).GetNormalized();

            NextIterationPositions.push_back(A);
            NextIterationPositions.push_back(AB);
            NextIterationPositions.push_back(AC);
            NextIterationPositions.push_back(B);
            NextIterationPositions.push_back(BC);
            NextIterationPositions.push_back(AB);
            NextIterationPositions.push_back(C);
            NextIterationPositions.push_back(AC);
            NextIterationPositions.push_back(BC);
            NextIterationPositions.push_back(AB);
            NextIterationPositions.push_back(BC);
            NextIterationPositions.push_back(AC);
        }
        Positions = std::move(NextIterationPositions);
    }

    FModel Icosahedron;

    for(uint32_t i = 0; i < Positions.size(); i += 3)
    {
        auto V1 = Positions[i+1] - Positions[i];
        auto V2 = Positions[i+2] - Positions[i];
        auto Normal = (V1 * V2).GetNormalized();
        Icosahedron.Vertices.push_back({Positions[i].X,    Positions[i].Y,    Positions[i].Z,
                                       Normal.X,           Normal.Y,                   Normal.Z,
                                       Color.X,            Color.Y,                    Color.Z,
                                       0.f, 0.f});
        Icosahedron.Vertices.push_back({Positions[i+1].X,    Positions[i+1].Y,    Positions[i+1].Z,
                                       Normal.X,                   Normal.Y,                   Normal.Z,
                                       Color.X,                    Color.Y,                    Color.Z,
                                       0.f, 0.f});
        Icosahedron.Vertices.push_back({Positions[i+2].X,    Positions[i+2].Y,    Positions[i+2].Z,
                                       Normal.X,                   Normal.Y,                   Normal.Z,
                                       Color.X,                    Color.Y,                    Color.Z,
                                       0.f, 0.f});
    }

    Icosahedron.LoadDataIntoGPU(LogicalDevice, std::move(ResourceAllocator));
    return Icosahedron;
}
