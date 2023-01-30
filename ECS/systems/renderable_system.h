#pragma once

#include "system.h"
#include "coordinator.h"

#include "buffer.h"

namespace ECS
{
    namespace SYSTEMS
    {
        class FRenderableSystem : public FSystem
        {
        private:
            template<typename T>
            T& GetComponent(FEntity Entity);

        public:
            void Init(int NumberOfSimultaneousSubmits);
            void Update(int IterationIndex);
            void SetRenderableColor(FEntity Entity, float Red, float Green, float Blue);
            void SetSelected(FEntity Entity);
            void SetSelectedByIndex(uint32_t Index);
            void SetNotSelected(FEntity Entity);

        public:
            bool bNeedsUpdate = false;
            int NumberOfSimultaneousSubmits = 2;

            FBuffer DeviceRenderableBuffer;
        };
    }
}

#define RENDERABLE_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FRenderableSystem>()