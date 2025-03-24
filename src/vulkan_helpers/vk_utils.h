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

/// @brief Generate importance sampling map and a weight map.
/// WidthIn and HeightIn are dimensions of the incoming DataIn.
/// WidthOut and HeightOut are the size of the outcoming image.
/// Evaluator is a function that measures a value of a single T. For example in case of rbg color it can be luminosity of that color.
/// The result is a pair of two vectors: Importance map of FMargin and a vector(map) of each value weight.
template <typename T>
std::pair<std::vector<FMargin>, std::vector<float>> GenerateImportanceMap(void* DataIn, uint32_t WidthIn, uint32_t HeightIn, uint32_t WidthOut, uint32_t HeightOut, std::function<double(T)> Evaluator)
{
	/// Cast the pointer;
	T* Data = static_cast<T*>(DataIn);
	/// Total number of incoming values
	uint32_t TotalValuesCountIn = WidthIn * HeightIn;
	/// Total number of pixels in the output image
	uint32_t PixelsCountOut = WidthOut * HeightOut;

	/// We have to use double, cause the precision of float might not be enough in some cases
	std::vector<double> EachValueEvaluated(TotalValuesCountIn);

	/// Calculate measured value of each entry (For example if we have a FVector4 as T, we need a function that will return it's 'value' (like Luminosity)
	for (int i = 0; i < TotalValuesCountIn; ++i)
	{
		EachValueEvaluated[i] = Evaluator(Data[i]);
	}

	/// Calculate 'total value' by copying sorting and adding all the values
	auto Copy = EachValueEvaluated;
	std::sort(Copy.begin(), Copy.end());
	double TotalValueSum = std::accumulate(Copy.begin(), Copy.end(), 0.);

	if (TotalValueSum > 0.)
	{
		/// Compute probability for each value
		std::vector<double> PDF(TotalValuesCountIn);

		for (int i = 0; i < TotalValuesCountIn; ++i)
		{
			PDF[i] = EachValueEvaluated[i] / TotalValueSum;
		}

		/// Calculate CDF
		std::vector<double> CDF(TotalValuesCountIn + 1);
		CDF[0] = 0;

		for (int i = 1; i < CDF.size(); ++i)
		{
			CDF[i] = CDF[i - 1] + PDF[i - 1];
		}
		CDF.back() = 1.;

		/// Map of margins that later will be use as a texture
		std::vector<FMargin> SamplingMap(PixelsCountOut);
		double				 Stride = 1. / double(PixelsCountOut);
		uint32_t			 Slow = 0;
		uint32_t			 Fast = 0;

		for (int i = 0; i < SamplingMap.size(); ++i)
		{
			/// We get the left and right values of a uniform distribution margins
			double Left = i * Stride;
			double Right = (i + 1) * Stride;

			/// Iterate Slow until it enters the margin
			while (Slow < PixelsCountOut && CDF[Slow] <= Left)
			{
				Slow++;
			}

			/// Iterate Fast until it leaves the margin
			while (Fast < PixelsCountOut && CDF[Fast] <= Right)
			{
				Fast++;
			}

			/// Record the margin. Slow - 1 is the index of texel that "enters" the margin and Fast - 1 is the index of pixel that leaves the margin
			SamplingMap[i] = { Slow - 1, Fast - 1 };
		}

		std::vector<float> InversePDFWeights(TotalValuesCountIn);
		const double	   UniformPDF = 1.f / static_cast<double>(TotalValuesCountIn);

		for (int i = 0; i < TotalValuesCountIn; ++i)
		{
			InversePDFWeights[i] = static_cast<float>(UniformPDF / PDF[i]);
		}

		return {SamplingMap, InversePDFWeights};
	}
	else
	{
		/// For cases like totally black IBL
		std::vector<FMargin> IBLSamplingMap(1, {0, 0});
		std::vector<float> InversePDFWeights(1, 1.);
		return {IBLSamplingMap, InversePDFWeights};
	}
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
