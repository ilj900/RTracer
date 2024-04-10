#include "render.h"
#include "controller.h"
#include "swapchain.h"

#include <memory>

class FApplication
{
public:
    FApplication();
    ~FApplication();

    int Run();

private:
    std::shared_ptr<FRender> Render = nullptr;
	std::shared_ptr<FSwapchain> Swapchain = nullptr;
};