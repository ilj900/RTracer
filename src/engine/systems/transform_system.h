#pragma once

#include "gpu_bufferable_system.h"
#include "maths.h"

#include <unordered_set>

namespace ECS
{
    namespace SYSTEMS
    {
        class FTransformSystem : public FGPUBufferableSystem
        {
        public:
            void Init(uint32_t NumberOfSimultaneousSubmits) override;
            bool Update() override;
            bool Update(int Index) override;

            void MoveForward(FEntity Entity, float Value);
            void MoveRight(FEntity Entity, float Value);
            void SetTransform(FEntity Entity, const FVector3& Position, const FVector3& Direction, const FVector3& Up, const FVector3& Scale = {1, 1, 1});
            void SetLookAt(FEntity Entity, const FVector3& PointOfInterest);
            void MoveUpward(FEntity Entity, float Value);
            void Roll(FEntity Entity, float Value);
            void Pitch(FEntity Entity, float Value);
            void Yaw(FEntity Entity, float Value);
            void Translate(FEntity Entity, float X, float Y, float Z);
            void SyncTransform(FEntity Entity);
            FMatrix4 GetModelMatrix(FEntity Entity);
        };
    }
}

#define TRANSFORM_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FTransformSystem>()