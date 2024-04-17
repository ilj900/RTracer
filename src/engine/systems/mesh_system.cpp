#include "tiny_obj_loader.h"

#include "mesh_component.h"
#include "device_mesh_component.h"
#include "device_renderable_component.h"
#include "acceleration_structure_component.h"
#include "mesh_system.h"
#include "renderable_system.h"

namespace ECS
{
    namespace SYSTEMS
    {
        void FMeshSystem::Init()
        {
            VertexBuffer = GetResourceAllocator()->CreateBuffer(VertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Vertex_Buffer");
            IndexBuffer = GetResourceAllocator()->CreateBuffer(IndexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Index_Buffer");

            GetResourceAllocator()->RegisterBuffer(VertexBuffer, "Vertex_Buffer");
            GetResourceAllocator()->RegisterBuffer(IndexBuffer, "Index_Buffer");
        }

        VkDeviceAddress FMeshSystem::GetVertexBufferAddress(FEntity Renderable)
        {
            auto& DeviceRenderableComponent = RENDERABLE_SYSTEM()->GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Renderable);
            auto& DeviceMeshComponent = GetComponent<ECS::COMPONENTS::FDeviceMeshComponent>(DeviceRenderableComponent.MeshIndex);

            return VK_CONTEXT()->GetBufferDeviceAddressInfo(VertexBuffer) + DeviceMeshComponent.VertexPtr.Offset;
        }

        VkDeviceAddress FMeshSystem::GetIndexBufferAddress(FEntity Renderable)
        {
            auto& DeviceRenderableComponent = RENDERABLE_SYSTEM()->GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Renderable);
            auto& DeviceMeshComponent = GetComponent<ECS::COMPONENTS::FDeviceMeshComponent>(DeviceRenderableComponent.MeshIndex);

            return VK_CONTEXT()->GetBufferDeviceAddressInfo(IndexBuffer) + DeviceMeshComponent.IndexPtr.Offset;
        }

        void FMeshSystem::LoadToGPU(FEntity Entity)
        {
            auto& MeshComponent = GetComponent<ECS::COMPONENTS::FMeshComponent>(Entity);
            auto& DeviceMeshComponent = GetComponent<ECS::COMPONENTS::FDeviceMeshComponent>(Entity);

            uint32_t VertexBufferRequiredSize = MeshComponent.Vertices.size() * sizeof(FVertex);
            uint32_t IndexBufferRequiredSize = MeshComponent.Indices.size() * sizeof(uint32_t);

            FMemoryPtr VertexBufferChunk = VertexBuffer.CheckAvailableMemory(VertexBufferRequiredSize);
            VertexBuffer.ReserveMemory(VertexBufferChunk);

            FMemoryPtr IndexBufferChunk{};

            if (MeshComponent.Indexed)
            {
                IndexBufferChunk = IndexBuffer.CheckAvailableMemory(IndexBufferRequiredSize);
                IndexBuffer.ReserveMemory(IndexBufferChunk);
            }

            DeviceMeshComponent.VertexPtr = VertexBufferChunk;

            RESOURCE_ALLOCATOR()->LoadDataToBuffer(VertexBuffer, {DeviceMeshComponent.VertexPtr.Size}, {DeviceMeshComponent.VertexPtr.Offset}, {MeshComponent.Vertices.data()});

            if (MeshComponent.Indexed)
            {
                DeviceMeshComponent.IndexPtr = IndexBufferChunk;
                RESOURCE_ALLOCATOR()->LoadDataToBuffer(IndexBuffer, {DeviceMeshComponent.IndexPtr.Size}, {DeviceMeshComponent.IndexPtr.Offset}, {MeshComponent.Indices.data()});
            }
        }

        void FMeshSystem::Terminate()
        {
            for (auto Entity : Entities)
            {
                DeleteBLAS(Entity);
            }
        }

        void FMeshSystem::GenerateBLAS(FEntity Entity)
        {
            COORDINATOR().AddComponent<ECS::COMPONENTS::FAccelerationStructureComponent>(Entity, {});
            auto& AccelerationStructureComponent = GetComponent<ECS::COMPONENTS::FAccelerationStructureComponent>(Entity);

            auto& MeshComponent = GetComponent<ECS::COMPONENTS::FMeshComponent>(Entity);
            auto& DeviceMeshComponent = GetComponent<ECS::COMPONENTS::FDeviceMeshComponent>(Entity);
            AccelerationStructureComponent =  VK_CONTEXT()->GenerateBlas(MESH_SYSTEM()->VertexBuffer, MESH_SYSTEM()->IndexBuffer,
                                                                        sizeof (FVertex), MeshComponent.Indexed ? MeshComponent.Indices.size() : MeshComponent.Vertices.size(),
                                                                        DeviceMeshComponent.VertexPtr, DeviceMeshComponent.IndexPtr);
        }

