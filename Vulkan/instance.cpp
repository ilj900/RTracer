#include "instance.h"

#include <stdexcept>

using namespace VKH;

FInstance::FInstance(const std::string& Name, uint32_t APIVersionMajor, uint32_t APIVersionMinor, uint32_t APIVersionPatch) :
    Name(Name), Major(Major), Minor(Minor), Patch(Patch)
{
}

FInstance::~FInstance()
{
    if (Instance)
    {
        vkDestroyInstance(Instance, nullptr);
    }
}

void FInstance::Init()
{
    uint32_t AvailableVersion;
    vkEnumerateInstanceVersion(&AvailableVersion);
    if (AvailableVersion <= VK_MAKE_VERSION(Major, Minor, Patch))
    {
        throw std::runtime_error("Desired API version not supported!");
    }

    VkApplicationInfo ApplicationInfo{};
    ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    ApplicationInfo.apiVersion = VK_MAKE_VERSION(Major, Minor, Patch);
    ApplicationInfo.pApplicationName = Name.c_str();

    VkInstanceCreateInfo InstanceCreateInfo{};
    InstanceCreateInfo.pApplicationInfo = &ApplicationInfo;
    InstanceCreateInfo.ppEnabledExtensionNames = Extensions.data();
    InstanceCreateInfo.enabledExtensionCount = Extensions.size();
    InstanceCreateInfo.ppEnabledLayerNames = Layers.data();
    InstanceCreateInfo.enabledLayerCount = Layers.size();

    auto Result = vkCreateInstance(&InstanceCreateInfo, nullptr, &Instance);
    if (Result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create an Instance!");
    }
}

void FInstance::InstancePushExtension(const char* ExtensionName)
{
    for (const auto& Extension : Extensions)
    {
        if (Extension == ExtensionName)
            return;
    }
    Extensions.push_back(ExtensionName);
}

void FInstance::InstancePushLayer(const char* LayerName)
{
    for (const auto& Layer : Layers)
    {
        if (Layer == LayerName)
            return;
    }
    Layers.push_back(LayerName);
}