#include "components/material_component.h"
#include "systems/material_system.h"
#include "coordinator.h"

#include "vk_context.h"

#include <cassert>

namespace ECS
{
    namespace SYSTEMS
    {
        void FMaterialSystem::Init(int NumberOfSimultaneousSubmits)
        {
            this->NumberOfSimultaneousSubmits = NumberOfSimultaneousSubmits;
            auto& Coordinator = GetCoordinator();
            auto& Context = GetContext();
            auto MaterialComponentsData = Coordinator.Data<ECS::COMPONENTS::FMaterialComponent>();
            auto MaterialComponentsSize = Coordinator.Size<ECS::COMPONENTS::FMaterialComponent>();

            VkDeviceSize DeviceMaterialBufferSize = MaterialComponentsSize * NumberOfSimultaneousSubmits;

            DeviceMaterialBuffer = GetContext().CreateBuffer(DeviceMaterialBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "Device_Material_Buffer");

            for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
            {
                Context.ResourceAllocator->LoadDataToBuffer(DeviceMaterialBuffer, {MaterialComponentsSize}, {MaterialComponentsSize * i}, {MaterialComponentsData});
            }

            BufferPartThatNeedsUpdate.resize(NumberOfSimultaneousSubmits);
        }

        void FMaterialSystem::Update()
        {
            for (int i = 0; i < BufferPartThatNeedsUpdate.size(); ++i)
            {
                if (true == BufferPartThatNeedsUpdate[i])
                {
                    auto& Coordinator = GetCoordinator();
                    auto& Context = GetContext();
                    auto MaterialComponentsData = Coordinator.Data<ECS::COMPONENTS::FMaterialComponent>();
                    auto MaterialComponentsSize = Coordinator.Size<ECS::COMPONENTS::FMaterialComponent>();

                    Context.ResourceAllocator->LoadDataToBuffer(DeviceMaterialBuffer, {MaterialComponentsSize}, {MaterialComponentsSize * i}, {MaterialComponentsData});

                    BufferPartThatNeedsUpdate[i] = false;
                }
            }
        }

        FMaterialSystem& FMaterialSystem::SetBaseAlbedo(FEntity MaterialEntity, float Red, float Green, float Blue)
        {
            auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
            MaterialComponent.BaseAlbedo = FVector3(Red, Green, Blue);
            return *this;
        }


        void FMaterialSystem::RequestAllUpdate()
        {
            for(int i = 0; i < NumberOfSimultaneousSubmits; ++i)
            {
                BufferPartThatNeedsUpdate[i] = true;
            }
        }

        void FMaterialSystem::RequestUpdate(int FrameIndex)
        {
            BufferPartThatNeedsUpdate[FrameIndex] = true;
        }

        int FMaterialSystem::GetTotalSize()
        {
            auto& Coordinator = GetCoordinator();
            return Coordinator.Size<ECS::COMPONENTS::FMaterialComponent>();
        }
    }
}