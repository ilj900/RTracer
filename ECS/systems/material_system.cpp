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
            FGPUBufferableSystem::Init(NumberOfSimultaneousSubmits, sizeof(ECS::COMPONENTS::FMaterialComponent) * MAX_MATERIALS,
                                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, "Device_Material_Buffer");
        }

        void FMaterialSystem::Update()
        {
            FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FMaterialComponent>();
        }

        void FMaterialSystem::Update(int Index)
        {
            FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FMaterialComponent>(Index);
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