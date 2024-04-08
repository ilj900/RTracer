#pragma once

namespace ECS
{
    namespace COMPONENTS
    {
        struct FFramebufferComponent
        {
            FFramebufferComponent() = default;
            FFramebufferComponent(uint32_t FramebufferImageIndexIn) : FramebufferImageIndex(FramebufferImageIndexIn) {};
            uint32_t FramebufferImageIndex;
        };
    }
}