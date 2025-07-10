#pragma once

#include <string>

#define SCENE_AREA_LIGHTS "Area_Lights"
#define SCENE_AREA_LIGHTS_2 "Area_Lights_2"
#define SCENE_BIG_PLANES "Big_Planes"
#define SCENE_COORDINATE_SYSTEM_REMINDER "Coordinate_System_Reminder"
#define SCENE_CORNELL_BOX "Cornell_Box"
#define SCENE_CORNELL_BOX_2 "Cornell_Box_2"
#define SCENE_CORNELL_BOX_ANIMATED "Cornell_Box_Animated"
#define SCENE_DIFFUSE_MATERIAL "Diffuse_Material"
#define SCENE_DIRECTIONAL_LIGHT "Directional_Light"
#define SCENE_FLOATING_SPHERES "Floating_Spheres"	/// Shading is broken
#define SCENE_GLASS_PLANES "Glass_Planes"
#define SCENE_POINT_LIGHT "Point_Light"
#define SCENE_ROUGH_GLASS "Rough_Glass"
#define SCENE_SPECULAR_ROUGHNESS "Specular_Roughness"
#define SCENE_SPOT_LIGHT "Spot_Light"
#define SCENE_STANFORD_DRAGON "Stanford_Dragon"
#define	SCENE_TEXTURE_SYSTEM_TEST "Texture_System_Test"
#define SCENE_THREE_SPHERES "Three_Spheres"
#define SCENE_VIKINGS_ROOM "Vikings_Room"
#define SCENE_WHITE_FURNACE "White_Furnace"

class FRender;

class FSceneLoader
{
public:
	FSceneLoader(std::shared_ptr<FRender> RenderIn);
	~FSceneLoader();

	void LoadScene(const std::string& Name);
	void UpdateScene(float DeltaTime, float CurrentTime);

private:
	std::vector<ECS::FEntity> Meshes;
	std::vector<ECS::FEntity> Instances;
	std::vector<ECS::FEntity> Lights;
	std::vector<ECS::FEntity> Materials;
	std::shared_ptr<FRender> Render = nullptr;
	std::function<void(float, float)> Updater = nullptr;
};