#pragma once

#include "buffer.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FDeviceMeshComponent
        {
            FDeviceMeshComponent() = default;

            FBuffer VertexBuffer;
            FBuffer IndexBuffer;
            FBuffer StagingBuffer;
        };
    }
}