#pragma once

#include "systems/gpu_bufferable_system.h"

#include "components/light_component.h"

#include "vk_context.h"

namespace ECS
{
    namespace SYSTEMS
    {
        void FGPUBufferableSystem::Init(int NumberOfSimultaneousSubmits, uint32_t Size, VkBufferUsageFlags BufferFlags, const std::string& Name)
        {
            this->NumberOfSimultaneousSubmits = NumberOfSimultaneousSubmits;

            DeviceBuffer = GetResourceAllocator()->CreateBuffer(Size * NumberOfSimultaneousSubmits, BufferFlags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, Name);
            BufferPartThatNeedsUpdate.resize(NumberOfSimultaneousSubmits);
        }

        void FGPUBufferableSystem::RequestAllUpdate()
        {
            for(int i = 0; i < NumberOfSimultaneousSubmits; ++i)
            {
                BufferPartThatNeedsUpdate[i] = true;
            }
        }

        void FGPUBufferableSystem::RequestUpdate(int FrameIndex)
        {
            BufferPartThatNeedsUpdate[FrameIndex] = true;
        }

        int FGPUBufferableSystem::GetTotalSize()
        {
            return DeviceBuffer.BufferSize;
        }
    }
}
