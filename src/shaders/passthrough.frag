#version 450

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "common_defines.h"
#include "common_structures.h"

layout (location = 0) in vec2 OutUV;

layout (location = 0) out vec4 FragColor;

layout (set = PASSTHROUGH_PER_FRAME_LAYOUT_INDEX, binding = PASSTHROUGH_TEXTURE_SAMPLER_LAYOUT_INDEX) uniform sampler2D InTexture;

layout (push_constant) uniform PushConstantsBlock
{
    FPassthroughPushConstants PushConstants;
};

void main()
{
    FragColor = texture(InTexture, OutUV).rgba;

    ivec2 PixelCoords = ivec2(gl_FragCoord.xy);
    ivec2 Center = ivec2(PushConstants.Width / 2, PushConstants.Height / 2);

    if ((PixelCoords.x == Center.x && (PixelCoords.y > Center.y - 20 && PixelCoords.y < Center.y + 20)) ||
         PixelCoords.y == Center.y && (PixelCoords.x > Center.x - 20 && PixelCoords.x < Center.x + 20))
    {
        FragColor.xyz = (dot(FragColor.xyz, vec3(0.299, 0.587, 0.114)) > 0.5) ? vec3(0) : vec3(1);
    }
}