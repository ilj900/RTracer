#pragma once

#include "vulkan/vulkan.h"
#include "image.h"

#include <vector>

class FVulkanContext;

struct FSwapchain
{
    FSwapchain(FVulkanContext &Context, VkPhysicalDevice PhysicalDevice, VkDevice LogicalDevice, VkSurfaceKHR Surface,
               GLFWwindow* Window, uint32_t GraphicsQueueFamilyIndex, uint32_t PresentQueueFamilyIndex,
               VkFormat Format, VkColorSpaceKHR ColorSpace, VkPresentModeKHR PresentMode);
    ~FSwapchain();
    size_t Size();
    VkResult GetNextImage(VkImage& Image, VkSemaphore &Semaphore, uint32_t& ImageIndex);
    VkFormat GetImageFormat();
    uint32_t GetWidth();
    uint32_t GetHeight();
    VkExtent2D GetExtent2D();
    std::vector<FImage>& GetImages();
    VkSwapchainKHR GetSwapchain();


    VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
    VkSurfaceFormatKHR SurfaceFormat;
    VkPresentModeKHR PresentMode;
    VkExtent2D Extent;
    std::vector<FImage> Images;

    VkDevice LogicalDevice = VK_NULL_HANDLE;
};