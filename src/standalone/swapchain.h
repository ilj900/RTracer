#pragma once

#include "vulkan/vulkan.h"
#include "image.h"
#include "entity.h"

#include "vk_utils.h"

#include <memory>
#include <vector>

struct FSwapchain
{
	FSwapchain(uint32_t Width, uint32_t Height, VkFormat Format, VkColorSpaceKHR ColorSpace, VkPresentModeKHR PresentMode);
	~FSwapchain();
	void Present(FSynchronizationPoint Wait, uint32_t ImageIndex);
	size_t Size();
	FSynchronizationPoint GetNextImage(uint32_t& ImageIndex);
	VkFormat GetImageFormat();
	uint32_t GetWidth();
	uint32_t GetHeight();
	VkExtent2D GetExtent2D();
	std::vector<ImagePtr> GetImages();
	std::vector<FSynchronizationPoint> GetSemaphores();
	VkSwapchainKHR GetSwapchain();
	uint32_t Counter = 0;


	VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
	VkSurfaceFormatKHR SurfaceFormat;
	VkPresentModeKHR PresentMode;
	VkExtent2D Extent;
	std::vector<ImagePtr> Images;
	std::vector<FSynchronizationPoint> ImageAvailable;
};
