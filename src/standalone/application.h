#pragma once

#include "render.h"
#include "swapchain.h"

#include <memory>

class FApplication
{
public:
    FApplication();
    ~FApplication();

    int Run();
	void SetShouldRecreateSwapchain(uint32_t NewWidth, uint32_t NewHeight);

private:
	uint32_t Width = 1920;
	uint32_t Height = 1080;
    std::shared_ptr<FRender> Render = nullptr;
	std::shared_ptr<FSwapchain> Swapchain = nullptr;
	bool bShouldRecreateSwaochain = false;
};