#include "vulkan_wrappers.h"

#include <cassert>
#include <set>

namespace V
{

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
        /// Process instance extensions
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
                    /// Make the CreateInfo pNext point to this structure
                    CreateInfo->pNext = &ExtensionsData[Extension.ExtensionStructureOffset];
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
                    PreviousExtensionStructure->pNext = &Extension;
                }
            }
        }
    }

    std::vector<const char*> FExtensionVector::GetExtensionSNamesList()
    {
        std::vector<const char*> ExtensionsList;

        for (uint32_t i = 0; i < Extensions.size(); ++i)
        {
            ExtensionsList.push_back(Extensions[i].ExtensionName.c_str());
        }

        return ExtensionsList;
    }

    void FInstanceCreationOptions::AddLayer(const std::string& LayerName)
    {
        Layers.push_back(LayerName);
    }

    void FInstanceCreationOptions::AddInstanceExtension(const std::string& ExtensionName, void* ExtensionStructure, uint32_t ExtensionStructureSize)
    {
        ExtensionVector.AddExtension(ExtensionName, ExtensionStructure, ExtensionStructureSize);
    }

    void FLogicalDeviceOptions::AddLayer(const std::string& LayerName)
    {
        Layers.push_back(LayerName);
    }

    void FLogicalDeviceOptions::RequestQueueSupport(uint32_t QueueFamilyIndex)
    {
        QueueFamilyIndices.emplace(QueueFamilyIndex);
    }

    void FLogicalDeviceOptions::AddDeviceExtension(const std::string& ExtensionName, void* ExtensionStructure, uint32_t ExtensionStructureSize)
    {
        ExtensionVector.AddExtension(ExtensionName, ExtensionStructure, ExtensionStructureSize);
    }

    /// Vulkan main functionality functions

    VkInstance CreateInstance(const std::string& AppName, const FVersion3& AppVersion, const std::string& EngineName, const FVersion3& EngineVersion, uint32_t ApiVersion, FInstanceCreationOptions& Options)
    {
        /// Check supported Layers
        {
            /// Fetch all available layers
            uint32_t LayerCount;
            vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);

            std::vector<VkLayerProperties> AvailableLayers(LayerCount);
            vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

            for (const auto &Layer : Options.Layers) {
                bool LayerFound = false;

                for (const auto &LayerProperties : AvailableLayers) {
                    if (Layer == LayerProperties.layerName) {
                        LayerFound = true;
                        break;
                    }
                }
                assert(LayerFound && "Validation layers requested, but not available!");
            }
        }

        /// Fill in instance creation data
        VkApplicationInfo AppInfo{};
        AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        AppInfo.pApplicationName = AppName.c_str();
        AppInfo.applicationVersion = VK_MAKE_VERSION(AppVersion.Major, AppVersion.Minor, AppVersion.Patch);
        AppInfo.pEngineName = EngineName.c_str();
        AppInfo.engineVersion = VK_MAKE_VERSION(EngineVersion.Major, EngineVersion.Minor, EngineVersion.Patch);
        AppInfo.apiVersion = ApiVersion;

        VkInstanceCreateInfo CreateInfo{};
        CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        CreateInfo.pApplicationInfo = &AppInfo;

        /// Process instance layers
        std::vector<const char*> CharLayers;

        for (const auto& Layer : Options.Layers)
        {
            CharLayers.push_back(Layer.c_str());
        }

        if (!Options.Layers.empty())
        {
            CreateInfo.enabledLayerCount = static_cast<uint32_t>(Options.Layers.size());
            CreateInfo.ppEnabledLayerNames = CharLayers.data();
        }
        else
        {
            CreateInfo.enabledLayerCount = 0;
        }

        Options.ExtensionVector.BuildPNextChain(reinterpret_cast<BaseVulkanStructure*>(&CreateInfo));
        std::vector<const char*> CharExtensions = Options.ExtensionVector.GetExtensionSNamesList();

        CreateInfo.enabledExtensionCount = static_cast<uint32_t>(Options.ExtensionVector.Extensions.size());
        CreateInfo.ppEnabledExtensionNames = CharExtensions.data();

        VkInstance Instance;

        VkResult Result = vkCreateInstance(&CreateInfo, nullptr, &Instance);

        assert((Result == VK_SUCCESS) && "Failed to create instance!");

        return Instance;
    }

    VkSampleCountFlagBits GetMaxMsaa(VkPhysicalDevice PhysicalDevice)
    {
        VkPhysicalDeviceProperties PhysicalDeviceProperties;

        vkGetPhysicalDeviceProperties(PhysicalDevice, &PhysicalDeviceProperties);

        VkSampleCountFlags Count = PhysicalDeviceProperties.limits.framebufferColorSampleCounts & PhysicalDeviceProperties.limits.framebufferDepthSampleCounts;

        if (Count & VK_SAMPLE_COUNT_64_BIT)
        {
            return VK_SAMPLE_COUNT_64_BIT;
        }
        else if (Count & VK_SAMPLE_COUNT_32_BIT)
        {
            return VK_SAMPLE_COUNT_32_BIT;
        }
        else if (Count & VK_SAMPLE_COUNT_16_BIT)
        {
            return VK_SAMPLE_COUNT_16_BIT;
        }
        else if (Count & VK_SAMPLE_COUNT_8_BIT)
        {
            return VK_SAMPLE_COUNT_8_BIT;
        }
        else if (Count & VK_SAMPLE_COUNT_4_BIT)
        {
            return VK_SAMPLE_COUNT_4_BIT;
        }
        else if (Count & VK_SAMPLE_COUNT_2_BIT)
        {
            return VK_SAMPLE_COUNT_2_BIT;
        }
        else
        {
            return VK_SAMPLE_COUNT_1_BIT;
        }
    }

    bool CheckQueueTypeSupport(VkPhysicalDevice PhysicalDevice, VkQueueFlagBits Type, uint32_t& QueueFamilyIndex)
    {
        /// Fetch all queue families
        uint32_t QueueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> QueueFamilyProperties(QueueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, QueueFamilyProperties.data());

        /// If any queue family has appropriate flag, then the queue is supported
        for (uint32_t i = 0; i < QueueFamilyCount; ++i)
        {
            if ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT) {
                QueueFamilyIndex = i;
                return true;
            }
        }

        return false;
    }

    bool CheckPresentQueueSupport(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface, uint32_t& QueueFamilyIndex)
    {
        /// Fetch all queue families
        uint32_t QueueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> QueueFamilyProperties(QueueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, QueueFamilyProperties.data());

        /// If any queue family has appropriate flag, then the queue is supported
        for (uint32_t i = 0; i < QueueFamilyCount; ++i)
        {
            VkBool32 PresentSupport = false;

            vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, i, Surface, &PresentSupport);

            if (PresentSupport)
            {
                QueueFamilyIndex = i;
                return true;
            }
        }

        return false;
    }

    std::vector<VkPhysicalDevice> GetAllPhysicalDevices(VkInstance Instance)
    {
        uint32_t PhysicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, nullptr);

        assert(PhysicalDeviceCount && "Failed to find GPUs with Vulkan support!");

        std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDeviceCount);
        vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, PhysicalDevices.data());

        return PhysicalDevices;
    }

    bool CheckDeviceExtensionSupport(VkPhysicalDevice PhysicalDevice, std::vector<std::string> RequiredDeviceExtensions)
    {
        uint32_t ExtensionCount = 0;
        vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &ExtensionCount, nullptr);

        std::vector<VkExtensionProperties>AvailableExtensions(ExtensionCount);
        vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &ExtensionCount, AvailableExtensions.data());

        std::set<std::string> RequiredExtensions(RequiredDeviceExtensions.begin(), RequiredDeviceExtensions.end());

        for (const auto& Extension : AvailableExtensions)
        {
            RequiredExtensions.erase(Extension.extensionName);
        }

        return RequiredExtensions.empty();
    }

    VkDevice CreateLogicalDevice(VkPhysicalDevice PhysicalDevice, FLogicalDeviceOptions& Options)
    {
        std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;

        float QueuePriority = 1.f;
        for (auto QueueFamilyIndex : Options.QueueFamilyIndices)
        {
            VkDeviceQueueCreateInfo QueueCreateInfo{};
            QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            QueueCreateInfo.queueFamilyIndex = QueueFamilyIndex;
            QueueCreateInfo.queueCount = 1;
            QueueCreateInfo.pQueuePriorities = &QueuePriority;
            QueueCreateInfos.push_back(QueueCreateInfo);
        }

        VkPhysicalDeviceFeatures DeviceFeatures{};
        DeviceFeatures.samplerAnisotropy = VK_TRUE;
        DeviceFeatures.sampleRateShading = VK_TRUE;

        VkDeviceCreateInfo CreateInfo{};
        CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        CreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
        CreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());

        /// Process device extensions
        Options.ExtensionVector.BuildPNextChain(reinterpret_cast<BaseVulkanStructure*>(&CreateInfo));
        std::vector<const char*> CharExtensions = Options.ExtensionVector.GetExtensionSNamesList();

        CreateInfo.pEnabledFeatures = &DeviceFeatures;
        CreateInfo.enabledExtensionCount = static_cast<uint32_t>(Options.ExtensionVector.Extensions.size());
        CreateInfo.ppEnabledExtensionNames = CharExtensions.data();

        std::vector<const char*> CharLayers;
        for (const auto& Layer : Options.Layers)
        {
            CharLayers.push_back(Layer.c_str());
        }

        if (!Options.Layers.empty())
        {
            CreateInfo.enabledLayerCount = static_cast<uint32_t>(Options.Layers.size());
            CreateInfo.ppEnabledLayerNames = CharLayers.data();
        }
        else
        {
            CreateInfo.enabledLayerCount = 0;
        }

        VkDevice LogicalDevice = nullptr;

        VkResult Result = vkCreateDevice(PhysicalDevice, &CreateInfo, nullptr, &LogicalDevice);

        assert((Result == VK_SUCCESS) && "Failed to create logical device!");

        return LogicalDevice;
    }

}

