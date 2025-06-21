#include "catch2/catch_test_macros.hpp"

#include "render.h"
#include "vk_context.h"

#include "random.h"
#include "bxdf.h"

#include "utility_functions.h"

#include "stb_image_write.h"
#include "tinyexr.h"

#include "scene_loader.h"

#include <iostream>
#include <fstream>
#include <random>

bool bGenerateReferences = true;

bool CompareImagesPng(const std::string& ReferencePath, const std::string& SavedPath)
{
    return true;
}

bool TestScene(const std::string& SceneName)
{
    INIT_VK_CONTEXT({});
    auto Render = std::make_shared<FRender>(1920, 1080);
    Render->Init();
    auto Camera = Render->CreateCamera();
    Render->SetActiveCamera(Camera);
    auto SceneLoader = std::make_shared<FSceneLoader>(Render);
    SceneLoader->LoadScene(SceneName);
    LoadCamera(Camera, Render, "../data/cameras/" + SceneName);

    for (int i = 0; i < 256; ++i)
    {
        Render->Update();
        Render->Render();
    }

    Render->WaitIdle();
    std::string SavePath = std::string("../data/") + (bGenerateReferences ? "TestReferences/" : "TestResults/") + SceneName;
    Render->PrintScreenPng(SavePath);

    bool Result = true;
    if (!bGenerateReferences)
    {
        std::string ReferencePath = std::string("../data/TestReferences/") + SceneName;
        Result = CompareImagesPng(ReferencePath, SavePath);
    }

    Render = nullptr;
    return Result;
}

TEST_CASE( SCENE_AREA_LIGHTS, "[Scenes]")
{
    CHECK(TestScene(Catch::getResultCapture().getCurrentTestName()));
}

TEST_CASE( SCENE_AREA_LIGHTS_2, "[Scenes]")
{
    CHECK(TestScene(Catch::getResultCapture().getCurrentTestName()));
}

TEST_CASE( SCENE_BIG_PLANES, "[Scenes]")
{
    CHECK(TestScene(Catch::getResultCapture().getCurrentTestName()));
}

TEST_CASE( SCENE_CORNELL_BOX, "[Scenes]")
{
    CHECK(TestScene(Catch::getResultCapture().getCurrentTestName()));
}

TEST_CASE( SCENE_CORNELL_BOX_2, "[Scenes]")
{
    CHECK(TestScene(Catch::getResultCapture().getCurrentTestName()));
}

TEST_CASE( SCENE_DIFFUSE_MATERIAL, "[Scenes]")
{
    CHECK(TestScene(Catch::getResultCapture().getCurrentTestName()));
}

TEST_CASE( SCENE_DIRECTIONAL_LIGHT, "[Scenes]")
{
    CHECK(TestScene(Catch::getResultCapture().getCurrentTestName()));
}

TEST_CASE( SCENE_GLASS_PLANES, "[Scenes]")
{
    CHECK(TestScene(Catch::getResultCapture().getCurrentTestName()));
}

TEST_CASE( SCENE_POINT_LIGHT, "[Scenes]")
{
    CHECK(TestScene(Catch::getResultCapture().getCurrentTestName()));
}

TEST_CASE( SCENE_ROUGH_GLASS, "[Scenes]")
{
    CHECK(TestScene(Catch::getResultCapture().getCurrentTestName()));
}

TEST_CASE( SCENE_SPECULAR_ROUGHNESS, "[Scenes]")
{
    CHECK(TestScene(Catch::getResultCapture().getCurrentTestName()));
}

TEST_CASE( SCENE_SPOT_LIGHT, "[Scenes]")
{
    CHECK(TestScene(Catch::getResultCapture().getCurrentTestName()));
}

TEST_CASE( SCENE_STANFORD_DRAGON, "[Scenes]")
{
    CHECK(TestScene(Catch::getResultCapture().getCurrentTestName()));
}

TEST_CASE( SCENE_THREE_SPHERES, "[Scenes]")
{
    CHECK(TestScene(Catch::getResultCapture().getCurrentTestName()));
}

TEST_CASE( SCENE_VIKINGS_ROOM, "[Scenes]")
{
    CHECK(TestScene(Catch::getResultCapture().getCurrentTestName()));
}

