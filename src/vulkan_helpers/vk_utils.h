#pragma once

#include "vulkan/vulkan.h"

#include "maths.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <numeric>
#include <optional>
#include <set>
#include <stdexcept>
#include <queue>
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

struct FAliasTableEntry
{
	FAliasTableEntry() {};
	FAliasTableEntry(float T, uint32_t A): Threshold(T), Alias(A) {};
	float Threshold;
	uint32_t Alias;
};

/// @brief Generate importance sampling map and a weight map.
/// WidthIn and HeightIn are dimensions of the incoming DataIn.
/// Evaluator is a function that measures a value of a single T. For example in case of rbg color it can be luminosity of that color.
/// The result is a pair of two vectors: Importance map of FAliasTableEntry and a vector(map) of each value weight.
template <typename T>
std::pair<std::vector<FAliasTableEntry>, std::vector<float>> GenerateImportanceMapFast(void* DataIn, uint32_t Width, uint32_t Height, std::function<double(T)> Evaluator)
{
	/// Cast the pointer;
	T* Data = static_cast<T*>(DataIn);
	/// Total number of pixels in the output image
	uint32_t PixelsCount = Width * Height;

	/// We have to use double, cause the precision of float might not be enough in some cases
	std::vector<double> PDF(PixelsCount);

	/// Calculate measured value of each entry (For example if we have a FVector4 as T, we need a function that will return it's 'value' (like Luminosity)
	for (int i = 0; i < PixelsCount; ++i)
	{
		PDF[i] = Evaluator(Data[i]);
	}

	/// Calculate 'total value' by copying sorting and adding all the values
	auto Copy = PDF;
	std::sort(Copy.begin(), Copy.end());
	double TotalValueSum = std::accumulate(Copy.begin(), Copy.end(), 0.);

	std::vector<FAliasTableEntry> IBLSamplingMap;
	std::vector<float> PDFResult;

	if (TotalValueSum <= 0.)
	{
		/// For cases like totally black IBL
		IBLSamplingMap.emplace_back(FAliasTableEntry{1.f, 0});
		PDFResult.emplace_back(1.f);
		return {IBLSamplingMap, PDFResult};
	}

	IBLSamplingMap.resize(PixelsCount);
	PDFResult.resize(PixelsCount);

	for (uint32_t i = 0; i < PixelsCount; ++i)
	{
		PDF[i] /= TotalValueSum;
		PDFResult[i] = PDF[i];
	}

	std::vector<double> NormalizedValues(PixelsCount);
	std::queue<uint32_t> IndicesThatGreater;
	std::queue<uint32_t> IndicesThatSmaller;

	for (uint32_t i = 0; i < PixelsCount; ++i)
	{
		NormalizedValues[i] = {PDF[i] * PixelsCount};
		if (NormalizedValues[i] < 1.)
		{
			IndicesThatSmaller.push(i);
			continue;
		}
		if (NormalizedValues[i] > 1.)
		{
			IndicesThatGreater.push(i);
		}
	}

	std::vector<double> Probabilities(PixelsCount);
	std::vector<uint32_t> Alias(PixelsCount);

	/// We run it until there's only one element left in each group
	/// Because of floating point precision they wouldn't get to perfect 1.
	while (!IndicesThatSmaller.empty() && !IndicesThatGreater.empty())
	{
		uint32_t SmallIndex = IndicesThatSmaller.front();
		uint32_t LargeIndex = IndicesThatGreater.front();
		IndicesThatSmaller.pop();
		IndicesThatGreater.pop();

		Probabilities[SmallIndex] = NormalizedValues[SmallIndex];
		Alias[SmallIndex] = LargeIndex;

		NormalizedValues[LargeIndex] = NormalizedValues[LargeIndex] - (1. - NormalizedValues[SmallIndex]);

		if (NormalizedValues[LargeIndex] < 1.)
		{
			IndicesThatSmaller.push(LargeIndex);
		}
		else
		{
			IndicesThatGreater.push(LargeIndex);
		}
	}

	while (!IndicesThatGreater.empty())
	{
		uint32_t L = IndicesThatGreater.back();
		IndicesThatGreater.pop();
		Probabilities[L] = 1.;
		Alias[L] = L;
	}

	while (!IndicesThatSmaller.empty())
	{
		uint32_t S = IndicesThatSmaller.back();
		IndicesThatSmaller.pop();
		Probabilities[S] = 1.;
		Alias[S] = S;
	}

	std::vector<FAliasTableEntry> Result(PixelsCount);
	for (uint32_t i = 0; i < PixelsCount; ++i)
	{
		Result[i].Threshold = Probabilities[i];
		Result[i].Alias = Alias[i];
	}

	return {Result, PDFResult};
}

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
