#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <stdexcept>
#include <set>

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
    /// Just a utility structure to simplify access to pNext pointer
    struct BaseVulkanStructure
    {
        VkStructureType                         sType;
        const void*                             pNext;
    };

    /// A structure that represents and extension data parameters
    struct FExtension
    {
        std::string ExtensionName;
        uint32_t ExtensionStructureSize;
        size_t ExtensionStructureOffset;
    };

    /// A structure used to store extension's parameters and raw extension structure data
    struct FExtensionVector
    {
        void AddExtension(const std::string& ExtensionName, void* ExtensionStructure = nullptr, uint32_t ExtensionStructureSize = 0);
        void BuildPNextChain(BaseVulkanStructure* CreateInfo);
        std::vector<const char*> GetExtensionSNamesList();

        std::vector<FExtension> Extensions;
        std::vector<char> ExtensionsData;
    };

    struct FInstanceCreationOptions
    {
        void AddLayer(const std::string& LayerName);

        /// Add an instance extension, possible with a structure that will be passed in pNext chain. Extension structure should be passed with pNext = nullptr
        /// \param ExtensionName
        /// \param ExtensionStructure
        /// \param ExtensionStructureSize
        void AddInstanceExtension(const std::string& ExtensionName, void* ExtensionStructure = nullptr, uint32_t ExtensionStructureSize = 0);

        std::vector<std::string> Layers;
        FExtensionVector ExtensionVector;
    };

    struct FLogicalDeviceOptions
    {
        void AddLayer(const std::string& LayerName);
        void RequestQueueSupport(uint32_t QueueFamilyIndex);

        /// Add an instance extension, possible with a structure that will be passed in pNext chain. Extension structure should be passed with pNext = nullptr
        /// \param ExtensionName
        /// \param ExtensionStructure
        /// \param ExtensionStructureSize
        void AddDeviceExtension(const std::string& ExtensionName, void* ExtensionStructure = nullptr, uint32_t ExtensionStructureSize = 0);

        std::set<uint32_t> QueueFamilyIndices{};

        std::vector<std::string> Layers;
        FExtensionVector ExtensionVector;
    };

    VkInstance CreateInstance(const std::string& AppName, const FVersion3& AppVersion, const std::string& EngineName, const FVersion3& EngineVersion, uint32_t ApiVersion, FInstanceCreationOptions& Options);
    VkSampleCountFlagBits GetMaxMsaa(VkPhysicalDevice PhysicalDevice);
    /// Check whether device support the requested queue type and if it does fills in QueueFamilyIndex
    bool CheckQueueTypeSupport(VkPhysicalDevice PhysicalDevice, VkQueueFlagBits Type, uint32_t& QueueFamilyIndex);
    bool CheckPresentQueueSupport(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface, uint32_t& QueueFamilyIndex);
    bool CheckDeviceExtensionSupport(VkPhysicalDevice PhysicalDevice, std::vector<std::string> RequiredDeviceExtensions);
    VkDevice CreateLogicalDevice(VkPhysicalDevice PhysicalDevice, FLogicalDeviceOptions& Options);

    std::vector<VkPhysicalDevice> GetAllPhysicalDevices(VkInstance Instance);

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
        T Function = (T) vkGetDeviceProcAddr(Device, FunctionName.c_str());
        CheckNotNullptr(Function, FunctionName);
        return Function;
    }
}