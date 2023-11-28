#version 450

#include "common_defines.h"

layout (location = 0) in vec2 OutUV;

layout (location = 0) out vec4 FragColor;

layout (set = PASSTHROUGH_PER_FRAME_LAYOUT_INDEX, binding = PASSTHROUGH_TEXTURE_SAMPLER_LAYOUT_INDEX) uniform sampler2D InTexture;

void main()
{
    FragColor = texture(InTexture, OutUV).rgba;
}