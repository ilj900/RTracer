#pragma once

#include "buffer.h"

#include "gpu_bufferable_system.h"
#include "coordinator.h"

namespace ECS
{
    namespace SYSTEMS
    {
        class FCameraSystem : public FGPUBufferableSystem
        {
        public:
            void Init(uint32_t NumberOfSimultaneousSubmits) override;
            bool Update() override;
            bool Update(int Index) override;

            void MoveCameraForward(FEntity CameraEntity, float Value);
            void MoveCameraRight(FEntity CameraEntity, float Value);
            void MoveCameraUpward(FEntity CameraEntity, float Value);
            void LookUp(FEntity CameraEntity, float Value);
            void LookRight(FEntity CameraEntity, float Value);
            void Roll(FEntity CameraEntity, float Value);

            const uint32_t MAX_CAMERAS = 8;
        };
    }
}

#define CAMERA_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FCameraSystem>()
