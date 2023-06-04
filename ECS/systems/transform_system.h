#pragma once

#include "system.h"
#include "coordinator.h"

#include "buffer.h"

#include "maths.h"

namespace ECS
{
    namespace SYSTEMS
    {
        class FTransformSystem : public FSystem
        {
        public:
            void Init(int NumberOfSimultaneousSubmits);
            void Update();
            void UpdateAllDeviceComponentsData();
            void UpdateDeviceComponentData(FEntity Entity);
            void MoveForward(FEntity Entity, float Value);
            void MoveRight(FEntity Entity, float Value);
            void SetTransform(FEntity Entity, const FVector3& Position, const FVector3& Direction, const FVector3& Up);
            void SetLookAt(FEntity Entity, const FVector3& PointOfInterest);
            void MoveUpward(FEntity Entity, float Value);
            void Roll(FEntity Entity, float Value);
            void Pitch(FEntity Entity, float Value);
            void Yaw(FEntity Entity, float Value);
            FMatrix4 GetModelMatrix(FEntity Entity);
            void RequestAllUpdate();
            void RequestUpdate(int FrameIndex);

        public:
            std::vector<bool> BufferPartThatNeedsUpdate;
            int NumberOfSimultaneousSubmits = 2;
            bool bIsDirty = false;

            FBuffer DeviceTransformBuffer;
        };
    }
}

#define TRANSFORM_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FTransformSystem>()