#pragma once

#include "buffer.h"

#include "system.h"
#include "coordinator.h"

namespace ECS
{
    namespace SYSTEMS
    {
        class FMaterialSystem : public FSystem
        {
        public:
            void Init(int NumberOfSimultaneousSubmits);
            void Update();
            FMaterialSystem& SetBaseAlbedo(FEntity MaterialEntity, float Red, float Green, float Blue);

            void RequestAllUpdate();
            void RequestUpdate(int FrameIndex);

            int GetTotalSize();


        public:
            std::vector<bool> BufferPartThatNeedsUpdate;
            int NumberOfSimultaneousSubmits = 2;

            FBuffer DeviceMaterialBuffer;
        };
    }
}

#define MATERIAL_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMaterialSystem>()
