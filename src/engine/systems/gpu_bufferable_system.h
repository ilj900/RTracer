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
            void MarkDirty(FEntity Entity);
            int GetTotalSize();
            virtual void Update() = 0;
            virtual void Update(int Index) = 0;

            virtual void RegisterEntity(FEntity Entity) override;
            virtual void UnregisterEntity(FEntity Entity) override;

            template<typename T>
            void UpdateTemplate()
            {
                for (int i = 0; i < BufferPartThatNeedsUpdate.size(); ++i)
                {
                    UpdateTemplate<T>(i);
                }
            }

            template<typename T>
            void UpdateTemplate(int Index)
            {
                VkDeviceSize DeviceBufferBaseOffset = Index * GetTotalSize() / NumberOfSimultaneousSubmits;

                if (BufferPartThatNeedsUpdate[Index])
                {
                    std::vector<VkDeviceSize> Sizes;
                    std::vector<VkDeviceSize> Offsets;
                    std::vector<void*> Data;

                    auto& Coordinator = GetCoordinator();

                    for (auto Entity : EntitiesToUpdate[Index])
                    {
                        if (Offsets.size() > 0 && ((Coordinator.GetOffset<T>(Entity) + DeviceBufferBaseOffset) == Offsets.back() + Sizes.back()))
                        {
                            Sizes.back() += sizeof(T);
                        }
                        else
                        {
                            Sizes.push_back(sizeof(T));
                            Offsets.push_back(Coordinator.GetOffset<T>(Entity) + DeviceBufferBaseOffset);
                            Data.push_back(Coordinator.Data<T>(Entity));
                        }
                    }

                    auto& Context = GetContext();
                    Context.ResourceAllocator->LoadDataToBuffer(DeviceBuffer, Sizes, Offsets, Data);

                    BufferPartThatNeedsUpdate[Index] = false;
                }

                EntitiesToUpdate[Index].clear();
            }

        public:
            void Init(int NumberOfSimultaneousSubmits, uint32_t Size, VkBufferUsageFlags BufferFlags, const std::string& Name);

            std::vector<bool> BufferPartThatNeedsUpdate;
            int NumberOfSimultaneousSubmits = 2;

            FBuffer DeviceBuffer;
            std::vector<std::set<FEntity>> EntitiesToUpdate;
        };
    }
}

#define TRANSFORM_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FTransformSystem>()