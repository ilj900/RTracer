#pragma once

#include "vulkan/vulkan.h"
#include "image.h"

#include <memory>
#include <vector>

struct FSwapchain
{
    FSwapchain(uint32_t Width, uint32_t Height, VkPhysicalDevice PhysicalDevice, VkDevice LogicalDevice,
               VkSurfaceKHR Surface, uint32_t GraphicsQueueFamilyIndex, uint32_t PresentQueueFamilyIndex,
               VkFormat Format, VkColorSpaceKHR ColorSpace, VkPresentModeKHR PresentMode);
    ~FSwapchain();
    size_t Size();
    VkResult GetNextImage(ImagePtr Image, VkSemaphore &Semaphore, uint32_t& ImageIndex);
    VkFormat GetImageFormat();
    uint32_t GetWidth();
    uint32_t GetHeight();
    VkExtent2D GetExtent2D();
    std::vector<ImagePtr> GetImages();
    VkSwapchainKHR GetSwapchain();


    VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
    VkSurfaceFormatKHR SurfaceFormat;
    VkPresentModeKHR PresentMode;
    VkExtent2D Extent;
    std::vector<ImagePtr> Images;

    VkDevice LogicalDevice = VK_NULL_HANDLE;
};