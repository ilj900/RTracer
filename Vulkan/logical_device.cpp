#include "logical_device.h"

using namespace VKH;

FLogicalDevice::FLogicalDevice(std::shared_ptr<FPhysicalDevice> PhysicalDevice):
    PhysicalDevice(PhysicalDevice)
{
}

FLogicalDevice& FLogicalDevice::Init()
{
     VkDeviceCreateInfo LogicalDeviceCreateInfo{};

     vkCreateDevice(PhysicalDevice->Device, &LogicalDeviceCreateInfo, nullptr, &LogicalDevice);

     return *this;
}