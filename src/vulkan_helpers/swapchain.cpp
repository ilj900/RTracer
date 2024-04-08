#include "GLFW/glfw3.h"

#include "swapchain.h"
#include "vk_context.h"
#include "vk_debug.h"

#include <stdexcept>

FSwapchain::FSwapchain(uint32_t Width, uint32_t Height, VkPhysicalDevice PhysicalDevice, VkDevice LogicalDevice,
                       VkSurfaceKHR Surface, uint32_t GraphicsQueueFamilyIndex, uint32_t PresentQueueFamilyIndex,
                       VkFormat Format, VkColorSpaceKHR ColorSpace, VkPresentModeKHR PresentMode) :
                       LogicalDevice(LogicalDevice)
{
    /// Find out supported formats and choose the one we need
    {
        uint32_t FormatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &FormatCount, nullptr);

        std::vector<VkSurfaceFormatKHR> SurfaceFormats;
        if (FormatCount != 0) {
            SurfaceFormats.resize(FormatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &FormatCount, SurfaceFormats.data());
        }

        SurfaceFormat = SurfaceFormats[0];

        for (const auto &AvailableSurfaceFormat : SurfaceFormats) {
            if (AvailableSurfaceFormat.format == Format && AvailableSurfaceFormat.colorSpace == ColorSpace) {
                SurfaceFormat = AvailableSurfaceFormat;
            }
        }
    }

    /// Find out supported present modes and select the one we need
    {
        uint32_t PresentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, nullptr);

        std::vector<VkPresentModeKHR> PresentModes;
        if (PresentModeCount != 0) {
            PresentModes.resize(PresentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, PresentModes.data());
        }

        this->PresentMode = VK_PRESENT_MODE_FIFO_KHR;

        for (const auto& AvailablePresentMode : PresentModes)
        {
            if (AvailablePresentMode == PresentMode)
            {
                this->PresentMode = AvailablePresentMode;
            }
        }
    }

    /// Request surface properties and pick a size
    VkSurfaceCapabilitiesKHR SurfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &SurfaceCapabilities);

    if (SurfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        Extent = SurfaceCapabilities.currentExtent;
    }
    else
    {
        VkExtent2D ActualExtent = {static_cast<uint32_t>(Width), static_cast<uint32_t>(Height)};
        ActualExtent.width = std::max(SurfaceCapabilities.minImageExtent.width, std::min(SurfaceCapabilities.minImageExtent.width, ActualExtent.width));
        ActualExtent.height = std::max(SurfaceCapabilities.minImageExtent.height, std::min(SurfaceCapabilities.minImageExtent.height, ActualExtent.height));
        Extent = ActualExtent;
    }

    /// Decide how many images we need
    uint32_t ImageCount = SurfaceCapabilities.minImageCount + 1;

    if (SurfaceCapabilities.maxImageCount > 0 && ImageCount > SurfaceCapabilities.maxImageCount)
    {
        ImageCount = SurfaceCapabilities.maxImageCount;
    }

    /// Create SwapChain
    VkSwapchainCreateInfoKHR CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    CreateInfo.surface = Surface;

    CreateInfo.minImageCount = ImageCount;
    CreateInfo.imageFormat = SurfaceFormat.format;
    CreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
    CreateInfo.imageExtent = Extent;
    CreateInfo.imageArrayLayers = 1;
    CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    uint32_t QueueFamilyIndices[] = {GraphicsQueueFamilyIndex, PresentQueueFamilyIndex};
    if (GraphicsQueueFamilyIndex != PresentQueueFamilyIndex)
    {
        CreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        CreateInfo.queueFamilyIndexCount = 2;
        CreateInfo.pQueueFamilyIndices = QueueFamilyIndices;
    }
    else
    {
        CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        CreateInfo.queueFamilyIndexCount = 0;
        CreateInfo.pQueueFamilyIndices = nullptr;
    }

    CreateInfo.preTransform = SurfaceCapabilities.currentTransform;
    CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    CreateInfo.presentMode = this->PresentMode;
    CreateInfo.clipped = VK_TRUE;

    CreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(LogicalDevice, &CreateInfo, nullptr, &Swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create swap chain!");
    }

    V::SetName(LogicalDevice, Swapchain, "");


    /// Queue SwapChain for it's images
    vkGetSwapchainImagesKHR(LogicalDevice, Swapchain, &ImageCount, nullptr);
    std::vector<VkImage> SwapchainImages(ImageCount);
    vkGetSwapchainImagesKHR(LogicalDevice, Swapchain, &ImageCount, SwapchainImages.data());
    Images.resize(ImageCount);

    for (uint32_t i = 0; i < SwapchainImages.size(); ++i)
    {
        Images[i] = VK_CONTEXT().Wrap(SwapchainImages[i], SurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, LogicalDevice, "V_SwapchainImage");
    }
}

FSwapchain::~FSwapchain()
{
    Images.clear();
    vkDestroySwapchainKHR(LogicalDevice, Swapchain, nullptr);
}

size_t FSwapchain::Size()
{
    return Images.size();
}

VkFormat FSwapchain::GetImageFormat()
{
    return SurfaceFormat.format;
}

uint32_t FSwapchain::GetWidth()
{
    return Extent.width;
}

uint32_t FSwapchain::GetHeight()
{
    return Extent.height;
}

VkExtent2D FSwapchain::GetExtent2D()
{
    return Extent;
}

std::vector<ImagePtr> FSwapchain::GetImages()
{
    return Images;
}

VkSwapchainKHR FSwapchain::GetSwapchain()
{
    return Swapchain;
}

VkResult FSwapchain::GetNextImage(ImagePtr Image, VkSemaphore &Semaphore, uint32_t& ImageIndex)
{
    VkResult Result = vkAcquireNextImageKHR(VK_CONTEXT().LogicalDevice, Swapchain, UINT64_MAX, Semaphore, VK_NULL_HANDLE, &ImageIndex);
    if (Image != nullptr)
    {
        Image = Images[ImageIndex];
    }
    return Result;
}