#pragma once

#include "gpu_bufferable_system.h"

#include "buffer.h"

#include "maths.h"

#include "vk_acceleration_structure.h"

#include <queue>

namespace ECS
{
    namespace SYSTEMS
    {
        class FAccelerationStructureSystem : public FGPUBufferableSystem
        {
        public:
            void Init(uint32_t NumberOfSimultaneousSubmits);
            void Update();
            void Update(int Index);
            void Terminate();

            FEntity CreateInstance(FEntity Entity, const FVector3& Position, const FVector3& Direction, const FVector3& Up);
			void UpdateInstancePosition(FEntity Entity);
            void UpdateTLAS();

            const uint32_t MAX_INSTANCE_COUNT = 512u * 1024u;
            std::queue<uint32_t> AvailableIndices;

            FAccelerationStructure TLAS;

            uint32_t InstanceCount = 0u;

            bool bIsDirty = true;
        };
    }
}

#define ACCELERATION_STRUCTURE_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FAccelerationStructureSystem>()