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

	Render->Update();
	Render->Render();
	Render->WaitIdle();
	Render->SaveOutput(OutputType(0), "Basic scene loading");

	Render = nullptr;
}
