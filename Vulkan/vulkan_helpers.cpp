#include "vulkan_helpers.h"

#include <cassert>

using namespace VKH;

void InstancePushExtension(const char* ExtensionName)
{
    for (const auto& Extension : Extensions)
    {
        if (Extension == ExtensionName)
            return;
    }
    Extensions.push_back(ExtensionName);
}

void InstancePushLayer(const char* LayerName)
{
    for (const auto& Layer : Layers)
    {
        if (Layer == LayerName)
            return;
    }
    Layers.push_back(LayerName);
}

VkInstance CreateInstance(const std::string& ApplicationName, uint32_t APIVersion)
{
    VkApplicationInfo ApplicationInfo{};
    ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    ApplicationInfo.apiVersion = APIVersion;
    ApplicationInfo.pApplicationName = ApplicationName.c_str();

    VkInstanceCreateInfo InstanceCreateInfo{};
    InstanceCreateInfo.pApplicationInfo = &ApplicationInfo;
    InstanceCreateInfo.ppEnabledExtensionNames = Extensions.data();
    InstanceCreateInfo.enabledExtensionCount = Extensions.size();
    InstanceCreateInfo.ppEnabledLayerNames = Layers.data();
    InstanceCreateInfo.enabledLayerCount = Layers.size();

    VkInstance Instance;
    auto Result = vkCreateInstance(&InstanceCreateInfo, nullptr, &Instance);
    assert(Result != VK_SUCCESS && "Failed to create an Instance!");

    return Instance;
}

void CreatePhysicalDevice()
{

}
