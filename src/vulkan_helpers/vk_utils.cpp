#include "vk_utils.h"

#include <iostream>
#ifndef NDEBUG
#include <unordered_set>
template <typename T>
bool Collide(const std::vector<T>& A, const std::vector<T>& B)
{
	std::unordered_set<T> SetA;
	SetA.reserve(A.size());

	for (auto& Entry : A)
	{
		SetA.insert(Entry);
	}

	for (auto& Entry : B)
	{
		if (SetA.find(Entry) != SetA.end())
		{
			return true;
		}
	}

	return false;
}

#endif

FSynchronizationPoint operator+(const FSynchronizationPoint& A, const FSynchronizationPoint& B)
{
#ifndef NDEBUG
	if (Collide(A.SemaphoresToWait, B.SemaphoresToWait) || Collide(A.FencesToWait, B.FencesToWait) || Collide(A.SemaphoresToSignal, B.SemaphoresToSignal) || Collide(A.FencesToSignal, B.FencesToSignal))
	{
		throw std::runtime_error("You are merging two FSynchronizationPoint, and they have some common elements");
	}
#endif

	FSynchronizationPoint Result = A;
	Result.SemaphoresToWait.insert(A.SemaphoresToWait.end(), B.SemaphoresToWait.begin(), B.SemaphoresToWait.end());
	Result.FencesToWait.insert(A.FencesToWait.end(), B.FencesToWait.begin(), B.FencesToWait.end());
	Result.SemaphoresToSignal.insert(A.SemaphoresToSignal.end(), B.SemaphoresToSignal.begin(), B.SemaphoresToSignal.end());
	Result.FencesToSignal.insert(A.FencesToSignal.end(), B.FencesToSignal.begin(), B.FencesToSignal.end());

	return Result;
}

FSynchronizationPoint& operator+=(FSynchronizationPoint& A, const FSynchronizationPoint& B)
{
#ifndef NDEBUG
	if (Collide(A.SemaphoresToWait, B.SemaphoresToWait) || Collide(A.FencesToWait, B.FencesToWait) || Collide(A.SemaphoresToSignal, B.SemaphoresToSignal) || Collide(A.FencesToSignal, B.FencesToSignal))
	{
		throw std::runtime_error("You are merging two FSynchronizationPoint, and they have some common elements");
	}
#endif
	
	A.SemaphoresToWait.insert(A.SemaphoresToWait.end(), B.SemaphoresToWait.begin(), B.SemaphoresToWait.end());
	A.FencesToWait.insert(A.FencesToWait.end(), B.FencesToWait.begin(), B.FencesToWait.end());
	A.SemaphoresToSignal.insert(A.SemaphoresToSignal.end(), B.SemaphoresToSignal.begin(), B.SemaphoresToSignal.end());
	A.FencesToSignal.insert(A.FencesToSignal.end(), B.FencesToSignal.begin(), B.FencesToSignal.end());

	return A;
}

void FStringStorage::AddString(const std::string& String)
{
    Strings.push_back(String);
}

/// Returns a vector of pointer to the stored strings
std::vector<const char*> FStringStorage::GetPointers()
{
    std::vector<const char*> StringVector;

    for (uint32_t i = 0; i < Strings.size(); ++i)
    {
        StringVector.push_back(Strings[i].c_str());
    }

    return StringVector;
}

void FExtensionVector::AddExtension(const std::string& ExtensionName, void* ExtensionStructure, uint32_t ExtensionStructureSize)
{
    /// If there's and extension structure
    if (ExtensionStructure)
    {
        /// Store the extension's data
        Extensions.push_back({ExtensionName, ExtensionStructureSize, ExtensionsData.size()});

        /// Store the extension's structure data
        auto StartingPosition = ExtensionsData.size();
        ExtensionsData.resize(ExtensionsData.size() + ExtensionStructureSize);
        memcpy(&ExtensionsData[StartingPosition], ExtensionStructure, ExtensionStructureSize);
    }
    else
    {
        /// Just store the extension name
        Extensions.push_back({ExtensionName, 0, 0});
    }
}

void FExtensionVector::BuildPNextChain(BaseVulkanStructure* CreateInfo)
{
    bool bFirstExtension = true;
    for (uint32_t i = 0; i < Extensions.size(); ++i)
    {
        /// Fetch extension
        auto& Extension = Extensions[i];
        /// If this extension has a structure that was passed as argument
        if (Extension.ExtensionStructureSize)
        {
            /// If it's first extension
            if (bFirstExtension)
            {
                if (CreateInfo != nullptr)
                {
                    CreateInfo->pNext = &ExtensionsData[Extension.ExtensionStructureOffset];
                }
                /// No longer first extension
                bFirstExtension = false;
            }
            else
            {
                /// Fetch previous extension
                auto& PreviousExtension = Extensions[i-1];
                /// Fetch data structure of the previous extension
                BaseVulkanStructure* PreviousExtensionStructure = reinterpret_cast<BaseVulkanStructure*>(&ExtensionsData[PreviousExtension.ExtensionStructureOffset]);
                /// Make it point to the current one
                PreviousExtensionStructure->pNext = &ExtensionsData[Extension.ExtensionStructureOffset];
            }
        }
    }
}

std::vector<const char*> FExtensionVector::GetExtensionsNamesList()
{
    std::vector<const char *> ExtensionsList;

    for (uint32_t i = 0; i < Extensions.size(); ++i) {
        ExtensionsList.push_back(Extensions[i].ExtensionName.c_str());
    }

    return ExtensionsList;
}

void FInstanceOptions::AddLayer(const std::string& LayerName)
{
    Layers.AddString(LayerName);
}

