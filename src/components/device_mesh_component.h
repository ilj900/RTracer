#pragma once

#include "buffer.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FDeviceMeshComponent
        {
            FDeviceMeshComponent() = default;

            FMemoryPtr VertexPtr;
            FMemoryPtr IndexPtr;
        };
    }
}