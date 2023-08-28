#include "components/device_renderable_component.h"
#include "systems/renderable_system.h"
#include "mesh_system.h"
#include "coordinator.h"

#include "vk_context.h"

#include <cassert>

namespace ECS
{
    namespace SYSTEMS
    {
        void FRenderableSystem::Init(int NumberOfSimultaneousSubmits)
        {
            FGPUBufferableSystem::Init(NumberOfSimultaneousSubmits, sizeof(ECS::COMPONENTS::FDeviceRenderableComponent) * MAX_RENDERABLES,
                                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, "Device_Light_Buffer");
        }

        void FRenderableSystem::Update()
        {
            FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FDeviceRenderableComponent>();
        }

        void FRenderableSystem::Update(int Index)
        {
            FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FDeviceRenderableComponent>(Index);
        }

        void FRenderableSystem::SetRenderableColor(FEntity Entity, float Red, float Green, float Blue)
        {
            auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Entity);
            RenderableComponent.RenderableColor = {Red, Green, Blue};
        }

        void FRenderableSystem::SetSelected(FEntity Entity)
        {
            for (auto E : Entities)
            {
                auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(E);
                if (Entity == E)
                {
                    RenderableComponent.RenderablePropertyMask |= COMPONENTS::RENDERABLE_SELECTED_BIT;
                }
                else
                {
                    RenderableComponent.RenderablePropertyMask &= ~COMPONENTS::RENDERABLE_SELECTED_BIT;
                }
            }
        }

        void FRenderableSystem::SetSelectedByIndex(uint32_t Index)
        {
            for (auto Entity : Entities)
            {
                auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Entity);
                if (RenderableComponent.RenderableIndex == Index)
                {
                    RenderableComponent.RenderablePropertyMask |= COMPONENTS::RENDERABLE_SELECTED_BIT;
                }
                else
                {
                    RenderableComponent.RenderablePropertyMask &= ~COMPONENTS::RENDERABLE_SELECTED_BIT;
                }
            }
        }

        void FRenderableSystem::SetNotSelected(FEntity Entity)
        {
            auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Entity);
            RenderableComponent.RenderablePropertyMask &= ~COMPONENTS::RENDERABLE_SELECTED_BIT;
        }

        void FRenderableSystem::SetIndexed(FEntity Entity)
        {
            auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Entity);
            RenderableComponent.RenderablePropertyMask |= COMPONENTS::RENDERABLE_IS_INDEXED;
        }

        void FRenderableSystem::SetRenderableHasTexture(FEntity Entity)
        {
            auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Entity);
            RenderableComponent.RenderablePropertyMask |= COMPONENTS::RENDERABLE_HAS_TEXTURE;
        }

        void FRenderableSystem::SetNotIndex(FEntity Entity)
        {
            auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Entity);
            RenderableComponent.RenderablePropertyMask &= ~COMPONENTS::RENDERABLE_IS_INDEXED;
        }

        void FRenderableSystem::SetRenderableDeviceAddress(FEntity Entity, VkDeviceAddress VertexDeviceAddress, VkDeviceAddress IndexDeviceAddress)
        {
            auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Entity);

            RenderableComponent.VertexBufferAddress = VertexDeviceAddress;
            RenderableComponent.IndexBufferAddress = IndexDeviceAddress;
        }
    }
}