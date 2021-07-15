#pragma once

#include "vulkan/vulkan.h"

#include "instance.h"

namespace VKH
{
    class FPhysicalDevice
    {
    public:
        FPhysicalDevice(VkPhysicalDevice Device);
        ~FPhysicalDevice() = default;

        bool HasGeometryShaderSupport() const;
        bool HasGraphicsSupport();
        bool HasComputeSupport();
        bool HasTransferSupport();
        bool HasSparseBindingSupport();
        uint32_t GetQueueIndexWith(bool GraphicsQueue = false, bool ComputeQueue = false, bool TransferQueue = false, bool SparseBindingQueue = false) const;

        void QuerryParameters();
    public:
        VkPhysicalDevice Device;
        VkPhysicalDeviceProperties DeviceProperties;
        VkPhysicalDeviceFeatures DeviceFeatures;
        std::vector<VkQueueFamilyProperties> QueueFamilyProperties;
    };
}