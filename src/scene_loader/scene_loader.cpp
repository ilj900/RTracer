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
		Materials.emplace_back(Render->CreateDiffuseMaterial({0, 1, 0}));

		for (int i = -20; i < 20; ++i)
		{
			for (int j = -20; j < 20; ++j)
			{
				Instances.emplace_back(Render->CreateInstance(Meshes.back(), {2.f * i, -5.f, 2.f * j}));
				Render->ShapeSetMaterial(Instances.back(), Materials.back());
			}
		}

		Lights.emplace_back(Render->CreatePointLight({5, 5, 5}, {1, 1, 1}, 1));

		Render->SetIBL("../resources/brown_photostudio_02_4k.exr");

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
		auto Sphere = Render->CreateUVSphere(512, 256, 0.5f);

		auto BackWall = Render->CreateInstance(Wall, {0, 0, -2}, {0, 0, 1}, {0, 1, 0});
		auto TopWall = Render->CreateInstance(Wall, {0, 2, 0}, {0, -1, 0}, {0, 0, 1});
		auto BottomWall = Render->CreateInstance(Wall, {0, -2, 0}, {0, 1, 0}, {0, 0, -1});
		auto LeftWall = Render->CreateInstance(Wall, {-2, 0, 0}, {1, 0, 0}, {0, 1, 0});
		auto RightWall = Render->CreateInstance(Wall, {2, 0, 0}, {-1, 0, 0}, {0, 1, 0});
		auto Sphere1 = Render->CreateInstance(Sphere, {0, -1.499, 1});
		auto Sphere2 = Render->CreateInstance(Sphere, {1, -1.499, -1});
		auto Sphere3 = Render->CreateInstance(Sphere, {-1, -1.499, -1});

		auto WhiteMaterial = Render->CreateDiffuseMaterial({1, 1, 1});
		auto RedMaterial = Render->CreateDiffuseMaterial({1, 0, 0});
		auto GreenMaterial = Render->CreateDiffuseMaterial({0, 1, 0});
		auto GlassMaterial = Render->CreateRefractiveMaterial({1, 1, 1});
		auto DiffuseMaterial = Render->CreateDiffuseMaterial({1, 1, 0});
		auto MetalMaterial = Render->CreateReflectiveMaterial({1, 0, 1});
		auto CheckerboardTexture = Render->CreateTexture("../resources/Checkerboard_256.png");
		Render->MaterialSetBaseColor(WhiteMaterial, CheckerboardTexture);

		Render->ShapeSetMaterial(BackWall, WhiteMaterial);
		Render->ShapeSetMaterial(TopWall, WhiteMaterial);
		Render->ShapeSetMaterial(BottomWall, WhiteMaterial);
		Render->ShapeSetMaterial(LeftWall, RedMaterial);
		Render->ShapeSetMaterial(RightWall, GreenMaterial);
		Render->ShapeSetMaterial(Sphere1, GlassMaterial);
		Render->ShapeSetMaterial(Sphere2, DiffuseMaterial);
		Render->ShapeSetMaterial(Sphere3, MetalMaterial);

		//auto Light = Render->CreateLight({0, 1.95, 0});

		Render->SetIBL("../resources/sun.exr");
	}
	else if (Name == SCENE_CORNELL_BOX_2)
	{
		auto Wall = Render->CreatePlane({4, 4});
		auto Sphere = Render->CreateUVSphere(512, 256, 0.5f);

		auto BackWall = Render->CreateInstance(Wall, {0, 0, -2}, {0, 0, 1}, {0, 1, 0});
		auto TopWall = Render->CreateInstance(Wall, {0, 2, 0}, {0, -1, 0}, {0, 0, 1});
		auto BottomWall = Render->CreateInstance(Wall, {0, -2, 0}, {0, 1, 0}, {0, 0, -1});
		auto LeftWall = Render->CreateInstance(Wall, {-2, 0, 0}, {1, 0, 0}, {0, 1, 0});
		auto RightWall = Render->CreateInstance(Wall, {2, 0, 0}, {-1, 0, 0}, {0, 1, 0});
		auto Sphere1 = Render->CreateInstance(Sphere, {0, -1.499, 1});
		auto Sphere2 = Render->CreateInstance(Sphere, {1, -1.499, -1});
		auto Sphere3 = Render->CreateInstance(Sphere, {-1, -1.499, -1});

		auto WhiteMaterial = Render->CreateDiffuseMaterial({1, 1, 1});
		auto RedMaterial = Render->CreateDiffuseMaterial({1, 0, 0});
		auto GreenMaterial = Render->CreateDiffuseMaterial({0, 1, 0});
		auto GlassMaterial = Render->CreateRefractiveMaterial({1, 1, 1});
		auto DiffuseMaterial = Render->CreateDiffuseMaterial({1, 1, 0});
		auto MetalMaterial = Render->CreateReflectiveMaterial({1, 0, 1});
		auto CheckerboardTexture = Render->CreateTexture("../resources/Checkerboard_256.png");
		Render->MaterialSetBaseColor(WhiteMaterial, CheckerboardTexture);

		Render->ShapeSetMaterial(BackWall, WhiteMaterial);
		Render->ShapeSetMaterial(TopWall, WhiteMaterial);
		Render->ShapeSetMaterial(BottomWall, WhiteMaterial);
		Render->ShapeSetMaterial(LeftWall, RedMaterial);
		Render->ShapeSetMaterial(RightWall, GreenMaterial);
		Render->ShapeSetMaterial(Sphere1, GlassMaterial);
		Render->ShapeSetMaterial(Sphere2, DiffuseMaterial);
		Render->ShapeSetMaterial(Sphere3, MetalMaterial);

		auto Light = Render->CreateDirectionalLight({-1, -1, -1}, {1, 1, 1}, 1);
		auto Light2 = Render->CreateDirectionalLight({1, -1, -1}, {1, 1, 1}, 1);

		Render->SetIBL("../resources/hdr_black_image.exr");
	}
	else if (Name == SCENE_STANFORD_DRAGON)
	{

		auto Wall = Render->CreatePlane({32, 32});
		auto Dragon = Render->CreateModel("../models/Dragon/dragon.obj");

		auto BottomWall = Render->CreateInstance(Wall, {0, -2, 0}, {0, 1, 0}, {0, 0, -1});
		auto DragonInstance = Render->CreateInstance(Dragon, {0, -1.292, 1}, {1, 0, 0}, {0, -1, 0}, {2.5, 2.5, 2.5});

		auto WhiteMaterial = Render->CreateDiffuseMaterial({1, 1, 1});
		auto GlassMaterial = Render->CreateRefractiveMaterial({1, 1, 1});
		auto CheckerboardTexture = Render->CreateTexture("../resources/Checkerboard_256.png");
		Render->MaterialSetBaseColor(WhiteMaterial, CheckerboardTexture);


		Render->ShapeSetMaterial(BottomWall, WhiteMaterial);
		Render->ShapeSetMaterial(DragonInstance, GlassMaterial);

		//auto Light = Render->CreateLight({0, 1.95, 0});

		Render->SetIBL("../resources/sun.exr");
	}
	else if (Name == SCENE_ROUGH_GLASS)
	{
		auto Plane = Render->CreatePlane({32, 32});
		auto Sphere = Render->CreateUVSphere(512, 256, 0.5f);

		auto PlaneInstance = Render->CreateInstance(Plane, {0, -2, 0}, {0, 1, 0}, {0, 0, 1});
		auto Material_1 = Render->CreateDiffuseMaterial({0.8, 0.8, 0.8});
		auto CheckerboardTexture = Render->CreateTexture("../resources/Checkerboard_256.png");
		Render->MaterialSetBaseColor(Material_1, CheckerboardTexture);
		Render->ShapeSetMaterial(PlaneInstance, Material_1);

		for (int i = 0; i < 20; ++i)
		{
			auto SphereInstance = Render->CreateInstance(Sphere, {float(-10 + i), -1.49, 1});
			auto Material = Render->CreateRefractiveMaterial({1, 1, 1});
			Render->MaterialSetTransmissionRoughness(Material, i * 0.05f);
			Render->ShapeSetMaterial(SphereInstance, Material);
		}

		Render->SetIBL("../resources/sun.exr");
	}
	else if (Name == SCENE_DIRECTIONAL_LIGHT)
	{
		auto Plane = Render->CreatePlane({1024, 1024});
		auto Sphere = Render->CreateUVSphere(512, 256, 0.5f);

		auto PlaneInstance = Render->CreateInstance(Plane, {0, -2, 0}, {0, -1, 0}, {0, 0, 1});
		auto Sphere1 = Render->CreateInstance(Sphere, {0, -1.499, 1});
		auto Sphere2 = Render->CreateInstance(Sphere, {1, -1.499, -1});
		auto Sphere3 = Render->CreateInstance(Sphere, {-1, -1.499, -1});
		auto Sphere4 = Render->CreateInstance(Sphere, {511, -1.499, 0});

		auto WhiteMaterial = Render->CreateDiffuseMaterial({1, 1, 1});
		auto GlassMaterial = Render->CreateRefractiveMaterial({1, 1, 1});
		auto MetalMaterial = Render->CreateReflectiveMaterial({1, 0, 1});

		Render->ShapeSetMaterial(PlaneInstance, WhiteMaterial);
		Render->ShapeSetMaterial(Sphere1, GlassMaterial);
		Render->ShapeSetMaterial(Sphere2, WhiteMaterial);
		Render->ShapeSetMaterial(Sphere3, MetalMaterial);
		Render->ShapeSetMaterial(Sphere4, WhiteMaterial);

		auto Light = Render->CreateDirectionalLight({-1, -1, -1}, {1, 1, 1}, 1);

		Render->SetIBL("../resources/hdr_black_image.exr");
	}
	else if (Name == SCENE_POINT_LIGHT)
	{
		auto Plane = Render->CreatePlane({1024, 1024});
		auto Sphere = Render->CreateUVSphere(512, 256, 0.5f);

		auto PlaneInstance = Render->CreateInstance(Plane, {0, -2, 0}, {0, 1, 0}, {0, 0, 1});
		auto Sphere1 = Render->CreateInstance(Sphere, {0, -1.499, 1});
		auto Sphere2 = Render->CreateInstance(Sphere, {1, -1.499, -1});
		auto Sphere3 = Render->CreateInstance(Sphere, {-1, -1.499, -1});

		auto WhiteMaterial = Render->CreateDiffuseMaterial({1, 1, 1});
		auto GlassMaterial = Render->CreateRefractiveMaterial({1, 1, 1});
		auto MetalMaterial = Render->CreateReflectiveMaterial({1, 0, 1});

		Render->ShapeSetMaterial(PlaneInstance, WhiteMaterial);
		Render->ShapeSetMaterial(Sphere1, GlassMaterial);
		Render->ShapeSetMaterial(Sphere2, WhiteMaterial);
		Render->ShapeSetMaterial(Sphere3, MetalMaterial);

		auto Light1 = Render->CreatePointLight({3, 3, 0}, {1, 1, 1}, 1);
		auto Light2 = Render->CreatePointLight({0, 9, -3}, {1, 1, 1}, 1);
		auto Light3 = Render->CreatePointLight({-3, 3, 0}, {1, 1, 1}, 3);

		Render->SetIBL("../resources/hdr_black_image.exr");
	}
	else if (Name == SCENE_SPOT_LIGHT)
	{
		auto Plane = Render->CreatePlane({1024, 1024});
		auto Sphere = Render->CreateUVSphere(512, 256, 0.5f);

		auto PlaneInstance = Render->CreateInstance(Plane, {0, -2, 0}, {0, 1, 0}, {0, 0, 1});
		auto Sphere1 = Render->CreateInstance(Sphere, {0, -1.499, 1});
		auto Sphere2 = Render->CreateInstance(Sphere, {1, -1.499, -1});
		auto Sphere3 = Render->CreateInstance(Sphere, {-1, -1.499, -1});

		auto WhiteMaterial = Render->CreateDiffuseMaterial({1, 1, 1});
		auto GlassMaterial = Render->CreateRefractiveMaterial({1, 1, 1});
		auto MetalMaterial = Render->CreateReflectiveMaterial({1, 0, 1});

		Render->ShapeSetMaterial(PlaneInstance, WhiteMaterial);
		Render->ShapeSetMaterial(Sphere1, GlassMaterial);
		Render->ShapeSetMaterial(Sphere2, WhiteMaterial);
		Render->ShapeSetMaterial(Sphere3, MetalMaterial);

		auto Light1 = Render->CreateSpotLight({10, 5, 10}, {-2, -1, -2}, {1, 1, 1}, 1, 0.6981f, 0.5235f);

		Render->SetIBL("../resources/hdr_black_image.exr");
	}
	else if (Name == SCENE_WORLD_COORDINATES_AOV)
	{
		auto Plane = Render->CreatePlane({4, 4});

		auto PlaneInstance = Render->CreateInstance(Plane, {0, 0, 0}, {0, 0, 1}, {0, 1, 0});

		auto WhiteMaterial = Render->CreateDiffuseMaterial({1, 1, 1});

		Render->ShapeSetMaterial(PlaneInstance, WhiteMaterial);

		Render->SetIBL("../resources/sun.exr");
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

		auto WhiteMaterial = Render->CreateDiffuseMaterial({1, 1, 1});
		auto RedMaterial = Render->CreateDiffuseMaterial({1, 0, 0});
		auto GreenMaterial = Render->CreateDiffuseMaterial({0, 1, 0});
		auto GlassMaterial = Render->CreateDiffuseMaterial({0, 1, 1});
		auto DiffuseMaterial = Render->CreateDiffuseMaterial({1, 1, 0});
		auto PlasticMaterial = Render->CreateDiffuseMaterial({1, 0, 1});

		Render->ShapeSetMaterial(BackWall, WhiteMaterial);
		Render->ShapeSetMaterial(TopWall, WhiteMaterial);
		Render->ShapeSetMaterial(BottomWall, WhiteMaterial);
		Render->ShapeSetMaterial(LeftWall, RedMaterial);
		Render->ShapeSetMaterial(RightWall, GreenMaterial);
		Render->ShapeSetMaterial(Sphere1, GlassMaterial);
		Render->ShapeSetMaterial(Sphere2, DiffuseMaterial);
		Render->ShapeSetMaterial(Sphere3, PlasticMaterial);
		Render->ShapeSetMaterial(Instances.back(), PlasticMaterial);

		auto Light = Render->CreatePointLight({0, 1.95, 0}, {1, 1, 1}, 1);

		Render->SetIBL("../resources/brown_photostudio_02_4k.exr");

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

		auto GlassMaterial = Render->CreateDiffuseMaterial({0, 1, 1});
		auto DiffuseMaterial = Render->CreateDiffuseMaterial({1, 1, 0});
		auto PlasticMaterial = Render->CreateDiffuseMaterial({1, 0, 1});

		Render->ShapeSetMaterial(Sphere1, GlassMaterial);
		Render->ShapeSetMaterial(Sphere2, DiffuseMaterial);
		Render->ShapeSetMaterial(Sphere3, PlasticMaterial);

		Render->SetIBL("../resources/hdr_image.exr");
	}
	else if (Name == SCENE_BIG_PLANES)
	{
		auto Wall = Render->CreatePlane({10, 10});
		auto BackWall1 = Render->CreateInstance(Wall, {0, 0, 0}, {0, 0, -1}, {0, 1, 0});
		auto BackWall = Render->CreateInstance(Wall, {0, 0, 1}, {0, 0, -1}, {0, 1, 0});

		auto RedMaterial = Render->CreateDiffuseMaterial({1, 0, 0});
		auto GreenMaterial = Render->CreateDiffuseMaterial({0, 1, 0});

		Render->ShapeSetMaterial(BackWall1, RedMaterial);
		Render->ShapeSetMaterial(BackWall, GreenMaterial);

		Render->SetIBL("../resources/hdr_image.exr");
	}
	else if (Name == SCENE_COORDINATE_SYSTEM_REMINDER)
	{
		auto Sphere = Render->CreateUVSphere(64, 32, 0.5f);
		auto SphereX = Render->CreateInstance(Sphere, {3, 0, 0}, {1, 0, 0}, {0, 1, 0});
		auto SphereY = Render->CreateInstance(Sphere, {0, 3, 0}, {1, 0, 0}, {0, 1, 0});
		auto SphereZ = Render->CreateInstance(Sphere, {0, 0, 3}, {1, 0, 0}, {0, 1, 0});
		auto SphereC = Render->CreateInstance(Sphere, {0, 0, 0}, {1, 0, 0}, {0, 1, 0});

		auto RedMaterial = Render->CreateDiffuseMaterial({1, 0, 0});
		auto GreenMaterial = Render->CreateDiffuseMaterial({0, 1, 0});
		auto BlueMaterial = Render->CreateDiffuseMaterial({0, 0, 1});
		auto WhiteMaterial = Render->CreateDiffuseMaterial({1, 1, 1});

		Render->ShapeSetMaterial(SphereX, RedMaterial);
		Render->ShapeSetMaterial(SphereY, GreenMaterial);
		Render->ShapeSetMaterial(SphereZ, BlueMaterial);
		Render->ShapeSetMaterial(SphereC, WhiteMaterial);

		Render->SetIBL("../resources/brown_photostudio_02_4k.exr");
	}
	else if (Name == SCENE_DIFFUSE_MATERIAL)
	{
		auto Plane = Render->CreatePlane({32, 32});
		auto Dragon = Render->CreateModel("../models/Dragon/dragon.obj");
		auto Sphere = Render->CreateUVSphere(256, 128, 0.5f);

		auto Floor = Render->CreateInstance(Plane, {0, -2, 0}, {0, 1, 0}, {0, 0, -1});
		auto DragonInstance = Render->CreateInstance(Dragon, {0, -1.292, 1}, {1, 0, 0}, {0, -1, 0}, {2.5, 2.5, 2.5});
		auto CrimsonSphere = Render->CreateInstance(Sphere, {-2, -1.5, 2}, {0, 1, 0}, {0, 0, -1});
		auto WhiteSphere = Render->CreateInstance(Sphere, {0, -1.5, 2}, {0, 1, 0}, {0, 0, -1});
		auto OrangeSphere = Render->CreateInstance(Sphere, {1.5, -1.5, 2}, {0, 1, 0}, {0, 0, -1});

		float DiffuseRoughness = 1.f;
		auto GreyMaterial = Render->CreateDiffuseMaterial({0.5f, 0.5f, 0.5f});
		Render->MaterialSetDiffuseRoughness(GreyMaterial, DiffuseRoughness);
		auto JadeMaterial = Render->CreateDiffuseMaterial({0, 0.7333f, 0.4666f});
		Render->MaterialSetDiffuseRoughness(JadeMaterial, DiffuseRoughness);
		auto CrimsonMaterial = Render->CreateDiffuseMaterial({0.698f, 0.1333f, 0.1333f});
		Render->MaterialSetDiffuseRoughness(CrimsonMaterial, DiffuseRoughness);
		auto WhiteMaterial = Render->CreateDiffuseMaterial({1, 1, 1});
		Render->MaterialSetDiffuseRoughness(WhiteMaterial, DiffuseRoughness);
		auto OrangeMaterial = Render->CreateDiffuseMaterial({1, 0.3019f, 0});
		Render->MaterialSetDiffuseRoughness(OrangeMaterial, DiffuseRoughness);

		Render->ShapeSetMaterial(Floor, GreyMaterial);
		Render->ShapeSetMaterial(DragonInstance, JadeMaterial);
		Render->ShapeSetMaterial(CrimsonSphere, CrimsonMaterial);
		Render->ShapeSetMaterial(WhiteSphere, WhiteMaterial);
		Render->ShapeSetMaterial(OrangeSphere, OrangeMaterial);

		Render->SetIBL("../resources/sun.exr");
	}
	else if (Name == SCENE_TEST_1)
	{
		auto Sphere = Render->CreateUVSphere(64, 32, 2.5f);

		auto SphereRight = Render->CreateInstance(Sphere, {50, 0, 0});
		auto SphereLeft = Render->CreateInstance(Sphere, {-50, 0, 0});
		auto SphereFront = Render->CreateInstance(Sphere, {0, 0, -50});
		auto SphereBack = Render->CreateInstance(Sphere, {0, 0, 50});
		auto SphereUp = Render->CreateInstance(Sphere, {0, 50, 0});
		auto SphereDown = Render->CreateInstance(Sphere, {0, -50, 0});

		auto OrangeMaterial = Render->CreateDiffuseMaterial({1, 0.3019f, 0});

		Render->ShapeSetMaterial(SphereRight, OrangeMaterial);
		Render->ShapeSetMaterial(SphereLeft, OrangeMaterial);
		Render->ShapeSetMaterial(SphereFront, OrangeMaterial);
		Render->ShapeSetMaterial(SphereBack, OrangeMaterial);
		Render->ShapeSetMaterial(SphereUp, OrangeMaterial);
		Render->ShapeSetMaterial(SphereDown, OrangeMaterial);

		Render->SetIBL("../resources/sun.exr");
	}
	else if (Name == SCENE_WHITE_FURNACE)
	{
		auto Sphere = Render->CreateUVSphere(256, 128, 0.5f);
		auto WhiteSphere = Render->CreateInstance(Sphere, {0, -1.5, 2}, {0, 1, 0}, {0, 0, -1});

		float DiffuseRoughness = 0.f;
		auto WhiteMaterial = Render->CreateDiffuseMaterial({1, 1, 1});
		Render->MaterialSetDiffuseRoughness(WhiteMaterial, DiffuseRoughness);
		Render->ShapeSetMaterial(WhiteSphere, WhiteMaterial);

		Render->SetIBL("../resources/white_furnace.exr");
	}
	else if (Name == SCENE_TEST_2)
	{
		auto Sphere = Render->CreateUVSphere(64, 32, 2.5f);

		auto SphereInstance = Render->CreateInstance(Sphere, {0, 0, 0}, {0, 1, 0}, {0, 0, -1});

		auto OrangeMaterial = Render->CreateDiffuseMaterial({1, 0.3019f, 0});

		Render->SetCameraPosition({0, 7.5, 0}, {{0, -1.f, 0.f}}, {{0, 0, 1}}, {});

		Render->ShapeSetMaterial(SphereInstance, OrangeMaterial);

		Render->SetIBL("../resources/sun.exr");
	}
	else if (Name == SCENE_VIKINGS_ROOM)
	{
		auto Model = Render->CreateModel("../models/viking_room/viking_room.obj");
		auto ModelInstance = Render->CreateInstance(Model, {3, 0, 0}, {1, 0, 0}, {0, 1, 0});

		auto Texture = Render->CreateTexture("../models/viking_room/viking_room.png");

		auto Material =  Render->CreateEmptyMaterial();
		Render->MaterialSetBaseColor(Material, Texture);

		Render->ShapeSetMaterial(ModelInstance, Material);

		Render->SetIBL("../resources/brown_photostudio_02_4k.exr");
	}
	else if (Name == SCENE_SPECULAR_ROUGHNESS)
	{
		auto Plane = Render->CreatePlane({1024, 1024});
		auto Sphere = Render->CreateUVSphere(512, 256, 0.5f);

		auto PlaneInstance = Render->CreateInstance(Plane, {0, -2, 0}, {0, 1, 0}, {0, 0, 1});
		auto Material_1 = Render->CreateDiffuseMaterial({0.8, 0.8, 0.8});
		Render->ShapeSetMaterial(PlaneInstance, Material_1);

		for (int i = 0; i < 20; ++i)
		{
			auto SphereInstance = Render->CreateInstance(Sphere, {float(-10 + i), -1.499, 1});
			auto Material = Render->CreateReflectiveMaterial({1, 0.8431f, 0});
			Render->MaterialSetSpecularRoughness(Material, i * 0.05f);
			Render->ShapeSetMaterial(SphereInstance, Material);
		}

		Render->SetIBL("../resources/sun.exr");
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