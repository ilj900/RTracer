#pragma once

#include "buffer.h"

#include "system.h"
#include "coordinator.h"

#include "maths.h"

namespace ECS
{
    namespace SYSTEMS
    {
        class FLightSystem : public FSystem
        {
        public:
            void Init(int NumberOfSimultaneousSubmits);
            void Update();
            FLightSystem& SetLightPosition(FEntity LightEntity, float X, float Y, float Z);

            void RequestAllUpdate();
            void RequestUpdate(int FrameIndex);

            int GetTotalSize();


        public:
            std::vector<bool> BufferPartThatNeedsUpdate;
            int NumberOfSimultaneousSubmits = 2;

            FBuffer DeviceLightBuffer;
        };
    }
}

#define LIGHT_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FLightSystem>()
