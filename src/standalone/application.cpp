#pragma once

#include <chrono>

#include "application.h"
#include "vk_context.h"
#include "window_manager.h"
#include "controller.h"

FApplication::FApplication()
{
    INIT_WINDOW_MANAGER(Width, Height, false, this, "RTRacer");
	INIT_VK_CONTEXT(WINDOW());
    INIT_RENDER(Width, Height);
	Swapchain = std::make_shared<FSwapchain>(Width, Height, VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_PRESENT_MODE_MAILBOX_KHR);
	RENDER()->RegisterExternalOutputs(Swapchain->GetImages(), Swapchain->GetSemaphores());
	RENDER()->Init();
    CONTROLLER()->SetWindow(WINDOW());
}

FApplication::~FApplication()
{
    FREE_RENDER();
    WINDOW_MANAGER()->Destroy();
}

int FApplication::Run()
{
    int i = 0;
	uint32_t ImageIndex = UINT32_MAX;
	VkSemaphore RenderingFinishedSemaphore = VK_NULL_HANDLE;
	VkSemaphore ImageReadySemaphore = VK_NULL_HANDLE;

    while (!WINDOW_MANAGER()->ShouldClose())
    {
		if (bShouldRecreateSwaochain)
		{
			Swapchain = nullptr;
			Swapchain = std::make_shared<FSwapchain>(Width, Height, VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_PRESENT_MODE_MAILBOX_KHR);
			RENDER()->RegisterExternalOutputs(Swapchain->GetImages(), Swapchain->GetSemaphores());
			RENDER()->Init();
		}

        static auto StartTime = std::chrono::high_resolution_clock::now();

        auto CurrentTime = std::chrono::high_resolution_clock::now();
        float Time = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - StartTime).count();
        StartTime = CurrentTime;

        CONTROLLER()->Update(Time);
        RENDER()->Update();
		ImageReadySemaphore = Swapchain->GetNextImage( ImageIndex);
        RENDER()->Render(ImageIndex, &RenderingFinishedSemaphore);
		Swapchain->Present(RenderingFinishedSemaphore, ImageIndex);

		glfwPollEvents();
    }

	Swapchain = nullptr;

    return 0;
}

void FApplication::SetShouldRecreateSwapchain(uint32_t NewWidth, uint32_t NewHeight)
{
	bShouldRecreateSwaochain = true;
	Width = NewWidth;
	Height = NewHeight;
}