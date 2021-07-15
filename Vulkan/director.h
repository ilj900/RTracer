#pragma once

#include "instance.h"
#include "physical_device.h"
#include "logical_device.h"

#include <memory>
#include <vector>

namespace VKH
{
    class FDirector
    {
    public:
        FDirector();
        ~FDirector();

        void Init();

        void PickPhysicalDevice();
        void CreateLogicalDevice();
        void CreateSurface();
        void CreateSwapChain();
        void CreateRenderPass();




        void CreateImage(std::string& Path);
        void CreateImageView();
        void CreateBuffer();

    private:
        std::unique_ptr<FInstance> Instance;
        std::shared_ptr<FPhysicalDevice> PhysicalDevice;
        std::shared_ptr<FLogicalDevice> LogicalDevice;
        std::shared_ptr<VkSurfaceKHR> Surface;

        bool bGraphicsQueueRequired = true;
        bool NumberOfGraphicsQueueRequired = 1;
    };

    FDirector* GetDirector();
}