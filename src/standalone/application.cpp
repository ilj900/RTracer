#pragma once

#include <chrono>

#include "application.h"
#include "window_manager.h"

FApplication::FApplication()
{
    INIT_WINDOW_MANAGER(1920, 1080, false, "RTRacer");
    Render = std::make_shared<FRender>();

    CONTROLLER()->SetWindow(WINDOW_MANAGER()->GetWindow());
    CONTROLLER()->SetRender(Render);
}

FApplication::~FApplication()
{
    Render = nullptr;
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
        i = Render->Update();
        i += Render->Render();
    }

    return 0;
}