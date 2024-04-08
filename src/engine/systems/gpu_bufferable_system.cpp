#pragma once

#include "gpu_bufferable_system.h"

namespace ECS
{
    namespace SYSTEMS
    {
        void FGPUBufferableSystem::RegisterEntity(FEntity Entity)
        {
            FSystem::RegisterEntity(Entity);
            MarkDirty(Entity);
        }

        void FGPUBufferableSystem::UnregisterEntity(FEntity Entity)
        {
            FSystem::UnregisterEntity(Entity);
            ///Todo: What to do?
        }

        void FGPUBufferableSystem::Init(uint32_t NumberOfSimultaneousSubmits, uint32_t Size, VkBufferUsageFlags BufferFlags, const std::string& Name)
        {
            this->NumberOfSimultaneousSubmits = NumberOfSimultaneousSubmits;

            DeviceBuffer = GetResourceAllocator()->CreateBuffer(Size * NumberOfSimultaneousSubmits, BufferFlags | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, Name);
            GetResourceAllocator()->RegisterBuffer(DeviceBuffer, Name);

            BufferPartThatNeedsUpdate.resize(NumberOfSimultaneousSubmits);
            EntitiesToUpdate.resize(NumberOfSimultaneousSubmits);
        }

        void FGPUBufferableSystem::MarkDirty(FEntity Entity)
        {
            for (int i = 0; i < NumberOfSimultaneousSubmits; ++i)
            {
                EntitiesToUpdate[i].insert(Entity);
                BufferPartThatNeedsUpdate[i] = true;
            }
        }

        int FGPUBufferableSystem::GetTotalSize()
        {
            return DeviceBuffer.BufferSize;
        }
    }
}
