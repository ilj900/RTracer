#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "components/mesh_component.h"
#include "components/device_mesh_component.h"
#include "systems/mesh_system.h"
#include "coordinator.h"
#include "context.h"

#include <cassert>

namespace ECS
{
    namespace SYSTEMS
    {
        template<typename T>
        inline T& FMeshSystem::GetComponent(FEntity Entity)
        {
            assert(Entities.find(Entity) != Entities.end() && "Entity doesn't have camera component");
            auto& Coordinator = GetCoordinator();
            auto& RenderableComponent = Coordinator.GetComponent<T>(Entity);
            return RenderableComponent;
        }

        void FMeshSystem::Draw(FEntity Entity, VkCommandBuffer CommandBuffer)
        {
            auto& MeshComponent = GetComponent<ECS::COMPONENTS::FMeshComponent>(Entity);

            if (MeshComponent.Indexed)
            {
                vkCmdDrawIndexed(CommandBuffer, static_cast<uint32_t>(MeshComponent.Indices.size()), 1, 0, 0, 0);
            }
            else
            {
                vkCmdDraw(CommandBuffer, static_cast<uint32_t>(MeshComponent.Vertices.size()), 1, 0, 0);
            }
        }

        void FMeshSystem::Bind(FEntity Entity, VkCommandBuffer CommandBuffer)
        {
            auto& MeshComponent = GetComponent<ECS::COMPONENTS::FMeshComponent>(Entity);
            auto& DeviceMeshComponent = GetComponent<ECS::COMPONENTS::FDeviceMeshComponent>(Entity);

            VkBuffer Buffers[] = {DeviceMeshComponent.VertexBuffer.Buffer};
            VkDeviceSize Offsets[] = {0};
            vkCmdBindVertexBuffers(CommandBuffer, 0, 1, Buffers, Offsets);
            if (MeshComponent.Indexed)
            {
                vkCmdBindIndexBuffer(CommandBuffer, DeviceMeshComponent.IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);
            }
        }

        void FMeshSystem::LoadToGPU(FEntity Entity)
        {
            auto& MeshComponent = GetComponent<ECS::COMPONENTS::FMeshComponent>(Entity);
            auto& DeviceMeshComponent = GetComponent<ECS::COMPONENTS::FDeviceMeshComponent>(Entity);

            auto& Context = GetContext();
            DeviceMeshComponent.VertexBuffer = Context.ResourceAllocator->CreateBufferWidthData(MeshComponent.Vertices.size() * sizeof(FVertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, MeshComponent.Vertices.data());
            if (MeshComponent.Indexed)
            {
                DeviceMeshComponent.IndexBuffer = Context.ResourceAllocator->CreateBufferWidthData(MeshComponent.Indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, MeshComponent.Indices.data() );
            }
        }

        std::set<FEntity>::iterator  FMeshSystem::begin()
        {
            return Entities.begin();
        }

        std::set<FEntity>::iterator  FMeshSystem::end()
        {
            return Entities.end();
        }

        uint32_t FMeshSystem::Size()
        {
            return static_cast<uint32_t>(Entities.size());
        }

        void FMeshSystem::FreeAllDeviceData()
        {
            auto& Context = GetContext();
            for (auto Entity : Entities)
            {
                auto& MeshComponent = GetComponent<ECS::COMPONENTS::FMeshComponent>(Entity);
                auto& DeviceMeshComponent = GetComponent<ECS::COMPONENTS::FDeviceMeshComponent>(Entity);
                Context.FreeData(DeviceMeshComponent.VertexBuffer);
                if (MeshComponent.Indexed)
                {
                    Context.FreeData(DeviceMeshComponent.IndexBuffer);
                }
            }
        }

        void FMeshSystem::LoadMesh(FEntity Entity, const std::string &Path)
        {
            auto& MeshComponent = GetComponent<ECS::COMPONENTS::FMeshComponent>(Entity);

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

                    if (UniqueVertices.find(Vert) == UniqueVertices.end())
                    {
                        UniqueVertices[Vert] = static_cast<uint32_t>(MeshComponent.Vertices.size());
                        MeshComponent.Vertices.push_back(Vert);
                    }

                    MeshComponent.Indices.push_back(UniqueVertices[Vert]);
                }
            }

