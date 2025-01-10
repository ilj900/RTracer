#include "catch2/catch_test_macros.hpp"

#include "render.h"
#include "vk_context.h"

#include "random.h"

#include "stb_image_write.h"
#include "tinyexr.h"

#include "scene_loader.h"

#include <fstream>
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
	Render->SaveOutputPng(EOutputType::Color, "Basic scene loading");

	SceneLoader->UpdateScene(0, 0);
	Render->SetIBL("../../../resources/brown_photostudio_02_4k.exr");

	for (int i = 0; i < 100; ++i)
	{
		Render->Update();
		Render->Render();
	}

	Render->WaitIdle();
	Render->SaveOutputPng(EOutputType::Color, "Basic scene loading_1");

	Render = nullptr;
}

TEST_CASE( "Test random Mersenne twister", "[Utility]")
{
	/// Generate some random values with default mersenne twister
	const int ImageSize = 512;
	std::vector<char> Texture(ImageSize * ImageSize * 3, char(0));
	std::random_device Device;
	std::mt19937 RNG(Device());
	std::uniform_real_distribution<float> Distribution(0., 1.);

	for (int i = 0; i < CMJ_TOTAL_GRID_SIZE; ++i)
	{
		FVector2 UV = {Distribution(RNG), Distribution(RNG)};

		int X = UV.X * ImageSize;
		int Y = UV.Y * ImageSize;
		int PixelIndex = Y * ImageSize + X;
		Texture[PixelIndex * 3] = char(255);
		Texture[PixelIndex * 3 + 1] = char(255);
		Texture[PixelIndex * 3 + 2] = char(255);
	}

	stbi_write_bmp("Mersenne_twister.bmp" , ImageSize, ImageSize, 3, Texture.data());
}

TEST_CASE( "Test random CMJ", "[Utility]")
{
	const int ImageSize = 512;
	/// Generate some random values to check how CMJ works
	std::vector<char> TextureCMJ(ImageSize * ImageSize * 3, char(0));

	for (int i = 0; i < CMJ_TOTAL_GRID_SIZE; ++i)
	{
		FVector2 UV = CMJ(i % CMJ_TOTAL_GRID_SIZE, CMJ_GRID_LINEAR_SIZE, CMJ_GRID_LINEAR_SIZE, i);

		int X = UV.X * ImageSize;
		int Y = UV.Y * ImageSize;
		int PixelIndex = Y * ImageSize + X;
		TextureCMJ[PixelIndex * 3] = char(255);
		TextureCMJ[PixelIndex * 3 + 1] = char(255);
		TextureCMJ[PixelIndex * 3 + 2] = char(255);
	}

	stbi_write_bmp("CMJ.bmp" , ImageSize, ImageSize, 3, TextureCMJ.data());
}

TEST_CASE( "Test random unit square", "[Utility]")
{
	const int ImageSize = 512;
	FSamplingState SamplingState = {0, 0, 0, SAMPLE_TYPE_GENERATE_RAYS, 0};
	std::vector<char> Texture(ImageSize * ImageSize * 3, char(0));

	/// Simulate CMJ_TOTAL_GRID_SIZE calls to generate random point in unit square in one pixel
	for (int i = 0; i < CMJ_TOTAL_GRID_SIZE; ++i)
	{
		FVector2 UV = Sample2DUnitQuad(SamplingState);

		int X = UV.X * ImageSize;
		int Y = UV.Y * ImageSize;
		int PixelIndex = Y * ImageSize + X;
		Texture[PixelIndex * 3] = char(255);
		Texture[PixelIndex * 3 + 1] = char(255);
		Texture[PixelIndex * 3 + 2] = char(255);

		SamplingState.RenderIteration++;
	}

	stbi_write_bmp("Unit_square.bmp" , ImageSize, ImageSize, 3, Texture.data());

	std::vector<float> ScreenTexture(ImageSize * ImageSize * 3, 0);
	SamplingState = {0, 0, 0, SAMPLE_TYPE_GENERATE_RAYS, 0};

	/// Simulate ImageSize * ImageSize calls to generate random points in unit square on a fullscreen image
	for (int i = 0; i < ImageSize * ImageSize; ++i)
	{
		FVector2 UV = Sample2DUnitQuad(SamplingState);
		ScreenTexture[i * 3] = UV.X;
		ScreenTexture[i * 3 + 1] = UV.Y;

		SamplingState.PixelIndex++;
	}

	const char* Err = NULL;
	SaveEXR(ScreenTexture.data(), ImageSize, ImageSize, 3, false, "Scree_UV.exr", &Err);
}

TEST_CASE( "Test random unit disk", "[Utility]")
{
	const int ImageSize = 512;
	FSamplingState SamplingState = {0, 0, 0, SAMPLE_TYPE_GENERATE_RAYS, 0};
	std::vector<char> Texture(ImageSize * ImageSize * 3, char(0));

	for (int i = 0; i < CMJ_TOTAL_GRID_SIZE; ++i)
	{
		SamplingState.RenderIteration++;
		FVector2 UV = Sample2DUnitDisk(SamplingState);

		int X = UV.X * ImageSize;
		int Y = UV.Y * ImageSize;
		int PixelIndex = Y * ImageSize + X;
		Texture[PixelIndex * 3] = char(255);
		Texture[PixelIndex * 3 + 1] = char(255);
		Texture[PixelIndex * 3 + 2] = char(255);
	}

	stbi_write_bmp("Unit_disk.bmp" , ImageSize, ImageSize, 3, Texture.data());
}

TEST_CASE( "Test random unit sphere", "[Utility]")
{
	/// Create a set of vertices uniformly placed on a 3D unit sphere
	/// To visualize result use "points_plotter_from_bin.py"
	FSamplingState SamplingState = {0, 0, 0, SAMPLE_TYPE_GENERATE_RAYS, 0};
	std::vector<FVector3> Sampled3DSphere(CMJ_TOTAL_GRID_SIZE);

	for (int i = 0; i < CMJ_TOTAL_GRID_SIZE; ++i)
	{
		SamplingState.RenderIteration++;
		Sampled3DSphere[i] = Sample3DUnitSphere(SamplingState);
	}

	std::ofstream File("Unit_sphere.bin", std::ios::binary);
	if (File)
	{
		File.write(reinterpret_cast<const char*>(Sampled3DSphere.data()), sizeof(FVector3) * Sampled3DSphere.size());
		File.close();
	}
}
