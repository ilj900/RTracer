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

    struct FStringStorage
    {
        void AddString(const std::string& String);

        /// Returns a vector of pointer to the stored strings
        std::vector<const char*> GetPointers();

        std::vector<std::string> Strings;
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

    struct FInstanceCreationOptions
    {
        void AddLayer(const std::string& LayerName);
        std::vector<const char*> GetLayers();

        /// Add an instance extension, possible with a structure that will be passed in pNext chain. Extension structure should be passed with pNext = nullptr
        void AddInstanceExtension(const std::string& ExtensionName, void* ExtensionStructure = nullptr, uint32_t ExtensionStructureSize = 0);

        FStringStorage Layers;
        FExtensionVector ExtensionVector;
    };

    struct FLogicalDeviceOptions
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

    VkInstance CreateInstance(const std::string& AppName, const FVersion3& AppVersion, const std::string& EngineName, const FVersion3& EngineVersion, uint32_t ApiVersion, FInstanceCreationOptions& Options);
    VkSampleCountFlagBits GetMaxMsaa(VkPhysicalDevice PhysicalDevice);
    /// Check whether device supports the requested queue type and if it does fills in QueueFamilyIndex
    bool CheckQueueTypeSupport(VkPhysicalDevice PhysicalDevice, VkQueueFlagBits Type, uint32_t& QueueFamilyIndex);
    /// Check whether device supports the present queue for the surface, if yes, fills in QueueFamilyIndex
    bool CheckPresentQueueSupport(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface, uint32_t& QueueFamilyIndex);
    /// Check whether device supports all the extensions from the list
    bool CheckDeviceExtensionSupport(VkPhysicalDevice PhysicalDevice, std::vector<std::string> RequiredDeviceExtensions);
    VkDevice CreateLogicalDevice(VkPhysicalDevice PhysicalDevice, FLogicalDeviceOptions& Options);
    /// Enumerate all physical devices
    std::vector<VkPhysicalDevice> GetAllPhysicalDevices(VkInstance Instance);

    VkCommandPool CreateCommandPool(VkDevice LogicalDevice, uint32_t QueueIndex);
    VkCommandBuffer AllocateCommandBuffer(VkDevice LogicalDevice, VkCommandPool CommandPool);
    VkCommandBuffer BeginWithAllocation(VkDevice LogicalDevice, VkCommandPool CommandPool);
    VkCommandBuffer BeginSingleTimeCommand(VkDevice LogicalDevice, VkCommandPool CommandPool);
    void SubmitCommandBuffer(VkDevice LogicalDevice, VkCommandPool CommandPool, VkQueue Queue, VkCommandBuffer &CommandBuffer);

    VkImage CreateImage(uint32_t Width, uint32_t Height, uint32_t MipsLevels, VkSampleCountFlagBits NumSamples, VkFormat Format, VkImageTiling Tiling, VkImageUsageFlags Usage, VkDevice Device);
    VkDeviceMemory AllocateMemoryForTheImage(VkImage Image, VkDevice Device, VkMemoryPropertyFlags Properties, uint32_t MemoryTypeIndex);
    void BindMemoryToImage(VkDevice Device, VkImage Image, VkDeviceMemory Memory, VkDeviceSize MemoryOffset = 0);
    VkImageView CreateImageView(VkDevice Device, VkImage Image, VkFormat Format, VkImageAspectFlags AspectFlags, uint32_t MipLevelsCount);

    uint32_t FindMemoryType(VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties, uint32_t TypeFilter, VkMemoryPropertyFlags Properties);
    VkMemoryRequirements GetImageMemoryRequirements(VkDevice Device, VkImage Image);

    VkPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties(VkPhysicalDevice PhysicalDevice);

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
}