std::vector<const char*> FInstanceOptions::GetLayers()
{
    return Layers.GetPointers();
}

void FInstanceOptions::AddInstanceExtension(const std::string& ExtensionName, void* ExtensionStructure, uint32_t ExtensionStructureSize)
{
    ExtensionVector.AddExtension(ExtensionName, ExtensionStructure, ExtensionStructureSize);
}

void FDeviceOptions::AddLayer(const std::string& LayerName)
{
    Layers.AddString(LayerName);
}

std::vector<const char*> FDeviceOptions::GetLayers()
{
    return Layers.GetPointers();
}

void FDeviceOptions::RequestQueueSupport(uint32_t QueueFamilyIndex)
{
    QueueFamilyIndices.emplace(QueueFamilyIndex);
}

void FDeviceOptions::AddDeviceExtension(const std::string& ExtensionName, void* ExtensionStructure, uint32_t ExtensionStructureSize)
{
    ExtensionVector.AddExtension(ExtensionName, ExtensionStructure, ExtensionStructureSize);
}

void FVulkanContextOptions::AddInstanceLayer(const std::string& InstanceLayerName)
{
    InstanceOptions.AddLayer(InstanceLayerName);
}

void FVulkanContextOptions::AddInstanceExtension(const std::string& ExtensionName, void* ExtensionStructure, uint32_t ExtensionStructureSize)
{
    InstanceOptions.AddInstanceExtension(ExtensionName, ExtensionStructure, ExtensionStructureSize);
}

std::vector<const char*> FVulkanContextOptions::GetInstanceLayers()
{
    return InstanceOptions.GetLayers();
}

std::vector<const char*> FVulkanContextOptions::GetInstanceExtensionsList()
{
    return InstanceOptions.ExtensionVector.GetExtensionsNamesList();
}

void FVulkanContextOptions::BuildInstancePNextChain(BaseVulkanStructure* CreateInfo)
{
    InstanceOptions.ExtensionVector.BuildPNextChain(CreateInfo);
}


void FVulkanContextOptions::AddDeviceLayer(const std::string& DeviceLayerName)
{
    DeviceOptions.AddLayer(DeviceLayerName);
}

void FVulkanContextOptions::AddDeviceExtension(const std::string& ExtensionName, void* ExtensionStructure, uint32_t ExtensionStructureSize)
{
    DeviceOptions.AddDeviceExtension(ExtensionName, ExtensionStructure, ExtensionStructureSize);
}

std::vector<const char*> FVulkanContextOptions::GetDeviceLayers()
{
    return DeviceOptions.GetLayers();
}

std::vector<const char*> FVulkanContextOptions::GetDeviceExtensionsList()
{
    return DeviceOptions.ExtensionVector.GetExtensionsNamesList();
}

void FVulkanContextOptions::BuildDevicePNextChain(BaseVulkanStructure* CreateInfo)
{
    DeviceOptions.ExtensionVector.BuildPNextChain(CreateInfo);
}

std::string ReadFileToString(const std::string& FileName)
{
    std::ifstream File(FileName, std::ios::ate | std::ios::binary);

    if (!File.is_open())
    {
        throw std::runtime_error("Failed to open file!");
    }

    std::size_t FileSize = (std::size_t)File.tellg();
    std::string Buffer;
    Buffer.resize(FileSize);

    File.seekg(0);
    File.read(Buffer.data(), FileSize);
    File.close();

    return Buffer;
}

FTimer::FTimer(const std::string &TextToPrint)
{
    NamedEvents.push_back({std::chrono::high_resolution_clock::now(), TextToPrint});
}

void FTimer::operator()(const std::string& TextToPrint)
{
    NamedEvents.push_back({std::chrono::high_resolution_clock::now(), TextToPrint});
}

FTimer::~FTimer()
{
    static std::vector<std::string> TimeUnits = {"us", "ms", "s"};

    for (int i = 0; i < NamedEvents.size() - 1; ++i)
    {
        auto Start = std::chrono::time_point_cast<std::chrono::microseconds>(NamedEvents[i].first).time_since_epoch().count();
        auto End = std::chrono::time_point_cast<std::chrono::microseconds>(NamedEvents[i + 1].first).time_since_epoch().count();

        auto Duration = static_cast<double>(End - Start);

        int TimeUnitUsed = 0;

        if (Duration > 1000)
        {
            Duration /= 1000;
            TimeUnitUsed++;
        }

        if (Duration > 1000)
        {
            Duration /= 1000;
            TimeUnitUsed++;
        }

        if (Duration > 1000)
        {
            Duration /= 1000;
            TimeUnitUsed++;
        }

        std::cout << NamedEvents[i].second << Duration << TimeUnits[TimeUnitUsed] << std::endl;
    }

    auto EndTime = std::chrono::high_resolution_clock::now();

    auto Start = std::chrono::time_point_cast<std::chrono::microseconds>(NamedEvents.back().first).time_since_epoch().count();
    auto End = std::chrono::time_point_cast<std::chrono::microseconds>(EndTime).time_since_epoch().count();

    auto Duration = static_cast<double>(End - Start);

    int TimeUnitUsed = 0;

    if (Duration > 1000)
    {
        Duration /= 1000;
        TimeUnitUsed++;
    }

    if (Duration > 1000)
    {
        Duration /= 1000;
        TimeUnitUsed++;
    }

    if (Duration > 1000)
    {
        Duration /= 1000;
        TimeUnitUsed++;
    }

    std::cout << NamedEvents.back().second << Duration << TimeUnits[TimeUnitUsed] << std::endl;
}
