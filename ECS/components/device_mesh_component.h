#pragma once

#include "buffer.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FDeviceMeshComponent
        {
            FDeviceMeshComponent() = default;

            FMemoryRegion VertexRegion;
            FMemoryRegion IndexRegion;
        };
    }
}