        void FMeshSystem::DeleteBLAS(FEntity Entity)
        {
            auto& AccelerationStructureComponent = GetComponent<ECS::COMPONENTS::FAccelerationStructureComponent>(Entity);
            VK_CONTEXT()->DestroyAccelerationStructure(AccelerationStructureComponent.AccelerationStructure);
            AccelerationStructureComponent.AccelerationStructure.AccelerationStructure = VK_NULL_HANDLE;
            AccelerationStructureComponent.AccelerationStructure.Buffer.Buffer = VK_NULL_HANDLE;
        }

        uint32_t FMeshSystem::Size()
        {
            return Entities.size();
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

            for (const auto& Shape : Shapes)
            {
                for (const auto& Index : Shape.mesh.indices)
                {
                    ECS::COMPONENTS::FVertexComponent Vert{};

                    Vert.Position = {
                            Attrib.vertices[3 * Index.vertex_index + 0],
                            Attrib.vertices[3 * Index.vertex_index + 1],
                            Attrib.vertices[3 * Index.vertex_index + 2]
                    };

                    if (!Attrib.normals.empty())
                    {
                        Vert.Normal =
                        {
                            Attrib.normals[3 * Index.normal_index + 0],
                            Attrib.normals[3 * Index.normal_index + 1],
                            Attrib.normals[3 * Index.normal_index + 2]
                        };
                    }

                    if (!Attrib.texcoords.empty())
                    {
                        Vert.TexCoord =
                        {
                        Attrib.texcoords[2 * Index.texcoord_index + 0],
                        1.f - Attrib.texcoords[2 * Index.texcoord_index + 1],
                        };
                    }

                    MeshComponent.Indices.push_back(static_cast<uint32_t>(MeshComponent.Vertices.size()));
                    MeshComponent.Vertices.push_back(Vert);
                }
            }

            MeshComponent.Indexed = true;
        }

        void FMeshSystem::CreateTetrahedron(FEntity Entity)
        {
            auto& MeshComponent = GetComponent<ECS::COMPONENTS::FMeshComponent>(Entity);

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
                auto Normal = Cross(V1, V2).GetNormalized();
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
            auto& MeshComponent = COORDINATOR().GetComponent<ECS::COMPONENTS::FMeshComponent>(Entity);

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

            std::vector<FVector2> TextureCoordinates(4);
            TextureCoordinates[0] = {0, 0};
            TextureCoordinates[1] = {1, 0};
            TextureCoordinates[2] = {1, 1};
            TextureCoordinates[3] = {0, 1};

            std::vector<uint32_t> Indices = {3, 2, 1, 3, 1, 0,
                                             2, 6, 5, 2, 5, 1,
                                             6, 7, 4, 6, 4, 5,
                                             7, 3, 0, 7, 0, 4,
                                             2, 3, 7, 2, 7, 6,
                                             5, 4, 0, 5, 0, 1};

            for(uint32_t i = 0; i < Indices.size(); i += 3)
            {
                std::vector<uint32_t> EvenTextureIndices = {2, 3, 0};
                std::vector<uint32_t> OddTextureIndices = {2, 0, 1};
                auto& TextureIndices = EvenTextureIndices;
                if (i % 2 == 1)
                {
                    TextureIndices = OddTextureIndices;
                }

                auto V1 = Positions[Indices[i+1]] - Positions[Indices[i]];
                auto V2 = Positions[Indices[i+2]] - Positions[Indices[i]];
                auto Normal = Cross(V1, V2).GetNormalized();
                MeshComponent.Vertices.emplace_back(Positions[Indices[i]].X, Positions[Indices[i]].Y, Positions[Indices[i]].Z,
                                                    Normal.X, Normal.Y, Normal.Z,
                                                    TextureCoordinates[TextureIndices[i % 3]].X, TextureCoordinates[TextureIndices[i % 3]].Y);
                MeshComponent.Vertices.emplace_back(Positions[Indices[i+1]].X, Positions[Indices[i+1]].Y, Positions[Indices[i+1]].Z,
                                                    Normal.X, Normal.Y, Normal.Z,
                                                    TextureCoordinates[TextureIndices[(i  + 1) % 3]].X, TextureCoordinates[TextureIndices[(i + 1) % 3]].Y);
                MeshComponent.Vertices.emplace_back(Positions[Indices[i+2]].X, Positions[Indices[i+2]].Y, Positions[Indices[i+2]].Z,
                                                    Normal.X, Normal.Y, Normal.Z,
                                                    TextureCoordinates[TextureIndices[(i + 2) % 3]].X, TextureCoordinates[TextureIndices[(i + 2) % 3]].Y);
            }
        }

