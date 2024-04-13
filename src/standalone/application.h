#pragma once

#include "render.h"
#include "swapchain.h"
#include "window_manager.h"
#include "controller.h"

#include <memory>

class FApplication
{
public:
    FApplication();
    ~FApplication();

    int Run();
	void LoadScene();
	void SetSwapchainWasResized(uint32_t NewWidth, uint32_t NewHeight);

private:
	uint32_t Width = 1920;
	uint32_t Height = 1080;
    std::shared_ptr<FRender> Render = nullptr;
	std::shared_ptr<FSwapchain> Swapchain = nullptr;
	std::shared_ptr<FWindowManager> WindowManager = nullptr;
	std::shared_ptr<FController> Controller = nullptr;
	bool bSwapchainWasResized = false;
};