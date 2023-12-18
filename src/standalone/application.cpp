#pragma once

#include <chrono>

#include "application.h"
#include "render.h"

FApplication::FApplication()
{
    Render = std::make_shared<FRender>();
    Controller = std::make_shared<FController>();

    Controller->SetWindow(Render->Window);
    Controller->SetRender(Render);
    Controller->UpdateCallbacks();
}

FApplication::~FApplication()
{
    Controller = nullptr;
    Render = nullptr;
}

int FApplication::Run()
{
    int i = 0;
    while (0 == i) {
        static auto StartTime = std::chrono::high_resolution_clock::now();

        auto CurrentTime = std::chrono::high_resolution_clock::now();
        float Time = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - StartTime).count();
        StartTime = CurrentTime;

        Controller->Update(Time);
        i = Render->Update();
        i += Render->Render();
    }

    return 0;
}