        void FMeshSystem::CreateIcosahedron(FEntity Entity, float Radius,  uint32_t Depth, bool Jagged)
        {
            auto& MeshComponent = GetComponent<ECS::COMPONENTS::FMeshComponent>(Entity);

            float X = 0.52573111211f * Radius;
            float Z = 0.85065080835f * Radius;
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

                    FVector3 AB = (A + B).GetNormalized() * Radius;
                    FVector3 AC = (A + C).GetNormalized() * Radius;
                    FVector3 BC = (B + C).GetNormalized() * Radius;

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
                FVector3 Normal0{};
				FVector3 Normal1{};
				FVector3 Normal2{};

                if (Jagged)
                {
                    auto V1 = Positions[i + 1] - Positions[i];
                    auto V2 = Positions[i + 2] - Positions[i];
                    Normal0 = Cross(V1, V2).GetNormalized();
					Normal1 = Normal0;
					Normal2 = Normal0;
                }
                else
                {
                    Normal0 = Positions[i].GetNormalized();
					Normal1 = Positions[i + 1].GetNormalized();
					Normal2 = Positions[i + 2].GetNormalized();
                }

                auto TransformXZ = [](float InX, float InZ)
                {
                    float Angle = ((asin(InX)) + 1.f) / 2.f;
                    if (InZ > 0.f)
                    {
                        Angle += 1.f;
                    }
                    return Angle / 2.f;
                };

                auto TransformY = [](float In)
                {
                    return ((asin(In)) + 1.f) / 2.f;
                };

                MeshComponent.Vertices.emplace_back(Positions[i].X,    Positions[i].Y,    Positions[i].Z,
                                                Normal0.X,           Normal0.Y,                   Normal0.Z,
                                                TransformXZ(Positions[i].Y, Positions[i].Z), TransformY(Positions[i].X));
                MeshComponent.Vertices.emplace_back(Positions[i+1].X,    Positions[i+1].Y,    Positions[i+1].Z,
                                                Normal1.X,                   Normal1.Y,                   Normal1.Z,
                                                    TransformXZ(Positions[i+1].Y, Positions[i+1].Z), TransformY(Positions[i+1].X));
                MeshComponent.Vertices.emplace_back(Positions[i+2].X,    Positions[i+2].Y,    Positions[i+2].Z,
                                                Normal2.X,                   Normal2.Y,                   Normal2.Z,
                                                    TransformXZ(Positions[i+2].Y, Positions[i+2].Z), TransformY(Positions[i+2].X));
            }
        }

