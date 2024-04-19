#pragma once

#include "render.h"
#include "swapchain.h"
#include "window_manager.h"
#include "controller.h"
#include "scene_loader.h"
#include "task_imgui.h"

#include <memory>

class FApplication
{
public:
    FApplication();
    ~FApplication();

    int Run();
	void SetSwapchainWasResized(uint32_t NewWidth, uint32_t NewHeight);

private:
	/// Scene data
	std::vector<ECS::FEntity> Models;
	std::vector<ECS::FEntity> Materials;
	std::vector<ECS::FEntity> Lights;
	std::vector<ECS::FEntity> Cameras;

	uint32_t Width = 1920;
	uint32_t Height = 1080;
    std::shared_ptr<FRender> Render = nullptr;
	std::shared_ptr<FSwapchain> Swapchain = nullptr;
	std::shared_ptr<FWindowManager> WindowManager = nullptr;
	std::shared_ptr<FController> Controller = nullptr;
	std::shared_ptr<FImguiTask> ImguiTask = nullptr;
	std::shared_ptr<FSceneLoader> SceneLoader = nullptr;
	bool bSwapchainWasResized = false;
};