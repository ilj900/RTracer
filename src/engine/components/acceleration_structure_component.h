#pragma once

#include "vk_acceleration_structure.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FAccelerationStructureComponent
        {
            FAccelerationStructureComponent() = default;
            FAccelerationStructureComponent(const FAccelerationStructure& AccelerationStructureIn) : AccelerationStructure(AccelerationStructureIn) {};

            FAccelerationStructure AccelerationStructure;
        };
    }
}