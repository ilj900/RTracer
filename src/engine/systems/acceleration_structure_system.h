#pragma once

#include "system.h"

#include "buffer.h"

#include "maths.h"

#include "vk_acceleration_structure.h"

#include <queue>

namespace ECS
{
    namespace SYSTEMS
    {
        class FAccelerationStructureSystem : public FSystem
        {
        public:
            void Init(int NumberOfSimultaneousSubmits);
            void Update();

            void GenerateBLAS(FEntity Entity);
            FEntity CreateInstance(FEntity Entity, const FVector3& Position);

            void UpdateTLAS();

            const uint32_t MAX_INSTANCE_COUNT = 512u * 1024u;
            std::queue<uint32_t> AvailableIndices;

            FBuffer BLASInstanceBuffer;
            FAccelerationStructure TLAS;

            uint32_t InstanceCount = 0u;

            std::set<FEntity> EntitiesToUpdate;
            bool bIsDirty = true;
        };
    }
}

#define ACCELERATION_STRUCTURE_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FAccelerationStructureSystem>()