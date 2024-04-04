#pragma once

#include "vulkan/vulkan.h"

#include <string>
#include <vector>

struct FCompileDefinitions
{
    std::vector<std::pair<std::string, std::string>> Defines;

    void Push(const std::string& Define, const std::string& Value);
};

class FShader
{
public:
    FShader(const std::string &Path, const FCompileDefinitions* CompileDefinitions = nullptr);
    ~FShader();
    VkShaderModule operator()();

    VkShaderModule ShaderModule = VK_NULL_HANDLE;
};