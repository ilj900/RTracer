#pragma once

#include "system.h"

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
            void SetRenderableColor(FEntity Entity, float Red, float Green, float Blue);
            void SetSelected(FEntity Entity);
            void SetSelectedByIndex(uint32_t Index);
            void SetNotSelected(FEntity Entity);
        };
    }
}