#pragma once

#include "maths.h"

namespace ECS
{
    namespace COMPONENTS
    {

        const uint32_t RENDERABLE_SELECTED_BIT = 1 << 5;
        const uint32_t RENDERABLE_HAS_TEXTURE = 1 << 6;
        const uint32_t RENDERABLE_IS_INDEXED = 1 << 7;

        struct FDeviceRenderableComponent
        {

            FVector3    RenderableColor = {0.3f, 0.1f, 0.2f};
            float       dummy_1;

            int32_t     RenderableIndex = 0;
            uint32_t    RenderablePropertyMask = 0;
            uint32_t    dummy_2;
            uint32_t    dummy_3;

            uint64_t    VertexBufferAddress;
            uint64_t    IndexBufferAddress;
        };
    }
}