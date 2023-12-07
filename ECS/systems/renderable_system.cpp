#include "components/device_renderable_component.h"
#include "components/device_transform_component.h"
#include "systems/renderable_system.h"
#include "systems/transform_system.h"
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
                                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, "Device_Renderable_Buffer");
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

            MarkDirty(Entity);
        }

        void FRenderableSystem::SetSelected(FEntity Entity)
        {
            for (auto E : Entities)
            {
                auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(E);
                if (Entity == E)
                {
                    RenderableComponent.RenderablePropertyMask |= RENDERABLE_SELECTED_BIT;
                }
                else
                {
                    RenderableComponent.RenderablePropertyMask &= ~RENDERABLE_SELECTED_BIT;
                }
            }

            MarkDirty(Entity);
        }

        void FRenderableSystem::SetSelectedByIndex(uint32_t Index)
        {
            FEntity Entity;
            for (auto EntityEntry : Entities)
            {
                auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(EntityEntry);
                if (RenderableComponent.RenderableIndex == Index)
                {
                    Entity = EntityEntry;
                    RenderableComponent.RenderablePropertyMask |= RENDERABLE_SELECTED_BIT;
                }
                else
                {
                    RenderableComponent.RenderablePropertyMask &= ~RENDERABLE_SELECTED_BIT;
                }
            }

            MarkDirty(Entity);
        }

        void FRenderableSystem::SetNotSelected(FEntity Entity)
        {
            auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Entity);
            RenderableComponent.RenderablePropertyMask &= ~RENDERABLE_SELECTED_BIT;
            MarkDirty(Entity);
        }

        void FRenderableSystem::SetIndexed(FEntity Entity)
        {
            auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Entity);
            RenderableComponent.RenderablePropertyMask |= RENDERABLE_IS_INDEXED;
            MarkDirty(Entity);
        }

        void FRenderableSystem::SyncTransform(FEntity Entity)
        {
            auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Entity);
            auto& DeviceTransformComponent = GetComponent<ECS::COMPONENTS::FDeviceTransformComponent>(Entity);
            DeviceTransformComponent.ModelMatrix = TRANSFORM_SYSTEM()->GetModelMatrix(Entity);
            RenderableComponent.TransformIndex = GetCoordinator().GetIndex<ECS::COMPONENTS::FDeviceTransformComponent>(Entity);
            MarkDirty(Entity);
        }

        void FRenderableSystem::SetRenderableHasTexture(FEntity Entity)
        {
            auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Entity);
            RenderableComponent.RenderablePropertyMask |= RENDERABLE_HAS_TEXTURE;
            MarkDirty(Entity);
        }

        void FRenderableSystem::SetNotIndex(FEntity Entity)
        {
            auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Entity);
            RenderableComponent.RenderablePropertyMask &= ~RENDERABLE_IS_INDEXED;
            MarkDirty(Entity);
        }

        void FRenderableSystem::SetRenderableDeviceAddress(FEntity Entity, VkDeviceAddress VertexDeviceAddress, VkDeviceAddress IndexDeviceAddress)
        {
            auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Entity);

            RenderableComponent.VertexBufferAddress = VertexDeviceAddress;
            RenderableComponent.IndexBufferAddress = IndexDeviceAddress;
            MarkDirty(Entity);
        }
    }
}