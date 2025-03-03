#pragma once

#include <chrono>

#include "application.h"
#include "vk_context.h"
#include "window_manager.h"
#include "controller.h"
#include "utility_functions.h"

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
	ImguiTask = std::make_shared<FImguiTask>(Width, Height, 1, int(Swapchain->Size()), VK_CONTEXT()->LogicalDevice);
	ImguiTask->SetGLFWWindow(WindowManager->GetWindow());
	ImguiTask->SetRender(Render);
	ImguiTask->Init(Swapchain->GetImages());
	ImguiTask->UpdateDescriptorSets();
	ImguiTask->RecordCommands();
	Render->AddExternalTaskAfterRender(ImguiTask);
	SceneLoader = std::make_shared<FSceneLoader>(Render);
}

FApplication::~FApplication()
{
	Render->WaitIdle();
	Swapchain = nullptr;
	ImguiTask = nullptr;
	Render = nullptr;
    WindowManager = nullptr;
}

int FApplication::Run()
{
	uint32_t ImageIndex = UINT32_MAX;
	LoadCamera(Controller->Camera, Render, "../data/cameras/Test");
	SceneLoader->LoadScene(SCENE_DIFFUSE_MATERIAL);

	FSynchronizationPoint RenderingFinished;

    while (!WindowManager->ShouldClose())
    {
		if (bSwapchainWasResized)
		{
			Render->SetSize(Width, Height);
			Swapchain = nullptr;
			ImguiTask = nullptr;
			Swapchain = std::make_shared<FSwapchain>(Width, Height, VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_PRESENT_MODE_MAILBOX_KHR);
			Render->RegisterExternalOutputs(Swapchain->GetImages(), Swapchain->GetSemaphores());
			ImguiTask = std::make_shared<FImguiTask>(Width, Height, 1, int(Swapchain->Size()), VK_CONTEXT()->LogicalDevice);
			ImguiTask->SetGLFWWindow(WindowManager->GetWindow());
			ImguiTask->SetRender(Render);
			ImguiTask->Init(Swapchain->GetImages());
			ImguiTask->UpdateDescriptorSets();
			ImguiTask->RecordCommands();
			Render->AddExternalTaskAfterRender(ImguiTask);
			bSwapchainWasResized = false;
		}

        static auto StartTime = std::chrono::high_resolution_clock::now();

        auto CurrentTime = std::chrono::high_resolution_clock::now();
        float DeltaTime = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - StartTime).count();
		float Time = std::chrono::duration<float>(CurrentTime.time_since_epoch()).count();
        StartTime = CurrentTime;

		SceneLoader->UpdateScene(DeltaTime, Time);
		Controller->Update(DeltaTime);
		Render->Update();
		Render->Wait(RenderingFinished);
		auto ImageReadySemaphore = Swapchain->GetNextImage( ImageIndex);
		RenderingFinished = Render->Render(ImageIndex);
		Swapchain->Present(RenderingFinished, ImageIndex);
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