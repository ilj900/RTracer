#pragma once

#include "gpu_bufferable_system.h"
#include "coordinator.h"

#include "buffer.h"

#include "maths.h"

namespace ECS
{
    namespace SYSTEMS
    {
        class FTransformSystem : public FGPUBufferableSystem
        {
        public:
            void Init(int NumberOfSimultaneousSubmits) override;
            void Update() override;
            void Update(int Index) override;

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
            void Translate(FEntity Entity, float X, float Y, float Z);
            FMatrix4 GetModelMatrix(FEntity Entity);

            const uint32_t MAX_TRANSFORMS = 8192;
        };
    }
}

#define TRANSFORM_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FTransformSystem>()