#include "GLFW/glfw3.h"

#include "swapchain.h"
#include "vk_context.h"
#include "render.h"
#include "vk_debug.h"

#include <stdexcept>

FSwapchain::FSwapchain(uint32_t Width, uint32_t Height, VkFormat Format, VkColorSpaceKHR ColorSpace, VkPresentModeKHR PresentMode)
{
	/// Find out supported formats and choose the one we need
	{
		uint32_t FormatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(VK_CONTEXT()->PhysicalDevice, VK_CONTEXT()->Surface, &FormatCount, nullptr);

		std::vector<VkSurfaceFormatKHR> SurfaceFormats;
		if (FormatCount != 0) {
			SurfaceFormats.resize(FormatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(VK_CONTEXT()->PhysicalDevice, VK_CONTEXT()->Surface, &FormatCount, SurfaceFormats.data());
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
		vkGetPhysicalDeviceSurfacePresentModesKHR(VK_CONTEXT()->PhysicalDevice, VK_CONTEXT()->Surface, &PresentModeCount, nullptr);

		std::vector<VkPresentModeKHR> PresentModes;
		if (PresentModeCount != 0) {
			PresentModes.resize(PresentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(VK_CONTEXT()->PhysicalDevice, VK_CONTEXT()->Surface, &PresentModeCount, PresentModes.data());
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
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VK_CONTEXT()->PhysicalDevice, VK_CONTEXT()->Surface, &SurfaceCapabilities);

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
	CreateInfo.surface = VK_CONTEXT()->Surface;

	CreateInfo.minImageCount = ImageCount;
	CreateInfo.imageFormat = SurfaceFormat.format;
	CreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
	CreateInfo.imageExtent = Extent;
	CreateInfo.imageArrayLayers = 1;
	CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	uint32_t QueueFamilyIndices[] = {VK_CONTEXT()->GetGraphicsQueueIndex(), VK_CONTEXT()->GetPresentIndex()};
	if (VK_CONTEXT()->GetGraphicsQueueIndex() != VK_CONTEXT()->GetPresentIndex())
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

	if (vkCreateSwapchainKHR(VK_CONTEXT()->LogicalDevice, &CreateInfo, nullptr, &Swapchain) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swap chain!");
	}

	V::SetName(VK_CONTEXT()->LogicalDevice, Swapchain, "");

	/// Queue SwapChain for it's images
	vkGetSwapchainImagesKHR(VK_CONTEXT()->LogicalDevice, Swapchain, &ImageCount, nullptr);
	std::vector<VkImage> SwapchainImages(ImageCount);
	vkGetSwapchainImagesKHR(VK_CONTEXT()->LogicalDevice, Swapchain, &ImageCount, SwapchainImages.data());
	Images.resize(ImageCount);

	/// We need to Wrap those images
	for (uint32_t i = 0; i < SwapchainImages.size(); ++i)
	{
		/// In FImage first, so that there would be an ImageView
		Images[i] = VK_CONTEXT()->Wrap(SwapchainImages[i], Width, Height, SurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, VK_CONTEXT()->LogicalDevice, "V_SwapchainImage");
		/// Also create some semaphores to track image availability
		ImageAvailableSemaphores.push_back(VK_CONTEXT()->CreateSemaphore());
		V::SetName(VK_CONTEXT()->LogicalDevice, ImageAvailableSemaphores[i], "Swapchain semaphore", i);
	}
}

FSwapchain::~FSwapchain()
{
	for (uint32_t i = 0; i < Images.size(); ++i)
	{
		vkDestroySemaphore(VK_CONTEXT()->LogicalDevice, ImageAvailableSemaphores[i], nullptr);
	}
	ImageAvailableSemaphores.clear();

	Images.clear();
	vkDestroySwapchainKHR(VK_CONTEXT()->LogicalDevice, Swapchain, nullptr);
}

void FSwapchain::Present(VkSemaphore WaitSemaphore, uint32_t ImageIndex)
{
	Images[ImageIndex]->Transition(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	VkSemaphore WaitSemaphores[] = {WaitSemaphore};

	VkPresentInfoKHR PresentInfo{};
	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pWaitSemaphores = WaitSemaphores;
	VkSwapchainKHR SwapChains[] = {Swapchain};
	PresentInfo.swapchainCount = 1;
	PresentInfo.pSwapchains = SwapChains;
	PresentInfo.pImageIndices = &ImageIndex;
	PresentInfo.pResults = nullptr;

	VkResult Result = vkQueuePresentKHR(VK_CONTEXT()->GetPresentQueue(), &PresentInfo);

	if (Result != VK_ERROR_OUT_OF_DATE_KHR && Result != VK_SUBOPTIMAL_KHR && Result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present swap chain image!");
	}
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

std::vector<VkSemaphore> FSwapchain::GetSemaphores()
{
	return ImageAvailableSemaphores;
}

VkSwapchainKHR FSwapchain::GetSwapchain()
{
	return Swapchain;
}

VkSemaphore FSwapchain::GetNextImage(uint32_t& ImageIndex)
{
	VkSemaphore ImageAcquiredSemaphore = ImageAvailableSemaphores[Counter++];
	Counter %= Images.size();
	VkResult Result = vkAcquireNextImageKHR(VK_CONTEXT()->LogicalDevice, Swapchain, UINT64_MAX, ImageAcquiredSemaphore, VK_NULL_HANDLE, &ImageIndex);

	/// Run some checks
	if (Result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		return VK_NULL_HANDLE;
	}
	if (Result != VK_SUCCESS && Result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

	return ImageAcquiredSemaphore;
}