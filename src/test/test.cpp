#include "catch2/catch_test_macros.hpp"

#include "render.h"
#include "vk_context.h"

#include "scene_loader.h"

TEST_CASE( "Basic scene loading", "[Basic]" )
{
	INIT_VK_CONTEXT({});
	auto Render = std::make_shared<FRender>(1920, 1080);
	Render->Init();
	auto Camera = Render->CreateCamera();
	Render->SetActiveCamera(Camera);
	auto SceneLoader = std::make_shared<FSceneLoader>(Render);
	SceneLoader->LoadScene("Cornell Box");

	for (int i = 0; i < 100; ++i)
	{
		Render->Update();
		Render->Render();
	}

	Render->WaitIdle();
	Render->SaveOutput(EOutputType::Color, "Basic scene loading");

	SceneLoader->UpdateScene(0, 0);
	Render->SetIBL("../../../resources/brown_photostudio_02_4k.exr");

	for (int i = 0; i < 100; ++i)
	{
		Render->Update();
		Render->Render();
	}

	Render->WaitIdle();
	Render->SaveOutput(EOutputType::Color, "Basic scene loading_1");

	Render = nullptr;
}
