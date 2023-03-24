#include "vk_shader_compiler.h"
#include "vk_utils.h"
#include "vk_context.h"

#include "shaderc/shaderc.hpp"

#include <cassert>
#include <iostream>
#include <map>

shaderc_shader_kind GetShaderType(const std::string& Path);
std::vector<uint32_t> CompileShaderToSpirVData(const std::string &Path);

FShader::FShader(const std::string &Path)
{
    auto SPIRVData = CompileShaderToSpirVData(Path);

    VkShaderModuleCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    CreateInfo.codeSize = SPIRVData.size() * sizeof(uint32_t);
    CreateInfo.pCode = reinterpret_cast<const uint32_t *>(SPIRVData.data());

    if (vkCreateShaderModule(GetContext().LogicalDevice, &CreateInfo, nullptr, &ShaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module!");
    }
}

VkShaderModule FShader::operator()()
{
    return ShaderModule;
}

FShader::~FShader()
{
    vkDestroyShaderModule(GetContext().LogicalDevice, ShaderModule, nullptr);
}

shaderc_shader_kind GetShaderType(const std::string& Path)
{
    size_t LastDotPos = Path.rfind('.', Path.size());
    if (LastDotPos == std::string::npos)
    {
        assert(false && "File passed for shader compilation has no extension.");
    }

    std::string Extension = Path.substr(LastDotPos + 1, Path.size() - LastDotPos);

    std::map<std::string, shaderc_shader_kind> ExtensionToShaderTypeMap
            {
                    {"rchit", shaderc_shader_kind::shaderc_closesthit_shader},
                    {"rgen", shaderc_shader_kind::shaderc_raygen_shader},
                    {"rmiss", shaderc_shader_kind::shaderc_miss_shader},
                    {"vert", shaderc_shader_kind::shaderc_vertex_shader},
                    {"frag", shaderc_shader_kind::shaderc_fragment_shader},
                    {"comp", shaderc_shader_kind::shaderc_compute_shader},
            };

    if (ExtensionToShaderTypeMap.find(Extension) == ExtensionToShaderTypeMap.end())
    {
        assert(false && "Failed to deduct shader's extension.");
    }

    return ExtensionToShaderTypeMap[Extension];
}

std::vector<uint32_t> CompileShaderToSpirVData(const std::string &Path)
{
    auto ShaderType = GetShaderType(Path);
    auto ShaderCode = ReadFileToString(Path);

    std::map<shaderc_shader_kind, std::string> ShaderTypeToExtensionMap
    {
        {shaderc_shader_kind::shaderc_closesthit_shader, "rchit"},
        {shaderc_shader_kind::shaderc_raygen_shader, "rgen"},
        {shaderc_shader_kind::shaderc_miss_shader, "rmiss"},
        {shaderc_shader_kind::shaderc_vertex_shader, "vert"},
        {shaderc_shader_kind::shaderc_fragment_shader, "frag"},
        {shaderc_shader_kind::shaderc_compute_shader, "comp"}
    };

    FTimer Timer("Shader : " + Path + " compilation time: ");
    static shaderc::Compiler Compiler;
    static shaderc::CompileOptions CompileOptions;
    CompileOptions.SetTargetEnvironment(shaderc_target_env_vulkan, 460);
    CompileOptions.SetTargetSpirv(shaderc_spirv_version_1_6);
    shaderc::CompilationResult CompilationResult = Compiler.CompileGlslToSpv(ShaderCode.c_str(), ShaderType, ShaderTypeToExtensionMap[ShaderType].c_str(), CompileOptions);

    if (CompilationResult.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::string ErrorMassage = CompilationResult.GetErrorMessage();
        std::cout << ErrorMassage << '\n';
        assert(false && "Failed to compile shader");
    }

    std::vector<uint32_t> SPIRVByteCode;
    SPIRVByteCode.assign(CompilationResult.begin(), CompilationResult.end());

    return SPIRVByteCode;
}