#pragma once

#include "system.h"

#include "buffer.h"
#include "vulkan/vulkan.h"

namespace ECS
{
    namespace SYSTEMS
    {
        class FMeshSystem : public FSystem
        {
        public:
            void Init(int NumberOfSimultaneousSubmits);

            void LoadMesh(FEntity Entity, const std::string &Path);
            void CreateTetrahedron(FEntity Entity);
            void CreateHexahedron(FEntity Entity);
            void CreateIcosahedron(FEntity Entity, uint32_t Depth);
            void CreatePlane(FEntity Entity);
            uint32_t Size();

            void Draw(FEntity Entity, VkCommandBuffer CommandBuffer);
            void Bind(FEntity Entity, VkCommandBuffer CommandBuffer);
            VkDeviceAddress GetVertexBufferAddress(FEntity Entity);
            VkDeviceAddress GetIndexBufferAddress(FEntity Entity);
            void LoadToGPU(FEntity Entity);

            VkDeviceSize VertexBufferSize = uint64_t(2) * 1024 * 1024 * 1024;
            VkDeviceSize IndexBufferSize = uint64_t(1) * 1024 * 1024 * 1024;

            FBuffer VertexBuffer;
            FBuffer IndexBuffer;
        };
    }
}

#define MESH_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMeshSystem>()