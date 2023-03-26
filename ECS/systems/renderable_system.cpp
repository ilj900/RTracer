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
        template<typename T>
        inline T& FRenderableSystem::GetComponent(FEntity Entity)
        {
            assert(Entities.find(Entity) != Entities.end() && "Entity doesn't have camera component");
            auto& Coordinator = GetCoordinator();
            auto& RenderableComponent = Coordinator.GetComponent<T>(Entity);
            return RenderableComponent;
        }

        void FRenderableSystem::Init(int NumberOfSimultaneousSubmits)
        {
            this->NumberOfSimultaneousSubmits = NumberOfSimultaneousSubmits;
            auto& Coordinator = GetCoordinator();
            auto& Context = GetContext();
            auto DeviceRenderableComponentsData = Coordinator.Data<ECS::COMPONENTS::FDeviceRenderableComponent>();
            auto DeviceRenderableComponentsSize = Coordinator.Size<ECS::COMPONENTS::FDeviceRenderableComponent>();

            VkDeviceSize RenderableBufferSize = DeviceRenderableComponentsSize * NumberOfSimultaneousSubmits;

            DeviceRenderableBuffer = GetContext().CreateBuffer(RenderableBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "Device_Renderable_Buffer");

            for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
            {
                Context.ResourceAllocator->LoadDataToBuffer(DeviceRenderableBuffer, DeviceRenderableComponentsSize, DeviceRenderableComponentsSize * i, DeviceRenderableComponentsData);
            }
        }

        void FRenderableSystem::Update(int IterationIndex)
        {
            if (bNeedsUpdate)
            {
                auto& Coordinator = GetCoordinator();
                auto& Context = GetContext();
                auto DeviceRenderableComponentsData = Coordinator.Data<ECS::COMPONENTS::FDeviceRenderableComponent>();
                auto DeviceRenderableComponentsSize = Coordinator.Size<ECS::COMPONENTS::FDeviceRenderableComponent>();

                Context.ResourceAllocator->LoadDataToBuffer(DeviceRenderableBuffer, DeviceRenderableComponentsSize, DeviceRenderableComponentsSize * IterationIndex, DeviceRenderableComponentsData);

                std::vector<ECS::COMPONENTS::FDeviceRenderableComponent> Backed;
                Backed.resize(8);

                Context.ResourceAllocator->LoadDataFromBuffer(DeviceRenderableBuffer, DeviceRenderableComponentsSize, 0, Backed.data());
            }

            bNeedsUpdate--;
        }

        void FRenderableSystem::SetRenderableColor(FEntity Entity, float Red, float Green, float Blue)
        {
            auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Entity);
            RenderableComponent.RenderableColor = {Red, Green, Blue};

            bNeedsUpdate = 2;
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

            bNeedsUpdate = 2;
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

            bNeedsUpdate = 2;
        }

        void FRenderableSystem::SetNotSelected(FEntity Entity)
        {
            auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Entity);
            RenderableComponent.RenderablePropertyMask &= ~COMPONENTS::RENDERABLE_SELECTED_BIT;

            bNeedsUpdate = true;
        }

        void FRenderableSystem::SetIndexed(FEntity Entity)
        {
            auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Entity);
            RenderableComponent.RenderablePropertyMask |= COMPONENTS::RENDERABLE_IS_INDEXED;

            bNeedsUpdate = 2;
        }

        void FRenderableSystem::SetNotIndex(FEntity Entity)
        {
            auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Entity);
            RenderableComponent.RenderablePropertyMask &= ~COMPONENTS::RENDERABLE_IS_INDEXED;

            bNeedsUpdate = 2;
        }

        void FRenderableSystem::SetRenderableDeviceAddress(FEntity Entity, VkDeviceAddress VertexDeviceAddress, VkDeviceAddress IndexDeviceAddress)
        {
            auto& RenderableComponent = GetComponent<ECS::COMPONENTS::FDeviceRenderableComponent>(Entity);

            RenderableComponent.VertexBufferAddress = VertexDeviceAddress;
            RenderableComponent.IndexBufferAddress = IndexDeviceAddress;

            bNeedsUpdate = 2;
        }

        std::set<FEntity>::iterator  FRenderableSystem::begin()
        {
            return Entities.begin();
        }

        std::set<FEntity>::iterator  FRenderableSystem::end()
        {
            return Entities.end();
        }
    }
}