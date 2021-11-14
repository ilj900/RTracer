#pragma once

#include "maths.h"

namespace ECS
{
    namespace COMPONENTS
    {

const uint32_t RENDERABLE_SELECTED_BIT = 1 << 5;

        struct FDeviceRenderableComponent
        {

            alignas(16) FVector3 RenderableColor = {0.3f, 0.1f, 0.2f};

            uint32_t RenderableIndex = 0;
            uint32_t RenderablePropertyMask = 0;
        };
    }
}