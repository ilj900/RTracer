#pragma once

#include "vulkan/vulkan.h"

#include <set>
#include <stdexcept>
#include <vector>

auto CheckNotNullptr = [](void* FunctionPointer, const std::string& FunctionName)
{
    if (FunctionPointer == nullptr)
    {
        throw std::runtime_error("Failed to load function: " + FunctionName + "\n");
    }
};

/// Just a utility structure to simplify access to pNext pointer
struct BaseVulkanStructure
{
    VkStructureType                         sType;
    const void*                             pNext;
};

/// A helper class to store strings
struct FStringStorage
{
    void AddString(const std::string& String);

    /// Returns a vector of pointer to the stored strings
   std::vector<const char*> GetPointers();

    std::vector<std::string> Strings;
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
    /// Build a chain of pNext data pointers and set's a CreateInfo's pNext to point at it
    void BuildPNextChain(BaseVulkanStructure* CreateInfo);
    std::vector<const char*> GetExtensionsNamesList();

    std::vector<FExtension> Extensions;
    std::vector<char> ExtensionsData;
};

struct FInstanceOptions
{
    void AddLayer(const std::string& LayerName);
    std::vector<const char*> GetLayers();

    /// Add an instance extension, possible with a structure that will be passed in pNext chain. Extension structure should be passed with pNext = nullptr
    void AddInstanceExtension(const std::string& ExtensionName, void* ExtensionStructure = nullptr, uint32_t ExtensionStructureSize = 0);

    FStringStorage Layers;
    FExtensionVector ExtensionVector;
};

struct FDeviceOptions
{
    void AddLayer(const std::string& LayerName);
    std::vector<const char*> GetLayers();

    void RequestQueueSupport(uint32_t QueueFamilyIndex);

    /// Add an instance extension, possible with a structure that will be passed in pNext chain. Extension structure should be passed with pNext = nullptr
    void AddDeviceExtension(const std::string& ExtensionName, void* ExtensionStructure = nullptr, uint32_t ExtensionStructureSize = 0);

    std::set<uint32_t> QueueFamilyIndices{};

    FStringStorage Layers;
    FExtensionVector ExtensionVector;
};

struct FVulkanContextOptions
{
    void AddInstanceLayer(const std::string& InstanceLayerName);
    void AddInstanceExtension(const std::string& ExtensionName, void* ExtensionStructure = nullptr, uint32_t ExtensionStructureSize = 0);
    std::vector<const char*> GetInstanceLayers();
    std::vector<const char*> GetInstanceExtensionsList();
    void BuildInstancePNextChain(BaseVulkanStructure* CreateInfo);

    void AddDeviceLayer(const std::string& DeviceLayerName);
    void AddDeviceExtension(const std::string& ExtensionName, void* ExtensionStructure = nullptr, uint32_t ExtensionStructureSize = 0);
    std::vector<const char*> GetDeviceLayers();
    std::vector<const char*> GetDeviceExtensionsList();
    void BuildDevicePNextChain(BaseVulkanStructure* CreateInfo);

    template <class T>
    T* GetExtensionStructurePtr(VkStructureType StructureType)
    {
        for (auto Extension : InstanceOptions.ExtensionVector.Extensions)
        {
            void* Ptr = &InstanceOptions.ExtensionVector.ExtensionsData[Extension.ExtensionStructureOffset];
            auto CastedStruct = reinterpret_cast<BaseVulkanStructure*>(Ptr);
            if (CastedStruct->sType == StructureType)
            {
                return reinterpret_cast<T*>(Ptr);;
            }
        }
        return nullptr;
    }

    FInstanceOptions InstanceOptions;
    FDeviceOptions DeviceOptions;
};

/// Load an instance function
template <class T>
T LoadInstanceFunction(const std::string& FunctionName, VkInstance Instance)
{
    T Function = (T) vkGetInstanceProcAddr(Instance, FunctionName.c_str());
    CheckNotNullptr(Function, FunctionName);
    return Function;
}

/// Load a device function
template <class T>
T LoadDeviceFunction(const std::string& FunctionName, VkDevice Device)
{
    T Function = (T) vkGetDeviceProcAddr(Device, FunctionName.c_str());
    CheckNotNullptr(Function, FunctionName);
    return Function;
}