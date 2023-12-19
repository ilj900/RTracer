#pragma once

#include "maths.h"
#include "common_structures.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FDeviceRenderableComponent : FRenderable
        {
            FDeviceRenderableComponent()
            {
                RenderableColor = {0.3f, 0.1f, 0.2f};
                MeshIndex = 0;

                RenderableIndex = 0;
                RenderablePropertyMask = 0;

                VertexBufferAddress = 0;
                IndexBufferAddress = 0;
            }

            FDeviceRenderableComponent(const FVector3& RenderableColorIn, uint32_t  MeshIndexIn, int32_t RenderableIndexIn, uint32_t RenderablePropertyMaskIn, uint64_t VertexBufferAddressIn, uint64_t IndexBufferAddressIn)
            {
                RenderableColor = RenderableColorIn;
                MeshIndex = MeshIndexIn;

                RenderableIndex = RenderableIndexIn;
                RenderablePropertyMask = RenderablePropertyMaskIn;

                VertexBufferAddress = VertexBufferAddressIn;
                IndexBufferAddress = IndexBufferAddressIn;
            }
        };
    }
}