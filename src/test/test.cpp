#include "test.h"

#include "catch2/catch_test_macros.hpp"

#include "render.h"
#include "vk_context.h"

TEST_CASE( "Basic scene loading", "[Basic]" )
{
	INIT_VK_CONTEXT({});
	auto Render = std::make_shared<FRender>(1920, 1080);
	Render->Init();
	auto Camera = Render->CreateCamera();
	Render->SetActiveCamera(Camera);

	auto Wall = Render->CreatePlane();
	auto BackWall = Render->CreateInstance(Wall, {0, 0, 0});
	auto WhiteMaterial = Render->CreateMaterial({1, 1, 1});
	Render->ShapeSetMaterial(BackWall, WhiteMaterial);

	auto Light = Render->CreateLight({0, 0, 5});

	for (int i = 0; i < 10; ++i)
	{
		Render->Update();
		Render->Render();
	}

	Render->WaitIdle();
	Render->SaveOutput(OutputType(0), "Basic scene loading");

	Render = nullptr;
}

//TEST_CASE( "Basic Empty scene", "[Basic]" )
//{
//	INIT_VK_CONTEXT({});
//	auto Render = std::make_shared<FRender>(1920, 1080);
//	Render->Init();
//	auto Camera = Render->CreateCamera();
//	Render->SetActiveCamera(Camera);
//
//	auto BackWall = Render->CreatePlane();
//
//	for (int i = 0; i < 10; ++i)
//	{
//		Render->Update();
//		Render->Render();
//	}
//
//	Render->WaitIdle();
//	Render->SaveOutput(OutputType(0), "Basic scene loading");
//
//	Render = nullptr;
//}
