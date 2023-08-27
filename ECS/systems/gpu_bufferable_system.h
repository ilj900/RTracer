#pragma once

#include "system.h"
#include "coordinator.h"

#include "buffer.h"

#include "vk_context.h"

#include "maths.h"

namespace ECS
{
    namespace SYSTEMS
    {
        class FGPUBufferableSystem : public FSystem
        {
        public:
            virtual void Init(int NumberOfSimultaneousSubmits) = 0;
            void RequestAllUpdate();
            void RequestUpdate(int FrameIndex);
            int GetTotalSize();
            virtual void Update() = 0;
            virtual void Update(int Index) = 0;

            template<typename T>
            void UpdateTemplate()
            {
                for (int i = 0; i < BufferPartThatNeedsUpdate.size(); ++i)
                {
                    if (BufferPartThatNeedsUpdate[i])
                    {
                        auto& Coordinator = GetCoordinator();
                        auto& Context = GetContext();
                        auto DeviceComponentsData = Coordinator.Data<T>();
                        auto DeviceComponentsSize = Coordinator.Size<T>();

                        Context.ResourceAllocator->LoadDataToBuffer(DeviceBuffer, {DeviceComponentsSize}, {DeviceComponentsSize * i}, {DeviceComponentsData});

                        BufferPartThatNeedsUpdate[i] = false;
                    }
                }
            }

            template<typename T>
            void UpdateTemplate(int Index)
            {
                if (BufferPartThatNeedsUpdate[Index])
                {
                    auto& Coordinator = GetCoordinator();
                    auto& Context = GetContext();
                    auto DeviceComponentsData = Coordinator.Data<T>();
                    auto DeviceComponentsSize = Coordinator.Size<T>();

                    Context.ResourceAllocator->LoadDataToBuffer(DeviceBuffer, {DeviceComponentsSize}, {DeviceComponentsSize * Index}, {DeviceComponentsData});

                    BufferPartThatNeedsUpdate[Index] = false;
                }
            }

        public:
            void Init(int NumberOfSimultaneousSubmits, uint32_t Size, const std::string& Name);

            std::vector<bool> BufferPartThatNeedsUpdate;
            int NumberOfSimultaneousSubmits = 2;
            bool bIsDirty = false;

            FBuffer DeviceBuffer;
            std::vector<FEntity> EntitiesToUpdate;
        };
    }
}

#define TRANSFORM_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FTransformSystem>()