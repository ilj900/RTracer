#pragma once

#include "vulkan/vulkan.h"

#include <string>
#include <vector>

class FShader
{
public:
    FShader(const std::string &Path);
    ~FShader();
    VkShaderModule operator()();

    VkShaderModule ShaderModule = VK_NULL_HANDLE;
};