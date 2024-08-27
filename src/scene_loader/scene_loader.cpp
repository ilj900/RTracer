#include "scene_loader.h"

FSceneLoader::FSceneLoader(std::shared_ptr<FRender> RenderIn) : Render(RenderIn)
{
}

FSceneLoader::~FSceneLoader()
{
	Render = nullptr;
}

void FSceneLoader::LoadScene(const std::string& Name)
{
	if (Name == SCENE_FLOATING_SPHERES)
	{
		Meshes.emplace_back(Render->CreateIcosahedronSphere(1, 5, false));
		Materials.emplace_back(Render->CreateMaterial({0, 1, 0}));

		for (int i = -20; i < 20; ++i)
		{
			for (int j = -20; j < 20; ++j)
			{
				Instances.emplace_back(Render->CreateInstance(Meshes.back(), {2.f * i, -5.f, 2.f * j}));
				Render->ShapeSetMaterial(Instances.back(), Materials.back());
			}
		}

		Lights.emplace_back(Render->CreateLight({5, 5, 5}));

		Render->SetIBL("../../../resources/brown_photostudio_02_4k.exr");

		Updater = [&](float DeltaTime, float CurrentTime)
		{
			for (auto& Instance : Instances)
			{
				auto NewInstanceCoordinates = Render->GetInstancePosition(Instance);
				float R = sqrt(NewInstanceCoordinates.X * NewInstanceCoordinates.X + NewInstanceCoordinates.Z * NewInstanceCoordinates.Z);
				NewInstanceCoordinates.Y = sin((R / 4 + CurrentTime)) * 2 - 5.f;
				Render->SetInstancePosition(Instance, NewInstanceCoordinates);
			}

			auto NewLightCoordinates = Render->GetLightPosition(Lights.back()).SelfRotateY(DeltaTime);
			Render->SetLightPosition(Lights.back(), NewLightCoordinates);
		};
	}
	else if (Name == SCENE_CORNELL_BOX)
	{
		auto Wall = Render->CreatePlane({4, 4});
		auto Sphere = Render->CreateIcosahedronSphere(0.5f, 5, false);

		auto BackWall = Render->CreateInstance(Wall, {0, 0, -2}, {0, 0, -1}, {0, 1, 0});
		auto TopWall = Render->CreateInstance(Wall, {0, 2, 0}, {0, -1, 0}, {0, 0, 1});
		auto BottomWall = Render->CreateInstance(Wall, {0, -2, 0}, {0, 1, 0}, {0, 0, -1});
		auto LeftWall = Render->CreateInstance(Wall, {-2, 0, 0}, {1, 0, 0}, {0, 1, 0});
		auto RightWall = Render->CreateInstance(Wall, {2, 0, 0}, {-1, 0, 0}, {0, 1, 0});
		auto Sphere1 = Render->CreateInstance(Sphere, {0, -1.5, 1});
		auto Sphere2 = Render->CreateInstance(Sphere, {1, -1.5, -1});
		auto Sphere3 = Render->CreateInstance(Sphere, {-1, -1.5, -1});

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

		//auto Light = Render->CreateLight({0, 1.95, 0});

		Render->SetIBL("../../../resources/brown_photostudio_02_4k.exr");
	}
	else if (Name == SCENE_CORNELL_BOX_ANIMATED)
	{
		auto Wall = Render->CreatePlane({4, 4});
		auto Sphere = Render->CreateIcosahedronSphere(0.5f, 5, false);
		auto Cube = Render->CreateCube();

		auto BackWall = Render->CreateInstance(Wall, {0, 0, -2}, {0, 0, -1}, {0, 1, 0});
		auto TopWall = Render->CreateInstance(Wall, {0, 2, 0}, {0, -1, 0}, {0, 0, 1});
		auto BottomWall = Render->CreateInstance(Wall, {0, -2, 0}, {0, 1, 0}, {0, 0, -1});
		auto LeftWall = Render->CreateInstance(Wall, {-2, 0, 0}, {1, 0, 0}, {0, 1, 0});
		auto RightWall = Render->CreateInstance(Wall, {2, 0, 0}, {-1, 0, 0}, {0, 1, 0});
		auto Sphere1 = Render->CreateInstance(Sphere, {0, -1.5, 1});
		auto Sphere2 = Render->CreateInstance(Sphere, {1, -1.5, -1});
		auto Sphere3 = Render->CreateInstance(Sphere, {-1, -1.5, -1});
		Instances.emplace_back(Render->CreateInstance(Cube, {0, 0, 0}));

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
		Render->ShapeSetMaterial(Instances.back(), PlasticMaterial);

		auto Light = Render->CreateLight({0, 1.95, 0});

		Render->SetIBL("../../../resources/brown_photostudio_02_4k.exr");

		Updater = [&](float, float CurrentTime)
		{
			auto NewInstanceCoordinates = Render->GetInstancePosition(Instances.back());
			float R = sqrt(NewInstanceCoordinates.X * NewInstanceCoordinates.X + NewInstanceCoordinates.Z * NewInstanceCoordinates.Z);
			NewInstanceCoordinates.Y = sin((R / 4 + CurrentTime)) * 2;
			Render->SetInstancePosition(Instances.back(), NewInstanceCoordinates);
		};

	}
	else if (Name == SCENE_THREE_SPHERES)
	{
		auto Sphere = Render->CreateIcosahedronSphere(1.5f, 5, false);
		auto Sphere1 = Render->CreateInstance(Sphere, {0, -1.5, 1});
		auto Sphere2 = Render->CreateInstance(Sphere, {1, -1.5, -1});
		auto Sphere3 = Render->CreateInstance(Sphere, {-1, -1.5, -1});

		auto GlassMaterial = Render->CreateMaterial({0, 1, 1});
		auto DiffuseMaterial = Render->CreateMaterial({1, 1, 0});
		auto PlasticMaterial = Render->CreateMaterial({1, 0, 1});

		Render->ShapeSetMaterial(Sphere1, GlassMaterial);
		Render->ShapeSetMaterial(Sphere2, DiffuseMaterial);
		Render->ShapeSetMaterial(Sphere3, PlasticMaterial);

		Render->SetIBL("../../../resources/hdr_image.exr");
	}
	else if (Name == SCENE_BIG_PLANES)
	{
		auto Wall = Render->CreatePlane({10, 10});
		auto BackWall1 = Render->CreateInstance(Wall, {0, 0, 0}, {0, 0, -1}, {0, 1, 0});
		auto BackWall = Render->CreateInstance(Wall, {0, 0, 1}, {0, 0, -1}, {0, 1, 0});

		auto RedMaterial = Render->CreateMaterial({1, 0, 0});
		auto GreenMaterial = Render->CreateMaterial({0, 1, 0});

		Render->ShapeSetMaterial(BackWall1, RedMaterial);
		Render->ShapeSetMaterial(BackWall, GreenMaterial);

		Render->SetIBL("../../../resources/hdr_image.exr");
	}
	else if (Name == SCENE_COORDINATE_SYSTEM_REMINDER)
	{
		auto Sphere = Render->CreateUVSphere(32, 16);
		auto SphereX = Render->CreateInstance(Sphere, {3, 0, 0}, {1, 0, 0}, {0, 1, 0});
		auto SphereY = Render->CreateInstance(Sphere, {0, 3, 0}, {1, 0, 0}, {0, 1, 0});
		auto SphereZ = Render->CreateInstance(Sphere, {0, 0, 3}, {1, 0, 0}, {0, 1, 0});
		auto SphereC = Render->CreateInstance(Sphere, {0, 0, 0}, {1, 0, 0}, {0, 1, 0});

		auto RedMaterial = Render->CreateMaterial({1, 0, 0});
		auto GreenMaterial = Render->CreateMaterial({0, 1, 0});
		auto BlueMaterial = Render->CreateMaterial({0, 0, 1});
		auto WhiteMaterial = Render->CreateMaterial({1, 1, 1});

		Render->ShapeSetMaterial(SphereX, RedMaterial);
		Render->ShapeSetMaterial(SphereY, GreenMaterial);
		Render->ShapeSetMaterial(SphereZ, BlueMaterial);
		Render->ShapeSetMaterial(SphereC, WhiteMaterial);

		Render->SetIBL("../../../resources/brown_photostudio_02_4k.exr");
	}
	else
	{
		throw std::runtime_error("Scene not found");
	}
}

void FSceneLoader::UpdateScene(float DeltaTime, float CurrentTime)
{
	if (Updater != nullptr)
	{
		Updater(DeltaTime, CurrentTime);
	}
}