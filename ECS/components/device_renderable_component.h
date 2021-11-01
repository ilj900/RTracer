#pragma once

#include "maths.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FDeviceRenderableComponent
        {

            alignas(16) FVector3 RenderableColor = {0.3f, 0.1f, 0.2f};

            alignas(16) uint32_t RenderableIndex = 0;
            alignas(16) uint32_t RenderablePropertyMask = 0;
        };
    }
}