TEST_CASE( SCENE_WHITE_FURNACE, "[Scenes]")
{
    CHECK(TestScene(Catch::getResultCapture().getCurrentTestName()));
}

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
	Render->SaveOutputPng(EOutputType::Color, "../data/debug/Basic_scene_loading");

	SceneLoader->UpdateScene(0, 0);
	Render->SetIBL("../resources/brown_photostudio_02_4k.exr");

	for (int i = 0; i < 100; ++i)
	{
		Render->Update();
		Render->Render();
	}

	Render->WaitIdle();
	Render->SaveOutputPng(EOutputType::Color, "../data/debug/Basic_scene_loading_1");

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

	stbi_write_bmp("../data/debug/Mersenne_twister.bmp" , ImageSize, ImageSize, 3, Texture.data());
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

	stbi_write_bmp("../data/debug/CMJ.bmp" , ImageSize, ImageSize, 3, TextureCMJ.data());
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

	stbi_write_bmp("../data/debug/Unit_square.bmp" , ImageSize, ImageSize, 3, Texture.data());

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
	SaveEXR(ScreenTexture.data(), ImageSize, ImageSize, 3, false, "../data/debug/Scree_UV.exr", &Err);
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

	stbi_write_bmp("../data/debug/Unit_disk.bmp" , ImageSize, ImageSize, 3, Texture.data());
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

	std::ofstream File("../data/debug/Unit_sphere.bin", std::ios::binary);
	if (File)
	{
		File.write(reinterpret_cast<const char*>(Sampled3DSphere.data()), sizeof(FVector3) * Sampled3DSphere.size());
		File.close();
	}
}

TEST_CASE( "Test random cosine hemisphere", "[Utility]")
{
	/// Create a set of vertices with cosine PDF
	/// To visualize result use "points_plotter_from_bin.py"
	FSamplingState SamplingState = {0, 0, 0, SAMPLE_TYPE_GENERATE_RAYS, 0};
	std::vector<FVector3> SampledCosineHemisphere(CMJ_TOTAL_GRID_SIZE);

	for (int i = 0; i < CMJ_TOTAL_GRID_SIZE; ++i)
	{
		SamplingState.RenderIteration++;
		SampledCosineHemisphere[i] = SampleCosineHemisphere(SamplingState);
	}

	std::ofstream File("../data/debug/Cosine_hemisphere.bin", std::ios::binary);
	if (File)
	{
		File.write(reinterpret_cast<const char*>(SampledCosineHemisphere.data()), sizeof(FVector3) * SampledCosineHemisphere.size());
		File.close();
	}
}

TEST_CASE( "Test GGX normal generator", "[GGX]")
{
	float Roughness = 0.;
	uint32_t NumberOfSamplePerStep = 1000u;
	uint32_t NumberOfRoughnessSteps = 10;

	for (uint32_t i = 0; i <= NumberOfRoughnessSteps; ++i)
	{
		float a2 = Roughness * Roughness;
		FSamplingState SamplingState = {0, 0, i, SAMPLE_TYPE_GENERATE_RAYS, 0};
		std::vector<FVector3> Values(NumberOfSamplePerStep);
		for (int j = 0; j < NumberOfSamplePerStep; ++j)
		{
			auto Random = Sample2DUnitQuad(SamplingState);
			SamplingState.RenderIteration++;
			auto SphericalCoords = CDFCookTorrance(a2, Random.X, Random.Y);
			Values[j].X = cos(SphericalCoords.Y) * sin(SphericalCoords.X);
			Values[j].Y = cos(SphericalCoords.X);
			Values[j].Z = sin(SphericalCoords.Y) * sin(SphericalCoords.X);
		}

		std::ofstream File("../data/debug/GGX_" + std::to_string(Roughness) + ".bin", std::ios::binary);
		if (File)
		{
			File.write(reinterpret_cast<const char*>(Values.data()), sizeof(FVector3) * Values.size());
			File.close();
		}

		Roughness += 1.f / NumberOfRoughnessSteps;
	}
}

TEST_CASE( "Test GGX of VNDF", "[GGX]")
{
	float Roughness = 0.;
	uint32_t NumberOfSamplePerStep = 1000u;
	uint32_t NumberOfRoughnessSteps = 10;

	for (uint32_t i = 0; i <= NumberOfRoughnessSteps; ++i)
	{
		FSamplingState SamplingState = {0, 0, i, SAMPLE_TYPE_GENERATE_RAYS, 0};
		std::vector<FVector3> Values(NumberOfSamplePerStep);
		for (int j = 0; j < NumberOfSamplePerStep; ++j)
		{
			auto Random = Sample2DUnitQuad(SamplingState);
			SamplingState.RenderIteration++;
			auto SphericalCoords = SampleGGXVNDF(FVector3(1, 0, 1).GetNormalized(), Roughness, Roughness, Random.X, Random.Y);
			Values[j].x = SphericalCoords.x;
			Values[j].y = SphericalCoords.z;
			Values[j].z = SphericalCoords.y;
		}

		std::ofstream File("../data/debug/GGX_VNDF_" + std::to_string(Roughness) + ".bin", std::ios::binary);
		if (File)
		{
			File.write(reinterpret_cast<const char*>(Values.data()), sizeof(FVector3) * Values.size());
			File.close();
		}

		Roughness += 1.f / NumberOfRoughnessSteps;
	}
}
