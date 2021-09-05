#pragma once

#include "vulkan/vulkan.h"

#include <vector>

class FContext;

struct FSwapchain
{
    FSwapchain(FContext &Context, VkPhysicalDevice PhysicalDevice, VkDevice LogicalDevice, VkSurfaceKHR Surface,
               GLFWwindow* Window, uint32_t GraphicsQueueFamilyIndex, uint32_t PresentQueueFamilyIndex,
               VkFormat Format, VkColorSpaceKHR ColorSpace, VkPresentModeKHR PresentMode);
    ~FSwapchain();
    uint32_t Size();
    VkImage GetNextImage();
    VkFormat GetImageFormat();
    uint32_t GetWidth();
    uint32_t GetHeight();
    VkExtent2D GetExtent2D();
    std::vector<VkImageView>& GetImageViews();
    VkSwapchainKHR GetSwapchain();


    VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
    VkSurfaceFormatKHR SurfaceFormat;
    VkPresentModeKHR PresentMode;
    VkExtent2D Extent;
    std::vector<VkImage> Images;
    std::vector<VkImageView> ImageViews;
    std::vector<VkSemaphore> ImageAvailableSemaphores;
    uint32_t CurrentImage = 0;

    VkDevice LogicalDevice = VK_NULL_HANDLE;
};