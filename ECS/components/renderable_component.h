#pragma once

#include "vulkan/vulkan.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FRenderableComponent
        {
            VkDescriptorSet RenderableDescriptorSet;
        };
    }
}