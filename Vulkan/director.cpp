#include "director.h"
#include "GLFW/glfw3.h"

#include <stdexcept>

using namespace VKH;

FDirector Director{};

FDirector* GetDirector()
{
    return &Director;
}

void FDirector::Init()
{
    Instance = std::make_unique<FInstance>("RTracer", 1, 2, 0);
    uint32_t ExtensionsCount = 0;
    const char** RequiredExtesions = glfwGetRequiredInstanceExtensions(&ExtensionsCount);
    for(uint32_t i = 0; i < ExtensionsCount; ++i)
    {
        Instance->InstancePushExtension(RequiredExtesions[i]);
    }
    Instance->Init();
    PickPhysicalDevice();
    CreateLogicalDevice();


}

void FDirector::PickPhysicalDevice()
{
    if (!Instance)
    {
        throw std::runtime_error("Instance not created!");
    }
    uint32_t DeviceCount = 0;
    vkEnumeratePhysicalDevices(Instance->Instance, &DeviceCount, nullptr);

    if (DeviceCount == 0)
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> QueriedDevices(DeviceCount);
    vkEnumeratePhysicalDevices(Instance->Instance, &DeviceCount, QueriedDevices.data());

    for (const auto Device : QueriedDevices)
    {
        PhysicalDevice = std::make_shared<FPhysicalDevice>(Device);
        PhysicalDevice->QuerryParameters();

        if (PhysicalDevice->HasGraphicsSupport())
        {
            break;
        }
    }
}

void FDirector::CreateLogicalDevice()
{
    auto GrapgicsQueueIndex = PhysicalDevice->GetQueueIndexWith(true);
    VkDeviceQueueCreateInfo QueueDeviceCreateInfo{};
    QueueDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    QueueDeviceCreateInfo.queueFamilyIndex = GrapgicsQueueIndex;
    QueueDeviceCreateInfo.queueCount = 1;

    float QueuePriority = 1.f;
    QueueDeviceCreateInfo.pQueuePriorities = &QueuePriority;

    VkPhysicalDeviceFeatures DeviceFeatures{};

    VkDeviceCreateInfo LogicalDeviceCreateInfo{};
    LogicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    LogicalDeviceCreateInfo.pQueueCreateInfos = &QueueDeviceCreateInfo;
    LogicalDeviceCreateInfo.pEnabledFeatures = &DeviceFeatures;
    LogicalDeviceCreateInfo.enabledExtensionCount = 0;

    LogicalDevice = std::make_shared<FLogicalDevice>(PhysicalDevice);

    vkCreateDevice(PhysicalDevice->Device, &LogicalDeviceCreateInfo, nullptr, &LogicalDevice->LogicalDevice);

    vkGetDeviceQueue(LogicalDevice->LogicalDevice, GrapgicsQueueIndex, 0, &LogicalDevice->GraphicsQueue);
}

void FDirector::CreateSurface()
{

}

void FDirector::CreateSwapChain()
{
}