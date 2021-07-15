#pragma once

#include "physical_device.h"

#include <memory>

namespace VKH
{
    class FLogicalDevice
    {
    public:
        FLogicalDevice(std::shared_ptr<FPhysicalDevice> PhysicalDevice);
        ~FLogicalDevice();

        FLogicalDevice& Init();

    public:
        std::shared_ptr<FPhysicalDevice> PhysicalDevice;

        VkDevice LogicalDevice;
        VkQueue GraphicsQueue;
    };
}