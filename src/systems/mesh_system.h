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
            void Init(int NumberOfSimultaneousSubmits);
            void Cleanup();

            void LoadMesh(FEntity Entity, const std::string &Path);
            void CreateTetrahedron(FEntity Entity);
            void CreateHexahedron(FEntity Entity);
            void CreateIcosahedron(FEntity Entity, uint32_t Depth, bool bGeometricNormals);
            void CreatePlane(FEntity Entity);
            uint32_t Size();

            VkDeviceAddress GetVertexBufferAddress(FEntity Entity);
            VkDeviceAddress GetIndexBufferAddress(FEntity Entity);
            void LoadToGPU(FEntity Entity);

            void UpdateAS();
            std::vector<FAccelerationStructure> BLASVector;
            FAccelerationStructure TLAS;

            VkDeviceSize VertexBufferSize = uint64_t(2) * 1024 * 1024 * 1024;
            VkDeviceSize IndexBufferSize = uint64_t(1) * 1024 * 1024 * 1024;

            FBuffer VertexBuffer;
            FBuffer IndexBuffer;
        };
    }
}

#define MESH_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMeshSystem>()