#pragma once

#include "vulkan/vulkan.h"
#include "image.h"
#include "entity.h"

#include <memory>
#include <vector>

struct FSwapchain
{
	FSwapchain(uint32_t Width, uint32_t Height, VkFormat Format, VkColorSpaceKHR ColorSpace, VkPresentModeKHR PresentMode);
	~FSwapchain();
	void Present(VkSemaphore WaitSemaphore, uint32_t ImageIndex);
	size_t Size();
	VkSemaphore GetNextImage(uint32_t& ImageIndex);
	VkFormat GetImageFormat();
	uint32_t GetWidth();
	uint32_t GetHeight();
	VkExtent2D GetExtent2D();
	std::vector<ImagePtr> GetImages();
	std::vector<VkSemaphore> GetSemaphores();
	VkSwapchainKHR GetSwapchain();


	VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
	VkSurfaceFormatKHR SurfaceFormat;
	VkPresentModeKHR PresentMode;
	VkExtent2D Extent;
	std::vector<ImagePtr> Images;
	std::vector<VkSemaphore> ImageAvailableSemaphores;
};
