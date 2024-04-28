#include "device_renderable_component.h"
#include "device_transform_component.h"
#include "material_component.h"
#include "renderable_system.h"
#include "transform_system.h"
#include "mesh_system.h"

namespace ECS
{
    namespace SYSTEMS
    {
        void FRenderableSystem::Init(uint32_t NumberOfSimultaneousSubmits)
        {
            FGPUBufferableSystem::Init(NumberOfSimultaneousSubmits, sizeof(ECS::COMPONENTS::FDeviceRenderableComponent) * MAX_RENDERABLES,
                                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, "Device_Renderable_Buffer");
        }

        bool FRenderableSystem::Update()
        {
			bool bAnyUpdate = false;

			for (auto& Entry : EntitiesToUpdate)
			{
				bAnyUpdate |= !Entry.empty();
			}

			if (bAnyUpdate)
			{
				FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FDeviceRenderableComponent>();
			}

			return bAnyUpdate;
        }

        bool FRenderableSystem::Update(int Index)
        {
			bool bAnyUpdate = false;

			for (auto& Entry : EntitiesToUpdate)
			{
				bAnyUpdate |= !Entry.empty();
			}

			if (bAnyUpdate)
			{
				FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FDeviceRenderableComponent>(Index);
			}

			return bAnyUpdate;
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

        void FRenderableSystem::SetMaterial(FEntity Renderable, FEntity Material)
        {
            auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Renderable);
            RenderableComponent.MaterialIndex = COORDINATOR().GetIndex<ECS::COMPONENTS::FMaterialComponent>(Material);
            MarkDirty(Renderable);
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
            TRANSFORM_SYSTEM()->SyncTransform(Entity);
            RenderableComponent.TransformIndex = COORDINATOR().GetIndex<ECS::COMPONENTS::FDeviceTransformComponent>(Entity);
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