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

            BufferPartThatNeedsUpdate.resize(NumberOfSimultaneousSubmits);
        }

        void FRenderableSystem::Update()
        {
            for (int i = 0; i < BufferPartThatNeedsUpdate.size(); ++i)
            {
                if (true == BufferPartThatNeedsUpdate[i])
                {
                    auto& Coordinator = GetCoordinator();
                    auto& Context = GetContext();
                    auto DeviceRenderableComponentsData = Coordinator.Data<ECS::COMPONENTS::FDeviceRenderableComponent>();
                    auto DeviceRenderableComponentsSize = Coordinator.Size<ECS::COMPONENTS::FDeviceRenderableComponent>();

                    Context.ResourceAllocator->LoadDataToBuffer(DeviceRenderableBuffer, DeviceRenderableComponentsSize, DeviceRenderableComponentsSize * i, DeviceRenderableComponentsData);

                    BufferPartThatNeedsUpdate[i] = false;
                }
            }
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

        std::set<FEntity>::iterator  FRenderableSystem::begin()
        {
            return Entities.begin();
        }

        std::set<FEntity>::iterator  FRenderableSystem::end()
        {
            return Entities.end();
        }

        void FRenderableSystem::RequestAllUpdate()
        {
            for(int i = 0; i < NumberOfSimultaneousSubmits; ++i)
            {
                BufferPartThatNeedsUpdate[i] = true;
            }
        }

        void FRenderableSystem::RequestUpdate(int FrameIndex)
        {
            BufferPartThatNeedsUpdate[FrameIndex] = true;
        }

        int FRenderableSystem::GetTotalSize()
        {
            auto& Coordinator = GetCoordinator();
            return Coordinator.Size<ECS::COMPONENTS::FDeviceRenderableComponent>();
        }
    }
}