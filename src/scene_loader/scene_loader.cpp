#include "render.h"

#include "scene_loader.h"

#include <random>

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
				Render->SetInstancePosition(Instance, NewInstanceCoordinates, {}, {});
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
		auto DragonInstance = Render->CreateInstance(Dragon, {0, -1.292, 1}, {1, 0, 0}, {0, 1, 0}, {2.5, 2.5, 2.5});

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
		auto Sphere = Render->CreateUVSphere(512, 256, 1.f);
        int32_t W = 4;
        int32_t H = 4;
        float Ws = 1.f / float(W);
        float Hs = 1.f / float (W * H);

        for (int i = 0; i < W; ++i)
        {
            for (int j = 0; j < H; ++j)
            {
                auto SphereInstance = Render->CreateInstance(Sphere, { float(float(-W) * 0.5f + float(i)) * 2.1f, float(float(H) * 0.5f - float(j)) * 2.1f, 0 });
                auto Material = Render->CreateRefractiveMaterial({ 1, 1, 1 });
                Render->MaterialSetTransmissionRoughness(Material, float(i) * Ws + float(j) * Hs);
                Render->ShapeSetMaterial(SphereInstance, Material);
            }
        }

		Render->SetIBL("../resources/sun.exr");
	}
	else if (Name == SCENE_GLASS_PLANES)
	{
		auto Plane = Render->CreateModel("../models/glass_plane.obj");

		auto SolidPlane = Render->CreateInstance(Plane, { 0, 0, 0});
		FVector3 Direction = {0, 0, 1};
		FVector3 Up = {0, 1, 0};
		Direction.SelfRotateZ(-15 / M_RAD);
		Up.SelfRotateZ(-15 / M_RAD);
		static FVector3 Position = { 0, 3, 0};
		static auto GlassPlane = Render->CreateInstance(Plane, Position, Direction, Up);

		auto SolidMaterial = Render->CreateDiffuseMaterial({ 1, 1, 1 });
		auto CheckerboardTexture = Render->CreateTexture("../resources/Checkerboard_32.png");
		Render->MaterialSetBaseColor(SolidMaterial, CheckerboardTexture);
		auto GlassMaterial = Render->CreateRefractiveMaterial({ 1, 1, 1 });
		auto RoughnessTexture = Render->CreateTexture("../resources/roughness_texture_1.0.png");
		Render->MaterialSetTransmissionRoughness(GlassMaterial, RoughnessTexture);

		Render->ShapeSetMaterial(SolidPlane, SolidMaterial);
		Render->ShapeSetMaterial(GlassPlane, GlassMaterial);

		Render->SetIBL("../resources/sun.exr");

		//Updater = [&](float, float CurrentTime)
		//{
		//	FVector3 Direction = {0, 0, 1};
		//	FVector3 Up = {0, 1, 0};
		//	Direction.SelfRotateZ(15 * CurrentTime / M_RAD);
		//	Up.SelfRotateZ(15 * CurrentTime / M_RAD);
		//	Render->SetInstancePosition(GlassPlane, Position, Direction, Up);
		//};
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

		auto Light1 = Render->CreateDirectionalLight({-1, -1, -1}, {1, 1, 1}, 1);
		auto Light2 = Render->CreateDirectionalLight({1, -1, -1}, {1, 1, 1}, 1);
		auto Light3 = Render->CreateDirectionalLight({-1, -1, 1}, {1, 1, 1}, 3);

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

		auto Light1 = Render->CreateSpotLight({10, 5, 10}, {-2, -1, -2}, {1, 1, 0}, 10, 0.6981f, 0.5235f);
		auto Light2 = Render->CreateSpotLight({-10, 10, 10}, {2, -2, -2}, {0, 1, 1}, 20, 0.6981f, 0.5235f);
		auto Light3 = Render->CreateSpotLight({-10, 2, -10}, {2, -1, 2}, {1, 0, 1}, 40, 0.6981f, 0.5235f);

		Render->SetIBL("../resources/hdr_black_image.exr");
	}
	else if (Name == SCENE_CORNELL_BOX_ANIMATED)
	{
		auto Wall = Render->CreatePlane({4, 4});
		auto Sphere = Render->CreateIcosahedronSphere(0.5f, 5, false);
		auto Cube = Render->CreateCube();

		auto BackWall = Render->CreateInstance(Wall, {0, 0, -2}, {0, 0, 1}, {0, 1, 0});
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

		auto Light = Render->CreatePointLight({0, 1.8, 0}, {1, 1, 1}, 1);

		Render->SetIBL("../resources/brown_photostudio_02_4k.exr");

		Updater = [&](float, float CurrentTime)
		{
			auto NewInstanceCoordinates = Render->GetInstancePosition(Instances.back());
			float R = sqrt(NewInstanceCoordinates.X * NewInstanceCoordinates.X + NewInstanceCoordinates.Z * NewInstanceCoordinates.Z);
			NewInstanceCoordinates.Y = sin((R / 4 + CurrentTime)) * 2;
			Render->SetInstancePosition(Instances.back(), NewInstanceCoordinates, {}, {});
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

		Render->SetIBL("../resources/brown_photostudio_02_4k.exr");
	}
	else if (Name == SCENE_BIG_PLANES)
	{
		auto Wall = Render->CreatePlane({10, 10});
		auto Wall1 = Render->CreateInstance(Wall, {0.5f, 0, 0}, {1, 0, 0}, {0, 1, 0});
		auto Wall2 = Render->CreateInstance(Wall, {-0.5f, 0, 0}, {1, 0, 0}, {0, 1, 0});

		auto RedMaterial = Render->CreateDiffuseMaterial({1, 0, 0});
		auto GreenMaterial = Render->CreateDiffuseMaterial({0, 1, 0});

		Render->ShapeSetMaterial(Wall1, RedMaterial);
		Render->ShapeSetMaterial(Wall2, GreenMaterial);

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
		auto DragonInstance = Render->CreateInstance(Dragon, {0, -1.292, 1}, {1, 0, 0}, {0, 1, 0}, {2.5, 2.5, 2.5});
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
	else if (Name == SCENE_VIKINGS_ROOM)
	{
		auto Model = Render->CreateModel("../models/viking_room/viking_room.obj");
		auto ModelInstance = Render->CreateInstance(Model, {0, -0.5f, 5}, {0, 1, 0}, {1, 0, 0});

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

        int SpheresCount = 5;
		for (int i = 0; i < SpheresCount; ++i)
		{
			auto SphereInstance = Render->CreateInstance(Sphere, {float(-SpheresCount + i * 2), -1.499, 1});
			auto Material = Render->CreateReflectiveMaterial({1, 0.8431f, 0});
			Render->MaterialSetSpecularRoughness(Material, i * (1.f / float(SpheresCount - 1)));
			Render->ShapeSetMaterial(SphereInstance, Material);
		}

		Render->SetIBL("../resources/sun.exr");
	}
	else if (Name == SCENE_AREA_LIGHTS)
	{
		auto Floor = Render->CreatePlane({32, 32});
		auto Sphere = Render->CreateUVSphere(512, 256, 0.5f);

		auto FloorInstance = Render->CreateInstance(Floor, {0, -2, 0}, {0, 1, 0}, {0, 0, -1});

		auto EmissiveMaterial = Render->CreateEmptyMaterial();
		Render->MaterialSetEmissionWeight(EmissiveMaterial, 1.f);
		Render->MaterialSetEmissionColor(EmissiveMaterial, {1, 1, 1});
		auto FloorMaterial = Render->CreateDiffuseMaterial({1, 1, 1});

		uint32_t Seed = 42;
		std::mt19937 Generator(Seed);
		std::uniform_real_distribution<float> XZ(-14.f, 14.f);
		std::uniform_real_distribution<float> Y(-1.5, 8.f);

		for (int i = 0; i < 32; ++i)
		{
			auto SphereInstance = Render->CreateInstance(Sphere, {XZ(Generator), Y(Generator), XZ(Generator)});
			Render->ShapeSetMaterial(SphereInstance, EmissiveMaterial);
			Render->CreateAreaLight(SphereInstance);
		}

		Render->ShapeSetMaterial(FloorInstance, FloorMaterial);

		Render->SetIBL("../resources/hdr_black_image.exr");
	}
	else if (Name == SCENE_AREA_LIGHTS_2)
	{
		auto Plane = Render->CreatePlane({32, 32});

		auto Floor = Render->CreateInstance(Plane, {0, -2, 0}, {0, 1, 0}, {0, 0, -1});
		auto Light = Render->CreateInstance(Plane, {0, 120, 0}, {0, -1, 0}, {0, 0, 1});

		auto EmissiveMaterial = Render->CreateEmptyMaterial();
		Render->MaterialSetEmissionWeight(EmissiveMaterial, 1.f);
		Render->MaterialSetEmissionColor(EmissiveMaterial, {1, 1, 1});
		auto FloorMaterial = Render->CreateDiffuseMaterial({1, 1, 1});

		Render->ShapeSetMaterial(Light, EmissiveMaterial);
		Render->CreateAreaLight(Light);

		Render->ShapeSetMaterial(Floor, FloorMaterial);

		Render->SetIBL("../resources/hdr_black_image.exr");
	}
	else if (Name == SCENE_TEXTURE_SYSTEM_TEST)
	{
		auto Shaderball = Render->CreateModel("../models/Shaderball.obj");

		{
			//auto ShaderballWood = Render->CreateInstance(Shaderball, {0, 0, 0}, {0, 0, 1}, {0, 1, 0});
//
			//auto WoodMaterial = Render->CreateEmptyMaterial();
//
			//Render->MaterialSetBaseColorWeight(WoodMaterial, 1.f);
			//auto AlbedoTexture = Render->CreateTexture("../resources/MaterialX_Wood/wood_color.jpg");
			//Render->MaterialSetBaseColor(WoodMaterial, AlbedoTexture);
//
			//Render->MaterialSetSpecularWeight(WoodMaterial, 0.4f);
			//auto RoughnessTexture = Render->CreateTexture("../resources/MaterialX_Wood/wood_roughness.jpg");
			//Render->MaterialSetSpecularColor(WoodMaterial, FVector3(1, 1, 1));
			//Render->MaterialSetSpecularRoughness(WoodMaterial, RoughnessTexture);
			//Render->MaterialSetSpecularAnisotropy(WoodMaterial, 0.5f);
//
			//Render->ShapeSetMaterial(ShaderballWood, WoodMaterial);
		}

		{
			//auto ShaderballChrome = Render->CreateInstance(Shaderball, {3, 0, 0}, {0, 0, 1}, {0, 1, 0});
//
			//auto ChromeMaterial = Render->CreateEmptyMaterial();
			//Render->MaterialSetBaseColorWeight(ChromeMaterial, 1.f);
			//Render->MaterialSetBaseColor(ChromeMaterial, FVector3(1, 1, 1));
//
			//Render->MaterialSetSpecularWeight(ChromeMaterial, 1.f);
			//Render->MaterialSetSpecularColor(ChromeMaterial, FVector3(1, 1, 1));
			//Render->MaterialSetSpecularRoughness(ChromeMaterial, 0.f);
			//Render->MaterialSetMetalness(ChromeMaterial, 1.f);
//
			//Render->ShapeSetMaterial(ShaderballChrome, ChromeMaterial);
		}

		{
			auto ShaderballCopper = Render->CreateInstance(Shaderball, {-3, 0, 0}, {0, 0, 1}, {0, 1, 0});

			auto CopperMaterial = Render->CreateEmptyMaterial();
			//Render->MaterialSetBaseColorWeight(CopperMaterial, 1.f);
			Render->MaterialSetBaseColor(CopperMaterial, FVector3(0.96467984, 0.37626296, 0.25818297));
			Render->MaterialSetMetalness(CopperMaterial, 1.f);

			Render->MaterialSetSpecularWeight(CopperMaterial, 1.f);
			Render->MaterialSetSpecularColor(CopperMaterial, FVector3(1, 1, 1));
			Render->MaterialSetSpecularRoughness(CopperMaterial, 0.2f);

			Render->ShapeSetMaterial(ShaderballCopper, CopperMaterial);
		}

		{
			auto ShaderballPlastic = Render->CreateInstance(Shaderball, {0, 3, 0}, {0, 0, 1}, {0, 1, 0});

			auto PlasticMaterial = Render->CreateEmptyMaterial();
			//Render->MaterialSetBaseColorWeight(CopperMaterial, 1.f);
			Render->MaterialSetBaseColor(PlasticMaterial, FVector3(1, 0, 0));
			Render->MaterialSetMetalness(PlasticMaterial, 0.f);

			Render->MaterialSetSpecularWeight(PlasticMaterial, 1.f);
			Render->MaterialSetSpecularColor(PlasticMaterial, FVector3(1, 1, 1));
			Render->MaterialSetSpecularRoughness(PlasticMaterial, 0.f);

			Render->ShapeSetMaterial(ShaderballPlastic, PlasticMaterial);
		}

		Render->SetIBL("../resources/san_giuseppe_bridge_4k.exr");
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