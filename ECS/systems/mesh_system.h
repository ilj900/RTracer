#pragma once

#include "system.h"

#include "vulkan/vulkan.h"

namespace ECS
{
    namespace SYSTEMS
    {
        class FMeshSystem : public FSystem
        {
        private:
            template<typename T>
            T& GetComponent(FEntity Entity);

        public:
            void LoadMesh(FEntity Entity, const std::string &Path);
            void CreateTetrahedron(FEntity Entity);
            void CreateHexahedron(FEntity Entity);
            void CreateIcosahedron(FEntity Entity, uint32_t Depth);
            std::set<FEntity>::iterator begin();
            std::set<FEntity>::iterator  end();
            uint32_t Size();

            void Draw(FEntity Entity, VkCommandBuffer CommandBuffer);
            void Bind(FEntity Entity, VkCommandBuffer CommandBuffer);
            void LoadToGPU(FEntity Entity);
            void FreeAllDeviceData();
        };
    }
}