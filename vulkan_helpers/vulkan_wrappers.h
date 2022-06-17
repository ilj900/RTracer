#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <stdexcept>

struct FVersion3
{
    uint32_t Major = 0u;
    uint32_t Minor = 0u;
    uint32_t Patch = 0u;
};

auto CheckNotNullptr = [](void* FunctionPointer, const std::string& FunctionName)
{
    if (FunctionPointer == nullptr)
    {
        throw std::runtime_error("Failed to load function: " + FunctionName + "\n");
    }
};

namespace V
{
    struct FInstanceCreationOptions
    {
        void AddLayer(const std::string& LayerName);

        /// Add an instance extension, possible with a structure that will be passed in pNext chain. Extension structure should be passed with pNext = nullptr
        /// \param ExtensionName
        /// \param ExtensionStructure
        /// \param ExtensionStructureSize
        void AddInstanceExtension(const std::string& ExtensionName, void* ExtensionStructure = nullptr, uint32_t ExtensionStructureSize = 0);

        std::vector<std::string> Layers;

        struct FExtension
        {
            std::string ExtensionName;
            uint32_t ExtensionStructureSize;
            size_t ExtensionStructureOffset;
        };

        std::vector<FExtension> Extensions;
        std::vector<char> ExtensionsData;

    };

    VkInstance CreateInstance(const std::string& AppName, const FVersion3& AppVersion, const std::string& EngineName, const FVersion3& EngineVersion, uint32_t ApiVersion, FInstanceCreationOptions& Options);
    VkSampleCountFlagBits GetMaxMsaa(VkPhysicalDevice PhysicalDevice);

    template <class T>
    T LoadInstanceFunction(const std::string& FunctionName, VkInstance Instance)
    {
        T Function = (T) vkGetInstanceProcAddr(Instance, FunctionName.c_str());
        CheckNotNullptr(Function, FunctionName);
        return Function;
    }

    template <class T>
    T LoadDeviceFunction(const std::string& FunctionName, VkDevice Device)
    {
        T Function = (T) vkGetInstanceProcAddr(Device, FunctionName.c_str());
        CheckNotNullptr(Function, FunctionName);
        return Function;
    }
}