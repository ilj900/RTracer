#pragma once

#include "buffer.h"

#include "system.h"
#include "coordinator.h"

namespace ECS
{
    namespace SYSTEMS
    {
        class FCameraSystem : public FSystem
        {
        private:
            template<typename T>
            T& GetComponent(FEntity CameraEntity);

        public:
            void Init(int NumberOfSimultaneousSubmits);
            void Update();
            void UpdateAllDeviceComponentsData();
            void UpdateDeviceComponentData(FEntity CameraEntity);
            void MoveCameraForward(FEntity CameraEntity, float Value);
            void MoveCameraRight(FEntity CameraEntity, float Value);
            void MoveCameraUpward(FEntity CameraEntity, float Value);
            void LookUp(FEntity CameraEntity, float Value);
            void LookRight(FEntity CameraEntity, float Value);
            void Roll(FEntity CameraEntity, float Value);
            void SetAspectRatio(FEntity CameraEntity, float AspectRatio);
            FMatrix4 GetProjectionMatrix(FEntity CameraEntity);
            FMatrix4 GetViewMatrix(FEntity CameraEntity);
            void Orthogonize(FEntity CameraEntity);

            void RequestAllUpdate();
            void RequestUpdate(int FrameIndex);

        public:
            std::vector<bool> BufferPartThatNeedsUpdate;
            int NumberOfSimultaneousSubmits = 2;

            FBuffer DeviceCameraBuffer;
        };
    }
}

#define CAMERA_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FCameraSystem>()