            MeshComponent.Indexed = true;
        }

        void FMeshSystem::CreateTetrahedron(FEntity Entity)
        {
            auto& MeshComponent = GetComponent<ECS::COMPONENTS::FMeshComponent>(Entity);

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

            for(uint32_t i = 0; i < Indices.size(); i += 3)
            {
                auto V1 = Positions[Indices[i+1]] - Positions[Indices[i]];
                auto V2 = Positions[Indices[i+2]] - Positions[Indices[i]];
                auto Normal = (V1 * V2).GetNormalized();
                MeshComponent.Vertices.emplace_back(Positions[Indices[i]].X,    Positions[Indices[i]].Y,    Positions[Indices[i]].Z,
                                                Normal.X,                   Normal.Y,                   Normal.Z,
                                                0.f, 0.f);
                MeshComponent.Vertices.emplace_back(Positions[Indices[i+1]].X,    Positions[Indices[i+1]].Y,    Positions[Indices[i+1]].Z,
                                                Normal.X,                   Normal.Y,                   Normal.Z,
                                                0.f, 0.f);
                MeshComponent.Vertices.emplace_back(Positions[Indices[i+2]].X,    Positions[Indices[i+2]].Y,    Positions[Indices[i+2]].Z,
                                                Normal.X,                   Normal.Y,                   Normal.Z,
                                                0.f, 0.f);
            }
        }

        void FMeshSystem::CreateHexahedron(FEntity Entity)
        {
            auto& Coordinator = ECS::GetCoordinator();
            auto& MeshComponent = Coordinator.GetComponent<ECS::COMPONENTS::FMeshComponent>(Entity);

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

            for(uint32_t i = 0; i < Indices.size(); i += 3)
            {
                auto V1 = Positions[Indices[i+1]] - Positions[Indices[i]];
                auto V2 = Positions[Indices[i+2]] - Positions[Indices[i]];
                auto Normal = (V1 * V2).GetNormalized();
                MeshComponent.Vertices.emplace_back(Positions[Indices[i]].X,    Positions[Indices[i]].Y,    Positions[Indices[i]].Z,
                                               Normal.X,                   Normal.Y,                   Normal.Z,
                                               0.f, 0.f);
                MeshComponent.Vertices.emplace_back(Positions[Indices[i+1]].X,    Positions[Indices[i+1]].Y,    Positions[Indices[i+1]].Z,
                                               Normal.X,                   Normal.Y,                   Normal.Z,
                                               0.f, 0.f);
                MeshComponent.Vertices.emplace_back(Positions[Indices[i+2]].X,    Positions[Indices[i+2]].Y,    Positions[Indices[i+2]].Z,
                                               Normal.X,                   Normal.Y,                   Normal.Z,
                                               0.f, 0.f);
            }
        }

        void FMeshSystem::CreateIcosahedron(FEntity Entity, uint32_t Depth)
        {
            auto& MeshComponent = GetComponent<ECS::COMPONENTS::FMeshComponent>(Entity);

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

            for(uint32_t i = 0; i < Positions.size(); i += 3)
            {
                auto V1 = Positions[i+1] - Positions[i];
                auto V2 = Positions[i+2] - Positions[i];
                auto Normal = (V1 * V2).GetNormalized();
                MeshComponent.Vertices.emplace_back(Positions[i].X,    Positions[i].Y,    Positions[i].Z,
                                                Normal.X,           Normal.Y,                   Normal.Z,
                                                0.f, 0.f);
                MeshComponent.Vertices.emplace_back(Positions[i+1].X,    Positions[i+1].Y,    Positions[i+1].Z,
                                                Normal.X,                   Normal.Y,                   Normal.Z,
                                                0.f, 0.f);
                MeshComponent.Vertices.emplace_back(Positions[i+2].X,    Positions[i+2].Y,    Positions[i+2].Z,
                                                Normal.X,                   Normal.Y,                   Normal.Z,
                                                0.f, 0.f);
            }
        }
    }
}