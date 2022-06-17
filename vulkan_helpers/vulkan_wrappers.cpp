#include "vulkan_wrappers.h"

#include <cassert>

namespace V
{
    void FInstanceCreationOptions::AddLayer(const std::string& LayerName)
    {
        Layers.push_back(LayerName);
    }

    void FInstanceCreationOptions::AddInstanceExtension(const std::string& ExtensionName, void* ExtensionStructure, uint32_t ExtensionStructureSize)
    {
        if (ExtensionStructure)
        {
            Extensions.push_back({ExtensionName, ExtensionStructureSize, ExtensionsData.size()});

            auto StartingPosition = ExtensionsData.size();
            ExtensionsData.resize(ExtensionsData.size() + ExtensionStructureSize);
            memcpy(&ExtensionsData[StartingPosition], ExtensionStructure, ExtensionStructureSize);
        }
        else
        {
            Extensions.push_back({ExtensionName, 0, 0});
        }
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

        /// Just a utility structure
        struct BaseVulkanStructure
        {
            VkStructureType                         sType;
            const void*                             pNext;
        };

        /// Process instance extensions
        std::vector<const char*> CharExtensions;
        bool bFirstExtension = true;
        for (uint32_t i = 0; i < Options.Extensions.size(); ++i)
        {
            /// Fetch extension
            auto& Extension = Options.Extensions[i];
            /// Push extension name to the extension list
            CharExtensions.push_back(Extension.ExtensionName.c_str());
            /// If this extension has a structure that was passed as argument
            if (Extension.ExtensionStructureSize)
            {
                /// If it's first extension
                if (bFirstExtension)
                {
                    /// Make the CreateInfo pNext point to this structure
                    CreateInfo.pNext = &Options.ExtensionsData[Extension.ExtensionStructureOffset];
                    /// No longer first extension
                    bFirstExtension = false;
                }
                else
                {
                    /// Fetch previous extension
                    auto& PreviousExtension = Options.Extensions[i-1];
                    /// Fetch data structure of the previous extension
                    BaseVulkanStructure* PreviousExtensionStructure = reinterpret_cast<BaseVulkanStructure*>(&Options.ExtensionsData[PreviousExtension.ExtensionStructureOffset]);
                    /// Make it point to the current one
                    PreviousExtensionStructure->pNext = &Extension;
                }
            }
        }

        CreateInfo.enabledExtensionCount = static_cast<uint32_t>(Options.Extensions.size());
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
}

