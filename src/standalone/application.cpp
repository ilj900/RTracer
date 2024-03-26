#pragma once

#include <chrono>

#include "application.h"
#include "window_manager.h"

FApplication::FApplication()
{
    INIT_WINDOW_MANAGER(1920, 1080, false, "RTRacer");
    RENDER();
    CONTROLLER()->SetWindow(WINDOW());
}

FApplication::~FApplication()
{
    RENDER()->Destroy();
    WINDOW_MANAGER()->Destroy();
}

int FApplication::Run()
{
    int i = 0;
    while (0 == i)
    {
        static auto StartTime = std::chrono::high_resolution_clock::now();

        auto CurrentTime = std::chrono::high_resolution_clock::now();
        float Time = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - StartTime).count();
        StartTime = CurrentTime;

        CONTROLLER()->Update(Time);
        i = RENDER()->Update();
        i += RENDER()->Render();
    }

    return 0;
}