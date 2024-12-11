#include "catch2/catch_test_macros.hpp"

#include "render.h"
#include "vk_context.h"

#include "random.h"

#include "stb_image_write.h"

#include "scene_loader.h"

#include <random>

TEST_CASE( "Basic scene loading", "[Basic]" )
{
	INIT_VK_CONTEXT({});
	auto Render = std::make_shared<FRender>(1920, 1080);
	Render->Init();
	auto Camera = Render->CreateCamera();
	Render->SetActiveCamera(Camera);
	auto SceneLoader = std::make_shared<FSceneLoader>(Render);
	SceneLoader->LoadScene(SCENE_CORNELL_BOX);

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

TEST_CASE( "Test Random", "[Utility]" )
{
	const int ImageSize = 512;
	std::vector<char> Texture(ImageSize * ImageSize * 3);
	FSamplingState SamplingState = {0, 0, 0};

	for (int i = 0; i < 3000; ++i)
	{
		SamplingState.Seed = i;
		FVector2 UV = Sample2DUnitQuad(SamplingState);

		int X = UV.X * ImageSize;
		int Y = UV.Y * ImageSize;
		int PixelIndex = Y * ImageSize + X;
		if (Texture[PixelIndex * 3] == 255)
		{
			if (Texture[PixelIndex * 3 + 1] == 255)
			{
				Texture[PixelIndex * 3 + 2] = 255;
			}
			Texture[PixelIndex * 3 + 1] = 255;
		}
		Texture[PixelIndex * 3] = 255;
	}

	stbi_write_bmp("CMJ.bmp" , ImageSize, ImageSize, 3, Texture.data());

	std::vector<char> Texture2(ImageSize * ImageSize * 3);
	std::random_device Device;
	std::mt19937 RNG(Device());
	std::uniform_real_distribution<float> Distribution(0., 1.);

	for (int i = 0; i < 3000; ++i)
	{
		FVector2 UV = {Distribution(RNG), Distribution(RNG)};

		int X = UV.X * ImageSize;
		int Y = UV.Y * ImageSize;
		int PixelIndex = Y * ImageSize + X;
		if (Texture2[PixelIndex * 3] == 255)
		{
			if (Texture2[PixelIndex * 3 + 1] == 255)
			{
				Texture2[PixelIndex * 3 + 2] = 255;
			}
			Texture2[PixelIndex * 3 + 1] = 255;
		}
		Texture2[PixelIndex * 3] = 255;
	}

	stbi_write_bmp("Random.bmp" , ImageSize, ImageSize, 3, Texture2.data());
}
