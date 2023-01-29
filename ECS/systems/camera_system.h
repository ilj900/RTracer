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
            void Update(int IterationIndex);
            void UpdateAllDeviceComponentsData();
            void UpdateDeviceComponentData(FEntity CameraEntity);
            void MoveCameraForward(FEntity CameraEntity, float Value);
            void MoveCameraRight(FEntity CameraEntity, float Value);
            void MoveCameraUpward(FEntity CameraEntity, float Value);
            void LookUp(FEntity CameraEntity, float Value);
            void LookRight(FEntity CameraEntity, float Value);
            void Roll(FEntity CameraEntity, float Value);
            FMatrix4 GetProjectionMatrix(FEntity CameraEntity);
            FMatrix4 GetViewMatrix(FEntity CameraEntity);
            void Orthogonize(FEntity CameraEntity);

        public:
            bool bNeedsUpdate = false;
            int NumberOfSimultaneousSubmits = 2;

            FBuffer DeviceCameraBuffer;
        };
    }
}

#define CAMERA_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FCameraSystem>()
