#pragma once

#include "maths.h"
#include "common_structures.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FTextureComponent
        {
            FTextureComponent() = default;
            FTextureComponent(uint32_t TextureIndexIn) : TextureIndex(TextureIndexIn) {};
            uint32_t TextureIndex;
        };
    }
}