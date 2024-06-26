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
            virtual void Init(uint32_t NumberOfSimultaneousSubmits) = 0;
            void MarkDirty(FEntity Entity);
            int GetTotalSize();
            virtual bool Update() = 0;
            virtual bool Update(int Index) = 0;

            void RegisterEntity(FEntity Entity) override;
            void UnregisterEntity(FEntity Entity) override;

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
                if (BufferPartThatNeedsUpdate[Index])
                {
                    std::vector<VkDeviceSize> Sizes;
                    std::vector<VkDeviceSize> Offsets;
                    std::vector<void*> Data;
					VkDeviceSize DeviceBufferBaseOffset = Index * GetTotalSize() / NumberOfSimultaneousSubmits;

                    for (auto Entity : EntitiesToUpdate[Index])
                    {
                        if (Offsets.size() > 0 && ((COORDINATOR().GetOffset<T>(Entity) + DeviceBufferBaseOffset) == Offsets.back() + Sizes.back()))
                        {
                            Sizes.back() += sizeof(T);
                        }
                        else
                        {
                            Sizes.push_back(sizeof(T));
                            Offsets.push_back(COORDINATOR().GetOffset<T>(Entity) + DeviceBufferBaseOffset);
                            Data.push_back(COORDINATOR().Data<T>(Entity));
                        }
                    }
                    RESOURCE_ALLOCATOR()->LoadDataToBuffer(DeviceBuffer, Sizes, Offsets, Data);

                    BufferPartThatNeedsUpdate[Index] = false;
                }

                EntitiesToUpdate[Index].clear();
            }

        public:
            void Init(uint32_t NumberOfSimultaneousSubmits, uint32_t Size, VkBufferUsageFlags BufferFlags, const std::string& Name);

            std::vector<bool> BufferPartThatNeedsUpdate;
            uint32_t NumberOfSimultaneousSubmits = 2;

            FBuffer DeviceBuffer;
            std::vector<std::set<FEntity>> EntitiesToUpdate;
        };
    }
}

#define TRANSFORM_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FTransformSystem>()