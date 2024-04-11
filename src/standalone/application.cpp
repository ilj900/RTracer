#pragma once

#include <chrono>

#include "application.h"
#include "vk_context.h"
#include "window_manager.h"
#include "controller.h"

FApplication::FApplication()
{
    WindowManager = std::make_shared<FWindowManager>(Width, Height, false, this, "RTRacer");
	INIT_VK_CONTEXT(WindowManager->GetWindow());
    INIT_RENDER(Width, Height);
	Swapchain = std::make_shared<FSwapchain>(Width, Height, VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_PRESENT_MODE_MAILBOX_KHR);
	RENDER()->RegisterExternalOutputs(Swapchain->GetImages(), Swapchain->GetSemaphores());
	RENDER()->Init();
    CONTROLLER()->SetWindow(WindowManager->GetWindow());
}

FApplication::~FApplication()
{
    FREE_RENDER();
    WindowManager = nullptr;
}

int FApplication::Run()
{
	uint32_t ImageIndex = UINT32_MAX;
	VkSemaphore RenderingFinishedSemaphore = VK_NULL_HANDLE;

    while (!WindowManager->ShouldClose())
    {
		if (bSwapchainWasResized)
		{
			Swapchain = nullptr;
			Swapchain = std::make_shared<FSwapchain>(Width, Height, VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_PRESENT_MODE_MAILBOX_KHR);
			RENDER()->RegisterExternalOutputs(Swapchain->GetImages(), Swapchain->GetSemaphores());
			bSwapchainWasResized = false;
		}

        static auto StartTime = std::chrono::high_resolution_clock::now();

        auto CurrentTime = std::chrono::high_resolution_clock::now();
        float Time = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - StartTime).count();
        StartTime = CurrentTime;

        CONTROLLER()->Update(Time);
        RENDER()->Update();
		Swapchain->GetNextImage( ImageIndex);
        RENDER()->Render(ImageIndex, &RenderingFinishedSemaphore);
		Swapchain->Present(RenderingFinishedSemaphore, ImageIndex);

		glfwPollEvents();
    }

	Swapchain = nullptr;

    return 0;
}

void FApplication::SetSwapchainWasResized(uint32_t NewWidth, uint32_t NewHeight)
{
	bSwapchainWasResized = true;
	Width = NewWidth;
	Height = NewHeight;
}