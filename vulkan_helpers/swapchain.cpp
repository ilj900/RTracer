#include "GLFW/glfw3.h"

#include "swapchain.h"
#include "context.h"

#include <stdexcept>

FSwapchain::FSwapchain(FContext &Context, VkPhysicalDevice PhysicalDevice, VkDevice LogicalDevice, VkSurfaceKHR Surface,
                       GLFWwindow* Window, uint32_t GraphicsQueueFamilyIndex, uint32_t PresentQueueFamilyIndex,
                       VkFormat Format, VkColorSpaceKHR ColorSpace, VkPresentModeKHR PresentMode):
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

    /// Find out supported present modes and select the one wee need
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
        int Width, Height;

        glfwGetFramebufferSize(Window, &Width, &Height);

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
    CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

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

    /// Queue SwapChain for it's images
    vkGetSwapchainImagesKHR(LogicalDevice, Swapchain, &ImageCount, nullptr);
    Images.resize(ImageCount);
    vkGetSwapchainImagesKHR(LogicalDevice, Swapchain, &ImageCount, Images.data());

    /// Create ImageViews for SwapChain Images
    ImageViews.resize(ImageCount);

    for (std::size_t i = 0; i < ImageCount; ++i)
    {
        ImageViews[i] = Context.CreateImageView(Images[i], SurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

FSwapchain::~FSwapchain()
{
    for (auto ImageView : ImageViews)
    {
        vkDestroyImageView(LogicalDevice, ImageView, nullptr);
    }
    vkDestroySwapchainKHR(LogicalDevice, Swapchain, nullptr);
}

uint32_t FSwapchain::Size()
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

std::vector<VkImageView>& FSwapchain::GetImageViews()
{
    return ImageViews;
}

VkSwapchainKHR FSwapchain::GetSwapchain()
{
    return Swapchain;
}

VkImage FSwapchain::GetNextImage()
{
    uint32_t ImageIndex = 0;
    if (vkAcquireNextImageKHR(LogicalDevice, Swapchain, UINT64_MAX, ImageAvailableSemaphores[CurrentImage], VK_NULL_HANDLE, &ImageIndex) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to acquire next image from swapchain!");
    }
    return Images[ImageIndex];
}