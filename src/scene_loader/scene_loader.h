#pragma once

#include <string>

#include "render.h"

#define SCENE_BIG_PLANES "Big Planes"
#define SCENE_THREE_SPHERES "Three Spheres"
#define SCENE_CORNELL_BOX_ANIMATED "Cornell Box Animated"
#define SCENE_CORNELL_BOX "Cornell Box"
#define SCENE_CORNELL_BOX_2 "Cornell Box 2"
#define SCENE_STANFORD_DRAGON "Stanford Dragon"
#define SCENE_DIRECTIONAL_LIGHT "Directional Light"
#define SCENE_POINT_LIGHT "Point Light"
#define SCENE_SPOT_LIGHT "Spot Light"
#define SCENE_SPECULAR_ROUGHNESS "Specular roughness"
#define SCENE_WORLD_COORDINATES_AOV "World coordinates aov"
#define SCENE_FLOATING_SPHERES "Floating spheres"
#define SCENE_COORDINATE_SYSTEM_REMINDER "Coordinate system reminder"
#define SCENE_VIKINGS_ROOM "Vikings room"
#define SCENE_DIFFUSE_MATERIAL "Diffuse material"

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