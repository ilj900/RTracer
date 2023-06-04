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
            void Update();
            void SetRenderableColor(FEntity Entity, float Red, float Green, float Blue);
            void SetSelected(FEntity Entity);
            void SetSelectedByIndex(uint32_t Index);
            void SetNotSelected(FEntity Entity);
            void SetIndexed(FEntity Entity);
            void SetNotIndex(FEntity Entity);
            void SetRenderableDeviceAddress(FEntity Entity, VkDeviceAddress VertexDeviceAddress, VkDeviceAddress IndexDeviceAddress);
            void RequestAllUpdate();
            void RequestUpdate(int FrameIndex);

            int GetTotalSize();

        public:
            std::vector<bool> BufferPartThatNeedsUpdate;
            int NumberOfSimultaneousSubmits = 2;
            bool bIsDirty = false;

            FBuffer DeviceRenderableBuffer;
        };
    }
}

#define RENDERABLE_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FRenderableSystem>()