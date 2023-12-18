#pragma once

#include "buffer.h"

struct FAccelerationStructure
{
    FBuffer Buffer;
    VkAccelerationStructureKHR AccelerationStructure;
    VkAccelerationStructureTypeKHR Type;
};