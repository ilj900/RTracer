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
        private:
            template<typename T>
            T& GetComponent(FEntity Entity);

        public:
            void Init(int NumberOfSimultaneousSubmits);
            void Update(int IterationIndex);
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

        public:
            bool bNeedsUpdate = false;
            int NumberOfSimultaneousSubmits = 2;

            FBuffer DeviceTransformBuffer;
        };
    }
}

#define TRANSFORM_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FTransformSystem>()