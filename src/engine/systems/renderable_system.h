#pragma once

#include "gpu_bufferable_system.h"
#include "coordinator.h"

#include "buffer.h"

namespace ECS
{
    namespace SYSTEMS
    {
        class FRenderableSystem : public FGPUBufferableSystem
        {
        public:
            void Init(uint32_t NumberOfSimultaneousSubmits) override;
            void Update() override;
            void Update(int Index) override;

            void SetRenderableColor(FEntity Entity, float Red, float Green, float Blue);
            void SetSelected(FEntity Entity);
            void SetSelectedByIndex(uint32_t Index);
            void SetNotSelected(FEntity Entity);
            void SetMaterial(FEntity Renderable, FEntity Material);
            void SetIndexed(FEntity Entity);
            void SyncTransform(FEntity Entity);
            void SetRenderableHasTexture(FEntity Entity);
            void SetNotIndex(FEntity Entity);
            void SetRenderableDeviceAddress(FEntity Entity, VkDeviceAddress VertexDeviceAddress, VkDeviceAddress IndexDeviceAddress);

            const uint32_t MAX_RENDERABLES = 512u * 1024u; /// TODO
        };
    }
}

#define RENDERABLE_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FRenderableSystem>()