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
	ImguiTask = std::make_shared<FImguiTask>(Width, Height, int(Swapchain->Size()), VK_CONTEXT()->LogicalDevice);
	ImguiTask->SetGLFWWindow(WindowManager->GetWindow());
	ImguiTask->Init(Swapchain->GetImages());
	ImguiTask->UpdateDescriptorSets();
	ImguiTask->RecordCommands();
	Render->AddExternalTaskAfterRender(ImguiTask);
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
	VkSemaphore RenderingFinishedSemaphore = VK_NULL_HANDLE;

    while (!WindowManager->ShouldClose())
    {
		if (bSwapchainWasResized)
		{
			Render->SetSize(Width, Height);
			Swapchain = nullptr;
			ImguiTask = nullptr;
			Swapchain = std::make_shared<FSwapchain>(Width, Height, VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, VK_PRESENT_MODE_MAILBOX_KHR);
			Render->RegisterExternalOutputs(Swapchain->GetImages(), Swapchain->GetSemaphores());
			ImguiTask = std::make_shared<FImguiTask>(Width, Height, int(Swapchain->Size()), VK_CONTEXT()->LogicalDevice);
			ImguiTask->SetGLFWWindow(WindowManager->GetWindow());
			ImguiTask->Init(Swapchain->GetImages());
			ImguiTask->UpdateDescriptorSets();
			ImguiTask->RecordCommands();
			Render->AddExternalTaskAfterRender(ImguiTask);
			bSwapchainWasResized = false;
		}

        static auto StartTime = std::chrono::high_resolution_clock::now();

        auto CurrentTime = std::chrono::high_resolution_clock::now();
        float Time = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - StartTime).count();
        StartTime = CurrentTime;

		Update();
		Controller->Update(Time);
		Render->Update();
		Swapchain->GetNextImage( ImageIndex);
		Render->Render(ImageIndex, &RenderingFinishedSemaphore);
		Swapchain->Present(RenderingFinishedSemaphore, ImageIndex);
		WindowManager->PollEvents();
    }

    return 0;
}

void FApplication::Update()
{
	auto NewLightCoordinates = Render->GetLightPosition(Lights.back()).SelfRotateY(0.025f);
	Render->SetLightPosition(Lights.back(), NewLightCoordinates);
	auto NewInstanceCoordinates = Render->GetInstancePosition(Models.back()).SelfRotateY(0.025f);
	Render->SetInstancePosition(Models.back(), NewInstanceCoordinates);
}

void FApplication::LoadScene()
{
	FTimer Timer("Loading scene time: ");
	auto Plane = Render->CreatePlane({1, 1});
	auto Pyramid = Render->CreatePyramid();
	auto VikingRoom = Render->CreateModel("../../../models/viking_room/viking_room.obj");
	auto Cube = Render->CreateCube();
	auto Sphere = Render->CreateIcosahedronSphere(1, 5, false);
	auto Shaderball = Render->CreateModel("../../../models/Shaderball.obj");
	auto UVSphere = Render->CreateUVSphere(32, 16);

	auto WoodMaterial = Render->CreateMaterial({1, 0, 1});
	auto YellowMaterial = Render->CreateMaterial({1, 1, 0});
	auto VikingRoomMaterial = Render->CreateMaterial({0, 1, 1});
	auto RedMaterial = Render->CreateMaterial({1, 0, 0});
	auto GreenMaterial = Render->CreateMaterial({0, 1, 0});
	auto BlueMaterial = Render->CreateMaterial({0, 0, 1});

	auto ModelTexture = Render->CreateTexture("../../../models/viking_room/viking_room.png");
	auto WoodAlbedoTexture = Render->CreateTexture("../../../resources/Wood/Wood_8K_Albedo.jpg");
	auto WoodAOTexture = Render->CreateTexture("../../../resources/Wood/Wood_8K_AO.jpg");
	auto WoodRoughnessTexture = Render->CreateTexture("../../../resources/Wood/Wood_8K_Roughness.jpg");
	auto WoodNormalTexture = Render->CreateTexture("../../../resources/Wood/Wood_8K_Normal.jpg");

	Render->MaterialSetBaseColor(WoodMaterial, WoodAlbedoTexture);
	Render->MaterialSetDiffuseRoughness(WoodMaterial, WoodRoughnessTexture);
	Render->MaterialSetNormal(WoodMaterial, WoodNormalTexture);
	Render->MaterialSetBaseColor(VikingRoomMaterial, ModelTexture);

	auto PlaneInstance = Render->CreateInstance(Plane, {-5.f, 0.f, -2.f});
	auto PyramidInstance = Render->CreateInstance(Pyramid, {-3.f, 0.f, -2.f});
	auto VikingRoomInstance = Render->CreateInstance(VikingRoom, {-1.f, 0.f, -2.f});
	auto CubeInstance = Render->CreateInstance(Cube, {1.f, 0.f, -2.f});
	auto ShaderballInstance = Render->CreateInstance(Shaderball, {3.f, -1.f, -2.f});
	auto UVSphereInstance = Render->CreateInstance(UVSphere, {5.f, 0.f, -2.f});
	Models.push_back(UVSphereInstance);

	for (int i = -10; i < 10; ++i)
	{
		for (int j = -10; j < 10; ++j)
		{
			auto SphereInstance = Render->CreateInstance(Sphere, {2.f * i, -5.f, 2.f * j});
			Render->ShapeSetMaterial(SphereInstance, GreenMaterial);
		}
	}


	Render->ShapeSetMaterial(PlaneInstance, WoodMaterial);
	Render->ShapeSetMaterial(PyramidInstance, YellowMaterial);
	Render->ShapeSetMaterial(VikingRoomInstance, VikingRoomMaterial);
	Render->ShapeSetMaterial(CubeInstance, RedMaterial);
	Render->ShapeSetMaterial(ShaderballInstance, BlueMaterial);
	Render->ShapeSetMaterial(UVSphereInstance, WoodMaterial);

	Lights.push_back(Render->CreateLight({5, 5, 5}));

	Render->SetIBL("../../../resources/brown_photostudio_02_4k.exr");
}

void FApplication::SetSwapchainWasResized(uint32_t NewWidth, uint32_t NewHeight)
{
	bSwapchainWasResized = true;
	Width = NewWidth;
	Height = NewHeight;
}