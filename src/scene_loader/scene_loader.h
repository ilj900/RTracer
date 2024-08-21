#pragma once

#include <string>

#include "render.h"

#define SCENE_BIG_PLANES "Big Planes"
#define SCENE_THREE_SPHERES "Three Spheres"
#define SCENE_CORNELL_BOX_ANIMATED "Cornell Box Animated"
#define SCENE_CORNELL_BOX "Cornell Box"
#define SCENE_FLOATING_SPHERES "Floating spheres"
#define SCENE_COORDINATE_SYSTEM_REMINDER "Coordinate system reminder"

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