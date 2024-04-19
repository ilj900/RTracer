#pragma once

#include "buffer.h"

struct FAccelerationStructure
{
    FBuffer Buffer;
    VkAccelerationStructureKHR AccelerationStructure = VK_NULL_HANDLE;
    VkAccelerationStructureTypeKHR Type;
};