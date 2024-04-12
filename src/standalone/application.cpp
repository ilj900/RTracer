#pragma once

#include <chrono>

#include "application.h"
#include "vk_context.h"
#include "window_manager.h"
#include "controller.h"

FApplication::FApplication()
{
    WindowManager = std::make_shared<FWindowManager>(Width, Height, false, this, "RTRacer");
	FVulkanContext::SetSurfaceCreationFunction(WindowManager->CreateSurfaceFunctor);
	auto RequiredExtensions = WindowManager->GetRequiredDeviceExtensions();
	INIT_VK_CONTEXT(RequiredExtensions);

    Render = std::make_shared<FRender>(Width, Height);
	Swapchain = std::make_shared<FSwapchain>(Width, Height, VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_PRESENT_MODE_MAILBOX_KHR);

	Render->RegisterExternalOutputs(Swapchain->GetImages(), Swapchain->GetSemaphores());
	Render->Init();
	Controller = std::make_shared<FController>(Render);
	Controller->SetWindow(WindowManager->GetWindow());
	WindowManager->SetController(Controller.get());
}

FApplication::~FApplication()
{
	Swapchain = nullptr;
	Render = nullptr;
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
			Render->SetSize(Width, Height);
			Swapchain = nullptr;
			Swapchain = std::make_shared<FSwapchain>(Width, Height, VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_PRESENT_MODE_MAILBOX_KHR);
			Render->RegisterExternalOutputs(Swapchain->GetImages(), Swapchain->GetSemaphores());
			bSwapchainWasResized = false;
		}

        static auto StartTime = std::chrono::high_resolution_clock::now();

        auto CurrentTime = std::chrono::high_resolution_clock::now();
        float Time = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - StartTime).count();
        StartTime = CurrentTime;

		Controller->Update(Time);
		Render->Update();
		Swapchain->GetNextImage( ImageIndex);
		Render->Render(ImageIndex, &RenderingFinishedSemaphore);

		static int i = 0;
		Render->SaveOutput(OutputType(ImageIndex), "Test" + std::to_string(i++));

		Swapchain->Present(RenderingFinishedSemaphore, ImageIndex);

		WindowManager->PollEvents();
    }

    return 0;
}

void FApplication::SetSwapchainWasResized(uint32_t NewWidth, uint32_t NewHeight)
{
	bSwapchainWasResized = true;
	Width = NewWidth;
	Height = NewHeight;
}