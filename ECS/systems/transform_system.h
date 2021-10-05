#pragma once

#include "system.h"

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
            void UpdateAllDeviceComponentsData();
            void UpdateDeviceComponentData(FEntity Entity);
            void MoveForward(FEntity Entity, float Value);
            void MoveRight(FEntity Entity, float Value);
            void MoveUpward(FEntity Entity, float Value);
            void Roll(FEntity Entity, float Value);
            void Pitch(FEntity Entity, float Value);
            void Yaw(FEntity Entity, float Value);
            FMatrix4 GetModelMatrix(FEntity Entity);
        };
    }
}