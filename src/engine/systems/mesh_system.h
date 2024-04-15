#pragma once

#include "system.h"

#include "vk_acceleration_structure.h"
#include "vulkan/vulkan.h"

namespace ECS
{
    namespace SYSTEMS
    {
        class FMeshSystem : public FSystem
        {
        public:
            void Init();

            void LoadMesh(FEntity Entity, const std::string &Path);
            void CreateTetrahedron(FEntity Entity);
            void CreateHexahedron(FEntity Entity);
            void CreateIcosahedron(FEntity Entity, uint32_t Depth, bool Jagged);
			void CreateUVSphere(FEntity Entity, uint32_t LongitudeCount, uint32_t LatitudeCount);
            void CreatePlane(FEntity Entity, const FVector2& Size);
            uint32_t Size();

            VkDeviceAddress GetVertexBufferAddress(FEntity Renderable);
            VkDeviceAddress GetIndexBufferAddress(FEntity Renderable);
            void LoadToGPU(FEntity Entity);
            void Terminate();
            void GenerateBLAS(FEntity Entity);
            void DeleteBLAS(FEntity Entity);

            VkDeviceSize VertexBufferSize = uint64_t(2) * 1024 * 1024 * 1024;
            VkDeviceSize IndexBufferSize = uint64_t(1) * 1024 * 1024 * 1024;

            FBuffer VertexBuffer;
            FBuffer IndexBuffer;
        };
    }
}

#define MESH_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMeshSystem>()