#include "render.h"
#include "controller.h"

#include <memory>

class FApplication
{
public:
    FApplication();
    ~FApplication();

    int Run();

private:
    std::shared_ptr<FRender> Render = nullptr;
};