		void FMeshSystem::CreateUVSphere(FEntity Entity, uint32_t LongitudeCount, uint32_t LatitudeCount)
		{
			auto& MeshComponent = GetComponent<ECS::COMPONENTS::FMeshComponent>(Entity);
			auto& Vertices = MeshComponent.Vertices;
			Vertices.resize((LongitudeCount + 1) * (LatitudeCount - 1) + 2);

			Vertices[0] = {0, 1, 0, 0, 1, 0, 0, 0};

			float LongitudeAngleStep = M_2_PI / float(LongitudeCount);
			float LatitudeAngleStep = M_PI / float(LatitudeCount);
			float UVLongitudeStep = 1.f / float(LongitudeCount);
			float UVLatitudeStep = 1.f / float(LatitudeCount);
			float CurrentLatitudeAngle = M_PI_2;
			FVector2 UV = {};

			for (int i = 0; i < (LatitudeCount - 1); ++i)
			{
				CurrentLatitudeAngle -= LatitudeAngleStep;
				float CurrentLongitudeAngle = 0;
				UV.Y += UVLatitudeStep;
				UV.X = 0;

				for (int j = 0; j < (LongitudeCount + 1); ++j)
				{
					float X = abs(cos(CurrentLatitudeAngle)) * sin(CurrentLongitudeAngle);
					float Y = sin(CurrentLatitudeAngle);
					float Z = abs(cos(CurrentLatitudeAngle)) * cos(CurrentLongitudeAngle);
					FVector3 Coordinates = {X, Y, Z};
					uint32_t Index = i * (LongitudeCount + 1) + j + 1;
					Vertices[Index] = {Coordinates.X, Coordinates.Y, Coordinates.Z, Coordinates.X, Coordinates.Y, Coordinates.Z, UV.X, UV.Y};

					CurrentLongitudeAngle += LongitudeAngleStep;
					UV.X += UVLongitudeStep;
				}
			}

			Vertices.back() = {0, -1, 0, 0, -1, 0, 1, 1};

			MeshComponent.Indexed = true;
			auto& Indices = MeshComponent.Indices;
			Indices.resize(LongitudeCount * (LatitudeCount - 1) * 6);

			{
				/// Emplace upper cap of the sphere
				int i = 0;

				for (; i < LongitudeCount; ++i)
				{
					Indices[i * 3] = 0;
					Indices[i * 3 + 1] = i + 1;
					Indices[i * 3 + 2] = i + 2;
				}
			}

			uint32_t StartingIndex = LongitudeCount * 3;
			uint32_t StartingValue = 1;

			for (int j = 0; j < (LatitudeCount - 2); ++j)
			{
				int i = 0;

				for (; i < LongitudeCount - 1; ++i)
				{
					uint32_t Index = ((j * LongitudeCount) + i) * 6;
					Indices[StartingIndex + Index] = StartingValue + (j * (LongitudeCount + 1) + i);
					Indices[StartingIndex + Index + 1] = StartingValue + ((j + 1) * (LongitudeCount + 1) + i);
					Indices[StartingIndex + Index + 2] = StartingValue + (j * (LongitudeCount + 1) + i + 1);
					Indices[StartingIndex + Index + 3] = StartingValue + ((j + 1) * (LongitudeCount + 1) + i);
					Indices[StartingIndex + Index + 4] = StartingValue + ((j + 1) * (LongitudeCount + 1) + i + 1);
					Indices[StartingIndex + Index + 5] = StartingValue + (j * (LongitudeCount + 1) + i + 1);
				}

				uint32_t Index = ((j * LongitudeCount) + i) * 6;
				Indices[StartingIndex + Index] = StartingValue + (j * (LongitudeCount + 1) + i);
				Indices[StartingIndex + Index + 1] = StartingValue + ((j + 1) * (LongitudeCount + 1) + i);
				Indices[StartingIndex + Index + 2] = StartingValue + (j * (LongitudeCount + 1) + i + 1);
				Indices[StartingIndex + Index + 3] = StartingValue + ((j + 1) * (LongitudeCount + 1) + i);
				Indices[StartingIndex + Index + 4] = StartingValue + ((j + 1) * (LongitudeCount + 1) + i + 1);
				Indices[StartingIndex + Index + 5] = StartingValue + (j * (LongitudeCount + 1) + i + 1);
			}

			{
				StartingIndex += (LatitudeCount - 2) * LongitudeCount * 6;
				StartingValue += (LatitudeCount - 2) * (LongitudeCount + 1);

				uint32_t i = 0;

				for (; i < LongitudeCount; ++i)
				{
					Indices[StartingIndex + i * 3] = StartingValue + i;
					Indices[StartingIndex + i * 3 + 1] = Vertices.size() - 1;
					Indices[StartingIndex + i * 3 + 2] = StartingValue + i + 1;
				}
			}
		}

        void FMeshSystem::CreatePlane(FEntity Entity, const FVector2& Size)
        {
            auto& MeshComponent = GetComponent<ECS::COMPONENTS::FMeshComponent>(Entity);

            MeshComponent.Vertices.resize(4);
            MeshComponent.Vertices[0] = {-Size.X / 2.f, -Size.Y / 2.f, 0, 0, 0, 1, 0, 0};
            MeshComponent.Vertices[1] = { Size.X / 2.f, -Size.Y / 2.f, 0, 0, 0, 1, 1, 0};
            MeshComponent.Vertices[2] = { Size.X / 2.f,  Size.Y / 2.f, 0, 0, 0, 1, 1, 1};
            MeshComponent.Vertices[3] = {-Size.X / 2.f,  Size.Y / 2.f, 0, 0, 0, 1, 0, 1};

            MeshComponent.Indexed = true;
            MeshComponent.Indices.resize(6);
            MeshComponent.Indices = {0, 3, 2, 0, 2, 1};
        }
    }
}