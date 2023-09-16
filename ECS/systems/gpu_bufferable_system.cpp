#pragma once

#include "systems/gpu_bufferable_system.h"

#include "components/light_component.h"

#include "vk_context.h"

namespace ECS
{
    namespace SYSTEMS
    {
        void FGPUBufferableSystem::RegisterEntity(FEntity Entity)
        {
            FSystem::RegisterEntity(Entity);
            for (auto& Entry : EntitiesToUpdate)
            {
                Entry.insert(Entity);
            }
        }

        void FGPUBufferableSystem::UnregisterEntity(FEntity Entity)
        {
            FSystem::UnregisterEntity(Entity);
            ///Todo: What to do?
        }

        void FGPUBufferableSystem::Init(int NumberOfSimultaneousSubmits, uint32_t Size, VkBufferUsageFlags BufferFlags, const std::string& Name)
        {
            this->NumberOfSimultaneousSubmits = NumberOfSimultaneousSubmits;

            DeviceBuffer = GetResourceAllocator()->CreateBuffer(Size * NumberOfSimultaneousSubmits, BufferFlags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, Name);
            BufferPartThatNeedsUpdate.resize(NumberOfSimultaneousSubmits);
            EntitiesToUpdate.resize(NumberOfSimultaneousSubmits);
        }

        void FGPUBufferableSystem::MarkDirty(FEntity Entity)
        {
            for (auto& Entry : EntitiesToUpdate)
            {
                Entry.insert(Entity);
            }
        }

        int FGPUBufferableSystem::GetTotalSize()
        {
            return DeviceBuffer.BufferSize;
        }
    }
}
