#pragma once

#include "maths.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FDeviceRenderableComponent
        {

            FVector3 RenderableColor = {0.3f, 0.1f, 0.2f};
            uint32_t Dummy1;

            uint32_t RenderableIndex = 0;
            uint32_t RenderablePropertyMask = 0;
            uint32_t Dummy2;
            uint32_t Dummy3;
        };
    }
}