#pragma once

namespace ECS
{
    namespace COMPONENTS
    {
        struct FFramebufferComponent
        {
            FFramebufferComponent() = default;
            FFramebufferComponent(const std::string& FramebufferNameIn) : FramebufferName(FramebufferNameIn) {};
            std::string FramebufferName;
        };
    }
}