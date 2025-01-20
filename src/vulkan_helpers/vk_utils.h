#pragma once

#include "vulkan/vulkan.h"

#include "maths.h"

#include <chrono>
#include <fstream>
#include <set>
#include <stdexcept>
#include <vector>

struct FVersion3
{
    uint32_t Major = 1u;
    uint32_t Minor = 0u;
    uint32_t Patch = 0u;
};

/// Just a utility structure to simplify access to pNext pointer
struct BaseVulkanStructure
{
    VkStructureType                         sType;
    const void*                             pNext;
};

struct FSynchronizationPoint
{
	std::vector<VkSemaphore> SemaphoresToWait;
	std::vector<VkFence> FencesToWait;
	std::vector<VkSemaphore> SemaphoresToSignal;
	std::vector<VkFence> FencesToSignal;

	friend FSynchronizationPoint operator+(const FSynchronizationPoint& A, const FSynchronizationPoint& B);
	friend FSynchronizationPoint& operator+=(FSynchronizationPoint& A, const FSynchronizationPoint& B);
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
    T* GetInstanceExtensionStructurePtr(VkStructureType StructureType)
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

    template <class T>
    T* GetDeviceExtensionStructurePtr(VkStructureType StructureType)
    {
        for (auto Extension : DeviceOptions.ExtensionVector.Extensions)
        {
            void* Ptr = &DeviceOptions.ExtensionVector.ExtensionsData[Extension.ExtensionStructureOffset];
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

struct FMargin
{
	FMargin() {};
	FMargin(uint32_t L, uint32_t R) : Left(L), Right(R) {};
	uint32_t Left;
	uint32_t Right;
};

std::pair<std::vector<FMargin>, std::vector<float>> GenerateImportanceMap(float* Data, uint32_t Width, uint32_t Height);

std::string ReadFileToString(const std::string& FileName);

class FTimer
{
public:
    FTimer(const std::string& TextToPrint);
    void operator()(const std::string& TextToPrint);
    ~FTimer();

private:
    typedef std::pair<std::chrono::time_point<std::chrono::high_resolution_clock>, std::string> NamedEvent;
    std::vector<NamedEvent> NamedEvents;
};

template <typename T>
void SaveFile(const std::string& FileName, std::vector<T> Data)
{
    std::ofstream File(FileName, std::ios::out | std::ios::binary);

    if (!File.is_open())
    {
        throw std::runtime_error("Failed to open file!");
    }

    File.write(Data.data(), Data.size() * sizeof(T));
    File.close();
}
