#include "physical_device.h"

#include <cassert>

using namespace VKH;


FPhysicalDevice::FPhysicalDevice(VkPhysicalDevice Device) :
    Device(Device)
{
}

void FPhysicalDevice::QuerryParameters()
{
    vkGetPhysicalDeviceProperties(Device, &DeviceProperties);
    vkGetPhysicalDeviceFeatures(Device, &DeviceFeatures);

    uint32_t QueueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, nullptr);
    QueueFamilyProperties.resize(QueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, QueueFamilyProperties.data());
}

bool FPhysicalDevice::HasGeometryShaderSupport() const
{
    return DeviceFeatures.geometryShader;
}

bool FPhysicalDevice::HasGraphicsSupport()
{
    for (uint32_t i = 0; i < QueueFamilyProperties.size(); ++i)
    {
        if ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
        {
            return true;
        }
    }
    return false;
}

bool FPhysicalDevice:: HasComputeSupport()
{
    for (uint32_t i = 0; i < QueueFamilyProperties.size(); ++i)
    {
        if ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT)
        {
            return true;
        }
    }
    return false;
}

bool FPhysicalDevice::HasTransferSupport()
{
    for (uint32_t i = 0; i < QueueFamilyProperties.size(); ++i)
    {
        if ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
        {
            return true;
        }
    }
    return false;
}
bool FPhysicalDevice::HasSparseBindingSupport()
{
    for (uint32_t i = 0; i < QueueFamilyProperties.size(); ++i)
    {
        if ((QueueFamilyProperties[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) == VK_QUEUE_SPARSE_BINDING_BIT)
        {
            return true;
        }
    }
    return false;
}

uint32_t FPhysicalDevice::GetQueueIndexWith(bool GraphicsQueue, bool ComputeQueue, bool TransferQueue, bool SparseBindingQueue) const
{
    assert((GraphicsQueue || ComputeQueue || TransferQueue || SparseBindingQueue) != false && "You asking for queue with no properties!");
    VkQueueFlags Pattern{};

    if (GraphicsQueue)
    {
        Pattern = Pattern & VK_QUEUE_GRAPHICS_BIT;
    }

    if (ComputeQueue)
    {
        Pattern = Pattern & VK_QUEUE_COMPUTE_BIT;
    }

    if (TransferQueue)
    {
        Pattern = Pattern & VK_QUEUE_TRANSFER_BIT;
    }

    if (SparseBindingQueue)
    {
        Pattern = Pattern & VK_QUEUE_SPARSE_BINDING_BIT;
    }

    for (uint32_t i = 0; i < QueueFamilyProperties.size(); ++i)
    {
        if ((QueueFamilyProperties[i].queueFlags & Pattern) == Pattern)
        {
            return i;
        }
    }

    return UINT32_MAX;
}