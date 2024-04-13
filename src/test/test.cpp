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

	auto Wall = Render->CreatePlane({4, 4});
	auto Sphere = Render->CreateSphere(5);

	auto BackWall = Render->CreateInstance(Wall, {0, 0, -2}, {0, 0, -1}, {0, 1, 0});
	auto TopWall = Render->CreateInstance(Wall, {0, 2, 0}, {0, -1, 0}, {0, 0, 1});
	auto BottomWall = Render->CreateInstance(Wall, {0, -2, 0}, {0, 1, 0}, {0, 0, -1});
	auto LeftWall = Render->CreateInstance(Wall, {2, 0, 0}, {1, 0, 0}, {0, 1, 0});
	auto RightWall = Render->CreateInstance(Wall, {-2, 0, 0}, {-1, 0, 0}, {0, 1, 0});
	auto Sphere1 = Render->CreateInstance(Sphere, {0, -1, 1});
	auto Sphere2 = Render->CreateInstance(Sphere, {1, -1, -1});
	auto Sphere3 = Render->CreateInstance(Sphere, {-1, -1, -1});

	auto WhiteMaterial = Render->CreateMaterial({1, 1, 1});
	auto RedMaterial = Render->CreateMaterial({1, 0, 0});
	auto GreenMaterial = Render->CreateMaterial({0, 1, 0});
	auto GlassMaterial = Render->CreateMaterial({0, 1, 1});
	auto DiffuseMaterial = Render->CreateMaterial({1, 1, 0});
	auto PlasticMaterial = Render->CreateMaterial({1, 0, 1});

	Render->ShapeSetMaterial(BackWall, WhiteMaterial);
	Render->ShapeSetMaterial(TopWall, WhiteMaterial);
	Render->ShapeSetMaterial(BottomWall, WhiteMaterial);
	Render->ShapeSetMaterial(LeftWall, RedMaterial);
	Render->ShapeSetMaterial(RightWall, GreenMaterial);
	Render->ShapeSetMaterial(Sphere1, GlassMaterial);
	Render->ShapeSetMaterial(Sphere2, DiffuseMaterial);
	Render->ShapeSetMaterial(Sphere3, PlasticMaterial);

	auto Light = Render->CreateLight({0, 2, 0});

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
