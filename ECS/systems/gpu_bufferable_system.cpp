#pragma once

#include "systems/gpu_bufferable_system.h"

#include "components/light_component.h"

#include "vk_context.h"

namespace ECS
{
    namespace SYSTEMS
    {
        void FGPUBufferableSystem::Init(int NumberOfSimultaneousSubmits, uint32_t Size, const std::string& Name)
        {
            this->NumberOfSimultaneousSubmits = NumberOfSimultaneousSubmits;

            DeviceBuffer = GetContext().CreateBuffer(Size * NumberOfSimultaneousSubmits, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "Device_Buffer");
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
