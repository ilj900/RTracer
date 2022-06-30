#include "vulkan_wrappers.h"

#include <cassert>
#include <cmath>
#include <set>

namespace V
{
    void FStringStorage::AddString(const std::string& String)
    {
        Strings.push_back(String);
    }

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

    std::vector<const char*> FExtensionVector::GetExtensionsNamesList()
    {
        std::vector<const char *> ExtensionsList;

        for (uint32_t i = 0; i < Extensions.size(); ++i) {
            ExtensionsList.push_back(Extensions[i].ExtensionName.c_str());
        }

        return ExtensionsList;
    }

    void FInstanceCreationOptions::AddLayer(const std::string& LayerName)
    {
        Layers.AddString(LayerName);
    }

    std::vector<const char*> FInstanceCreationOptions::GetLayers()
    {
        return Layers.GetPointers();
    }

    void FInstanceCreationOptions::AddInstanceExtension(const std::string& ExtensionName, void* ExtensionStructure, uint32_t ExtensionStructureSize)
    {
        ExtensionVector.AddExtension(ExtensionName, ExtensionStructure, ExtensionStructureSize);
    }

    void FLogicalDeviceOptions::AddLayer(const std::string& LayerName)
    {
        Layers.AddString(LayerName);
    }

    std::vector<const char*> FLogicalDeviceOptions::GetLayers()
    {
        return Layers.GetPointers();
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
        auto InstanceLayers = Options.GetLayers();

        /// Check supported Layers
        {
            /// Fetch all available layers
            uint32_t LayerCount;
            vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);

            std::vector<VkLayerProperties> AvailableLayers(LayerCount);
            vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

            for ( auto Layer : InstanceLayers) {
                bool LayerFound = false;

                for (const auto &LayerProperties : AvailableLayers) {
                    if (std::string(Layer) == LayerProperties.layerName) {
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

        if (!InstanceLayers.empty())
        {
            CreateInfo.enabledLayerCount = static_cast<uint32_t>(InstanceLayers.size());
            CreateInfo.ppEnabledLayerNames = InstanceLayers.data();
        }
        else
        {
            CreateInfo.enabledLayerCount = 0;
        }

        Options.ExtensionVector.BuildPNextChain(reinterpret_cast<BaseVulkanStructure*>(&CreateInfo));
        std::vector<const char*> CharExtensions = Options.ExtensionVector.GetExtensionsNamesList();

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

        /// If set is empty, then all extensions are supported
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
        std::vector<const char*> CharExtensions = Options.ExtensionVector.GetExtensionsNamesList();

        CreateInfo.pEnabledFeatures = &DeviceFeatures;
        CreateInfo.enabledExtensionCount = static_cast<uint32_t>(Options.ExtensionVector.Extensions.size());
        CreateInfo.ppEnabledExtensionNames = CharExtensions.data();

        auto DeviceLayers = Options.GetLayers();

        if (!DeviceLayers.empty())
        {
            CreateInfo.enabledLayerCount = static_cast<uint32_t>(DeviceLayers.size());
            CreateInfo.ppEnabledLayerNames = DeviceLayers.data();
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

    VkCommandPool CreateCommandPool(VkDevice LogicalDevice, uint32_t QueueIndex)
    {
        VkCommandPool CommandPool;
        VkCommandPoolCreateInfo PoolInfo{};
        PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        PoolInfo.queueFamilyIndex = QueueIndex;
        PoolInfo.flags = 0;

        VkResult  Result = vkCreateCommandPool(LogicalDevice, &PoolInfo, nullptr, &CommandPool);

        assert((Result == VK_SUCCESS) && "Failed to create command pool!");

        return CommandPool;
    }

    VkCommandBuffer AllocateCommandBuffer(VkDevice LogicalDevice, VkCommandPool CommandPool)
    {
        VkCommandBuffer CommandBuffer;

        VkCommandBufferAllocateInfo AllocInfo{};
        AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        AllocInfo.commandPool = CommandPool;
        AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        AllocInfo.commandBufferCount = 1u;

        auto Result = vkAllocateCommandBuffers(LogicalDevice, &AllocInfo, &CommandBuffer);
        assert((Result == VK_SUCCESS) && "Failed to allocate command buffers!");

        return CommandBuffer;
    }

    VkCommandBuffer BeginWithAllocation(VkDevice LogicalDevice, VkCommandPool CommandPool)
    {
        VkCommandBuffer CommandBuffer = AllocateCommandBuffer(LogicalDevice, CommandPool);

        VkCommandBufferBeginInfo BeginInfo{};
        BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        BeginInfo.flags = 0;

        vkBeginCommandBuffer(CommandBuffer, &BeginInfo);

        return CommandBuffer;
    }

    VkCommandBuffer BeginSingleTimeCommand(VkDevice LogicalDevice, VkCommandPool CommandPool)
    {
        VkCommandBuffer CommandBuffer = AllocateCommandBuffer(LogicalDevice, CommandPool);

        VkCommandBufferBeginInfo BeginInfo{};
        BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(CommandBuffer, &BeginInfo);

        return CommandBuffer;
    }

    void SubmitCommandBuffer(VkDevice LogicalDevice, VkCommandPool CommandPool, VkQueue Queue, VkCommandBuffer &CommandBuffer)
    {
        VkSubmitInfo SubmitInfo{};
        SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        SubmitInfo.commandBufferCount = 1;
        SubmitInfo.pCommandBuffers = &CommandBuffer;

        vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(Queue);

        vkFreeCommandBuffers(LogicalDevice, CommandPool, 1, &CommandBuffer);
    }

    VkImage CreateImage(uint32_t Width, uint32_t Height, uint32_t MipsLevels, VkSampleCountFlagBits NumSamples, VkFormat Format, VkImageTiling Tiling, VkImageUsageFlags Usage, VkDevice Device)
    {
        VkImage Image;

        VkImageCreateInfo ImageInfo{};
        ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ImageInfo.imageType = VK_IMAGE_TYPE_2D;
        ImageInfo.extent.width = Width;
        ImageInfo.extent.height = Height;
        ImageInfo.extent.depth = 1;
        ImageInfo.mipLevels = MipsLevels;
        ImageInfo.arrayLayers = 1;
        ImageInfo.format = Format;
        ImageInfo.tiling = Tiling;
        ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        ImageInfo.usage = Usage;
        ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        ImageInfo.samples = NumSamples;

        auto Result = vkCreateImage(Device, &ImageInfo, nullptr, &Image);
        assert((Result == VK_SUCCESS) && "Failed to create image!");

        return Image;
    }

    VkDeviceMemory AllocateMemoryForTheImage(VkImage Image, VkDevice Device, VkMemoryPropertyFlags Properties, uint32_t MemoryTypeIndex)
    {
        VkDeviceMemory Memory;

        VkMemoryRequirements MemRequirements;
        vkGetImageMemoryRequirements(Device, Image, &MemRequirements);

        VkMemoryAllocateInfo AllocInfo{};
        AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        AllocInfo.allocationSize = MemRequirements.size;
        AllocInfo.memoryTypeIndex = MemoryTypeIndex;

        auto Result = vkAllocateMemory(Device, &AllocInfo, nullptr, &Memory);
        assert((Result == VK_SUCCESS) && "Failed to allocate image memory!");

        return Memory;
    }

    void BindMemoryToImage(VkDevice Device, VkImage Image, VkDeviceMemory Memory, VkDeviceSize MemoryOffset)
    {
        vkBindImageMemory(Device, Image, Memory, MemoryOffset);
    }

    VkImageView CreateImageView(VkDevice Device, VkImage Image, VkFormat Format, VkImageAspectFlags AspectFlags, uint32_t MipLevelsCount)
    {
        VkImageView View;

        VkImageViewCreateInfo ViewInfo{};
        ViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ViewInfo.image = Image;
        ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ViewInfo.format = Format;
        ViewInfo.subresourceRange.aspectMask = AspectFlags;
        ViewInfo.subresourceRange.baseMipLevel = 0;
        ViewInfo.subresourceRange.levelCount = MipLevelsCount;
        ViewInfo.subresourceRange.baseArrayLayer = 0;
        ViewInfo.subresourceRange.layerCount = 1;

        auto Result = vkCreateImageView(Device, &ViewInfo, nullptr, &View);
        assert((Result == VK_SUCCESS) && "Failed to create texture image view!");

        return View;
    }

    uint32_t FindMemoryType(VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties, uint32_t TypeFilter, VkMemoryPropertyFlags Properties)
    {
        for (uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryTypeCount; ++i)
        {
            if (TypeFilter & (1 << i) && (PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & Properties) == Properties)
            {
                return i;
            }
        }

        assert("Failed to find suitable memory type!");

        return std::numeric_limits<uint32_t>().max();
    }

    VkMemoryRequirements GetImageMemoryRequirements(VkDevice Device, VkImage Image)
    {
        VkMemoryRequirements MemRequirements;
        vkGetImageMemoryRequirements(Device, Image, &MemRequirements);

        return MemRequirements;
    }

    VkPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties(VkPhysicalDevice PhysicalDevice)
    {
        VkPhysicalDeviceMemoryProperties MemProperties;
        vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &MemProperties);
        return MemProperties;
    }

}

